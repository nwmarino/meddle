#include "ccgn.h"
#include "expr.h"
#include "stmt.h"
#include "type.h"
#include "unit.h"
#include "../core/logger.h"

#include <string>

using namespace meddle;

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
        }
    }

    return "";
}

CCGN::CCGN(const Options &opts, TranslationUnit *U) : m_Opts(opts), m_Unit(U) {
    m_Cout = std::ofstream(U->getFile().filename + ".c");
    m_Hout = std::ofstream(U->getFile().filename + ".h");

    phase = Phase::Declare;
    String noext = U->getFile().filename;
    auto pos = noext.find_last_of('.');
    if (pos != std::string::npos)
        noext = noext.substr(0, pos);

    emitHLn("#ifndef " + noext + "_H");
    emitHLn("#define " + noext + "_H\n");
    m_Unit->accept(this);
    emitHLn("\n#endif // " + noext + "_H");

    phase = Phase::Define;
    emitCLn("#include \"" + U->getFile().filename + ".h\"\n");
    m_Unit->accept(this);

    m_Cout.close();
    m_Hout.close();
}

void CCGN::visit(TranslationUnit *U) {
    for (auto &D : U->getDecls())
        D->accept(this);
}

void CCGN::visit(FunctionDecl *decl) {
    if (phase == Phase::Declare) {
        emitHSeg(translateType(decl->getReturnType()) + " " + decl->getName() + "(");
        for (auto &P : decl->getParams()) {
            P->accept(this);
            if (P != decl->getParams().back())
                emitHSeg(", ");
        }

        emitHLn(");");
    } else if (phase == Phase::Define) {
        emitCSeg(translateType(decl->getReturnType()) + " " + decl->getName() + "(");

        for (auto &P : decl->getParams()) {
            P->accept(this);
            if (P != decl->getParams().back())
                emitCSeg(", ");
        }

        emitCLn(")");

        decl->m_Body->accept(this);
    }
}

void CCGN::visit(VarDecl *decl) {
    if (phase == Phase::Declare) {
        
    } else if (phase == Phase::Define) {
        emitCSeg(translateType(decl->getType()) + " " + decl->getName());
        if (decl->getInit()) {
            emitCSeg(" = ");
            decl->getInit()->accept(this);
        }

        emitCLn(";");
    }
}

void CCGN::visit(ParamDecl *decl) {
    if (phase == Phase::Define)
        emitHSeg(translateType(decl->getType()) + " " + decl->getName());
    else if (phase == Phase::Declare)
        emitCSeg(translateType(decl->getType()) + " " + decl->getName());
}

void CCGN::visit(CompoundStmt *stmt) {
    emitCLn("{");
    for (auto &S : stmt->getStmts())
        S->accept(this);

    emitCLn("}");
}

void CCGN::visit(DeclStmt *stmt) {
    stmt->getDecl()->accept(this);
}

void CCGN::visit(RetStmt *stmt) {
    emitCSeg("return ");
    stmt->m_Expr->accept(this);
    emitCLn(";");
}

void CCGN::visit(IntegerLiteral *expr) {
    emitCSeg(std::to_string(expr->getValue()));
}

void CCGN::visit(RefExpr *expr) {
    emitCSeg(expr->getName());
}