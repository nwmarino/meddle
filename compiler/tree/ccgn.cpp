#include "ccgn.h"
#include "expr.h"
#include "stmt.h"
#include "type.h"
#include "unit.h"
#include "../core/logger.h"

#include <string>

using namespace meddle;

static String multChar(char c, unsigned n) {
    String str;
    for (unsigned i = 0; i < n; ++i) str += c;
    return str;
}

void CCGN::emitCSeg(const String &seg) {
    m_Cout << seg;
}

void CCGN::emitCLn(const String &ln) {
    m_Cout << ln << "\n";
}

void CCGN::emitHSeg(const String &seg) {
    m_Hout << seg;
}

void CCGN::emitHLn(const String &ln) {
    m_Hout << ln << "\n";
}

String CCGN::translateType(Type *T) {
    if (auto *PT = dynamic_cast<PrimitiveType *>(T)) {
        switch (PT->getKind()) {
            case PrimitiveType::Kind::Void: return "void";
            case PrimitiveType::Kind::Bool: return "unsigned";
            case PrimitiveType::Kind::Char: return "char";
            case PrimitiveType::Kind::Int8: return "char";
            case PrimitiveType::Kind::Int16: return "short";
            case PrimitiveType::Kind::Int32: return "int";
            case PrimitiveType::Kind::Int64: return "long";
            case PrimitiveType::Kind::UInt8: return "unsigned char";
            case PrimitiveType::Kind::UInt16: return "unsigned short";
            case PrimitiveType::Kind::UInt32: return "unsigned int";
            case PrimitiveType::Kind::UInt64: return "unsigned long";
            case PrimitiveType::Kind::Float32: return "float";
            case PrimitiveType::Kind::Float64: return "double";
        }
    }

    fatal("unknown type");
    return "";
}

CCGN::CCGN(const Options &opts, TranslationUnit *U) : m_Opts(opts), m_Unit(U) {
    m_Cout = std::ofstream(U->getFile().filename + ".c");
    m_Hout = std::ofstream(U->getFile().filename + ".h");

    // Get the file name without extension for use in the include guard.
    String noext = U->getFile().filename;
    auto pos = noext.find_last_of('.');
    if (pos != std::string::npos)
        noext = noext.substr(0, pos);

    // Emit the header file define guards and as part of the "declare" phase,
    // all public declarations within the translation unit.
    phase = Phase::Declare;
    m_Hout << "#ifndef " << noext << "_H\n#define " << noext << "_H\n\n";
    m_Unit->accept(this);
    m_Hout << "\n#endif // " << noext << "_H\n";

    // Switch to the "define" phase, and emit actual C code for the package.
    phase = Phase::Define;
    m_Cout << "#include \"" << U->getFile().filename << ".h\"\n\n";
    m_Unit->accept(this);

    m_Cout.close();
    m_Hout.close();
}

void CCGN::visit(TranslationUnit *U) {
    for (auto &D : U->getDecls()) D->accept(this);
}

void CCGN::visit(FunctionDecl *decl) {
    if (phase == Phase::Declare) {
        m_Hout << translateType(decl->getReturnType()) << " " << decl->m_Name 
               << "(";
        for (auto &P : decl->getParams()) {
            P->accept(this);
            m_Hout << (P != decl->getParams().back() ? ", " : "");
        }

        m_Hout << ");\n";
    } else if (phase == Phase::Define) {
        m_Cout << translateType(decl->getReturnType()) << " " << decl->m_Name 
               << "(";

        for (auto &P : decl->getParams()) {
            P->accept(this);
            m_Cout << (P != decl->getParams().back() ? ", " : "");
        }

        m_Cout << ") ";
        decl->m_Body->accept(this);
        m_Cout << "\n";
    }
}

void CCGN::visit(VarDecl *decl) {
    m_Cout << multChar('\t', m_Indent);
    if (decl->isMutable())
        m_Cout << "const ";

    m_Cout << translateType(decl->m_Type) << " " << decl->m_Name;
    if (decl->m_Init) {
        m_Cout << " = ";
        decl->getInit()->accept(this);
    }

    m_Cout << ";\n";
}

void CCGN::visit(ParamDecl *decl) {
    if (phase == Phase::Define)
        m_Hout << translateType(decl->m_Type) << " " << decl->m_Name;
    else if (phase == Phase::Declare)
        m_Cout << translateType(decl->m_Type) << " " << decl->m_Name;
}

void CCGN::visit(BreakStmt *stmt) {
    m_Cout << multChar('\t', m_Indent) << "break;\n";
}

void CCGN::visit(ContinueStmt *stmt) {
    m_Cout << multChar('\t', m_Indent) << "continue;\n";
}

void CCGN::visit(CompoundStmt *stmt) {
    m_Cout << "{\n";
    m_Indent++;
    for (auto &S : stmt->getStmts()) S->accept(this);
    m_Cout << multChar('\t', --m_Indent) << "}\n";
}

void CCGN::visit(DeclStmt *stmt) {
    stmt->getDecl()->accept(this);
}

void CCGN::visit(ExprStmt *stmt) {
    stmt->getExpr()->accept(this);
}

void CCGN::visit(IfStmt *stmt) {
    m_Cout << multChar('\t', m_Indent) << "if (";
    stmt->getCond()->accept(this);
    m_Cout << ") ";
    if (dynamic_cast<CompoundStmt *>(stmt->getThen())) {
        stmt->getThen()->accept(this);
    } else {
        m_Cout << "\n";
        m_Indent++;
        stmt->getThen()->accept(this);
        m_Indent--;
    }

    if (stmt->getElse()) {
        m_Cout << multChar('\t', m_Indent) << "else ";
        if (dynamic_cast<CompoundStmt *>(stmt->getElse())) {
            stmt->getElse()->accept(this);
        } else {
            m_Cout << "\n";
            m_Indent++;
            stmt->getElse()->accept(this);
            m_Indent--;
        }
    }
}

void CCGN::visit(CaseStmt *stmt) {
    m_Cout << multChar('\t', m_Indent) << "case ";
    stmt->getPattern()->accept(this);
    m_Cout << ": ";
    if (dynamic_cast<CompoundStmt *>(stmt->getBody())) {
        stmt->getBody()->accept(this);
    } else {
        m_Cout << "\n";
        m_Indent++;
        stmt->getBody()->accept(this);
        m_Indent--;
    }

    m_Cout << multChar('\t', m_Indent + 1) << "break;\n";
}

void CCGN::visit(MatchStmt *stmt) {
    m_Cout << multChar('\t', m_Indent) << "switch (";
    stmt->getPattern()->accept(this);
    m_Cout << ") {\n";

    for (auto &C : stmt->getCases()) C->accept(this);

    if (stmt->getDefault()) {
        m_Cout << multChar('\t', m_Indent) << "default:\n";
        m_Indent++;
        stmt->getDefault()->accept(this);
        m_Cout << multChar('\t', m_Indent--) << "break;\n";
    }

    m_Cout << multChar('\t', m_Indent) << "}\n";
}

void CCGN::visit(RetStmt *stmt) {
    m_Cout << multChar('\t', m_Indent) << "return";
    if (stmt->getExpr()) {
        m_Cout << " ";
        stmt->m_Expr->accept(this);
    }

    m_Cout << ";\n";
}

void CCGN::visit(UntilStmt *stmt) {
    m_Cout << multChar('\t', m_Indent) << "while (!(";
    stmt->getCond()->accept(this);
    m_Cout << ")) ";
    if (dynamic_cast<CompoundStmt *>(stmt->getBody())) {
        stmt->getBody()->accept(this);
    } else {
        m_Cout << "\n";
        m_Indent++;
        stmt->getBody()->accept(this);
        m_Indent--;
    }
}

void CCGN::visit(IntegerLiteral *expr) {
    m_Cout << expr->getValue();
}

void CCGN::visit(FloatLiteral *expr) {
    m_Cout << expr->getValue() << "f";
}

void CCGN::visit(CharLiteral *expr) {
    m_Cout << "'" << expr->getValue() << "'";
}

void CCGN::visit(RefExpr *expr) {
    m_Cout << expr->getName();
}
