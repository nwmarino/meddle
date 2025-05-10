#include "decl.h"
#include "expr.h"
#include "stmt.h"
#include "unit.h"

#include <iostream>
#include <ostream>

using namespace meddle;

static unsigned g_Indent = 0;

static String stringify_metadata(const Metadata &md) {
    return md.file.filename + "<" + std::to_string(md.line) + ":" + 
           std::to_string(md.col) + ">";
}

static String stringify_indent() {
    String result;
    for (unsigned i = 0; i != g_Indent; ++i)
        result += "  ";
    return result;
}

void TranslationUnit::print(std::ostream &OS) const {
    g_Indent = 0;

    OS << "TranslationUnit (" << m_Decls.size() << ") " << m_File.filename 
       << "\n";

    g_Indent++;

    for (auto &use : m_Uses)
        use->print(OS);

    for (auto &decl : m_Decls)
        decl->print(OS);

    g_Indent--;
}

void FunctionDecl::print(std::ostream &OS) const {
    OS << stringify_indent() << "FunctionDecl";
    
    if (isTemplate()) {
        OS << "<";
        for (auto &param : m_TemplateParams)
            OS << param->getName() << (param == m_TemplateParams.back() ? "" : ", ");
        OS << ">";
    }

    OS << " " << stringify_metadata(m_Metadata) << " '" << m_Type->getName() 
       << "' " << m_Name << "\n";

    g_Indent++;
    for (auto &param : m_TemplateParams)
        param->print(OS);

    for (auto &param : m_Params)
        param->print(OS);

    if (m_Body)
        m_Body->print(OS);

    for (auto &spec : m_TemplateSpecs)
        spec->print(OS);

    g_Indent--;
}

void FunctionTemplateSpecializationDecl::print(std::ostream &OS) const {
    OS << stringify_indent() << "FunctionSpecializationDecl<";
    
    for (auto &arg : m_Args) {
        OS << arg->getName() << (arg == m_Args.back() ? "" : ", ");
    }

    OS << "> '" << m_Type->getName() << "' " << m_Name << "\n";

    g_Indent++;
    for (auto &param : m_Params)
        param->print(OS);

    if (m_Body)
        m_Body->print(OS);

    g_Indent--;
}

void VarDecl::print(std::ostream &OS) const {
    OS << stringify_indent() << "VarDecl " << stringify_metadata(m_Metadata) 
       << " " << m_Name << " '" << m_Type->getName() << "' " << "\n";

    if (m_Init) {
        g_Indent++;
        m_Init->print(OS);
        g_Indent--;
    }
}

void ParamDecl::print(std::ostream &OS) const {
    OS << stringify_indent() << "ParamDecl " << stringify_metadata(m_Metadata) 
       << " " << m_Name << " '" << m_Type->getName() << "' " << "\n";
}

void UseDecl::print(std::ostream &OS) const {
    OS << stringify_indent() << "UseDecl " << stringify_metadata(m_Metadata) 
       << " " << m_Path << " ";

    if (isNamed())
        OS << m_Name;
    else if (!m_Symbols.empty()) {
        for (auto &name : m_Symbols)
            OS << name << (name == m_Symbols.back() ? "" : ", ");
    }

    OS << "\n";
}

void EnumVariantDecl::print(std::ostream &OS) const {
    OS << stringify_indent() << "EnumVariantDecl " << stringify_metadata(m_Metadata)
       << " " << m_Name << " '" << m_Type->getName() << "' = " << m_Value << "\n";
}

void EnumDecl::print(std::ostream &OS) const {
    OS << stringify_indent() << "EnumDecl (" << std::to_string(m_Variants.size()) 
       << ") " << stringify_metadata(m_Metadata) << " " << m_Name << "\n";

    g_Indent++;
    for (auto &variant : m_Variants)
        variant->print(OS);

    g_Indent--;
}

void FieldDecl::print(std::ostream &OS) const {
    OS << stringify_indent() << "FieldDecl " << stringify_metadata(m_Metadata) 
       << " " << m_Name << " '" << m_Type->getName() << "'\n";

    if (m_Init) {
        g_Indent++;
        m_Init->print(OS);
        g_Indent--;
    }
}

void StructDecl::print(std::ostream &OS) const {
    OS << stringify_indent() << "StructDecl (" << std::to_string(m_Fields.size()) 
       << ") " << stringify_metadata(m_Metadata) << " " << m_Name << "\n";

    g_Indent++;

    for (auto &field : m_Fields)
        field->print(OS);

    for (auto &fn : m_Functions)
        fn->print(OS);

    for (auto &spec : m_TemplateSpecs)
        spec->print(OS);

    g_Indent--;
}

void StructTemplateSpecializationDecl::print(std::ostream &OS) const {
    OS << stringify_indent() << "StructSpecializationDecl<";
    
    for (auto &arg : m_Args) {
        OS << arg->getName() << (arg == m_Args.back() ? "" : ", ");
    }

    OS << "> '" << m_Type->getName() << "' " << m_Name << "\n";

    g_Indent++;

    for (auto &field : m_Fields)
        field->print(OS);

    for (auto &fn : m_Functions)
        fn->print(OS);

    g_Indent--;
}

void TemplateParamDecl::print(std::ostream &OS) const {
    OS << stringify_indent() << "TemplateParamDecl " << stringify_metadata(m_Metadata) 
       << " " << m_Name << " '" << m_Type->getName() << "'\n";
}

void BreakStmt::print(std::ostream &OS) const {
    OS << stringify_indent() << "BreakStmt " << stringify_metadata(m_Metadata) 
       << "\n";
}

void ContinueStmt::print(std::ostream &OS) const {
    OS << stringify_indent() << "ContinueStmt " << stringify_metadata(m_Metadata) 
       << "\n";
}

void CompoundStmt::print(std::ostream &OS) const {
    OS << stringify_indent() << "CompoundStmt (" << m_Stmts.size() << ") " 
       << stringify_metadata(m_Metadata) << "\n";

    g_Indent++;
    for (auto &stmt : m_Stmts)
        stmt->print(OS);
    g_Indent--;
}

void DeclStmt::print(std::ostream &OS) const {
    OS << stringify_indent() << "DeclStmt " << stringify_metadata(m_Metadata) 
       << "\n";

    g_Indent++;
    m_Decl->print(OS);
    g_Indent--;
}

void ExprStmt::print(std::ostream &OS) const {
    OS << stringify_indent() << "ExprStmt " << stringify_metadata(m_Metadata) 
       << "\n";

    g_Indent++;
    m_Expr->print(OS);
    g_Indent--;
}

void IfStmt::print(std::ostream &OS) const {
    OS << stringify_indent() << "IfStmt " << stringify_metadata(m_Metadata) 
       << "\n";

    g_Indent++;
    m_Cond->print(OS);
    m_Then->print(OS);

    if (m_Else)
        m_Else->print(OS);

    g_Indent--;
}

void CaseStmt::print(std::ostream &OS) const {
    OS << stringify_indent() << "CaseStmt " << stringify_metadata(m_Metadata) 
       << "\n";

    g_Indent++;
    m_Pattern->print(OS);
    m_Body->print(OS);
    g_Indent--;
}

void MatchStmt::print(std::ostream &OS) const {
    OS << stringify_indent() << "MatchStmt " << stringify_metadata(m_Metadata) 
       << "\n";

    g_Indent++;
    m_Pattern->print(OS);

    for (auto &C : m_Cases)
        C->print(OS);

    if (m_Default)
        m_Default->print(OS);

    g_Indent--;
}

void RetStmt::print(std::ostream &OS) const {
    OS << stringify_indent() << "RetStmt " << stringify_metadata(m_Metadata) 
       << "\n";

    g_Indent++;
    if (m_Expr)
        m_Expr->print(OS);
    g_Indent--;
}

void UntilStmt::print(std::ostream &OS) const {
    OS << stringify_indent() << "UntilStmt " << stringify_metadata(m_Metadata) 
       << "\n";

    g_Indent++;
    m_Cond->print(OS);
    m_Body->print(OS);
    g_Indent--;
}

void BoolLiteral::print(std::ostream &OS) const {
    OS << stringify_indent() << "BoolLiteral " << stringify_metadata(m_Metadata) 
       << " " << m_Value << "\n";
}

void IntegerLiteral::print(std::ostream &OS) const {
    OS << stringify_indent() << "IntegerLiteral " << stringify_metadata(m_Metadata) 
       << " " << m_Value << "\n";
}

void FloatLiteral::print(std::ostream &OS) const {
    OS << stringify_indent() << "FloatLiteral " << stringify_metadata(m_Metadata) 
       << " " << m_Value << "\n";
}

void CharLiteral::print(std::ostream &OS) const {
    OS << stringify_indent() << "CharLiteral " << stringify_metadata(m_Metadata) 
       << " " << m_Value << "\n";
}

void StringLiteral::print(std::ostream &OS) const {
    OS << stringify_indent() << "StringLiteral " << stringify_metadata(m_Metadata) 
       << " '" << m_Value << "'\n";
}

void NilLiteral::print(std::ostream &OS) const {
    OS << stringify_indent() << "NilLiteral " << stringify_metadata(m_Metadata) 
       << "\n";
}

void ArrayExpr::print(std::ostream &OS) const {
    OS << stringify_indent() << "ArrayExpr (" << m_Elements.size() << ") " 
       << stringify_metadata(m_Metadata) << "\n";

    g_Indent++;
    for (auto &E : m_Elements)
        E->print(OS);
    g_Indent--;
}

void BinaryExpr::print(std::ostream &OS) const {
    OS << stringify_indent() << "BinaryExpr " << stringify_metadata(m_Metadata) 
       << " '" << m_Type->getName() << "' " << "\n";

    g_Indent++;
    m_LHS->print(OS);
    m_RHS->print(OS);
    g_Indent--;
}

void CastExpr::print(std::ostream &OS) const {
    OS << stringify_indent() << "CastExpr " << stringify_metadata(m_Metadata) 
       << " '" << m_Type->getName() << "'\n";

    g_Indent++;
    m_Expr->print(OS);
    g_Indent--;
}

void ParenExpr::print(std::ostream &OS) const {
    OS << stringify_indent() << "ParenExpr " << stringify_metadata(m_Metadata) 
       << "\n";

    g_Indent++;
    m_Expr->print(OS);
    g_Indent--;
}

void RefExpr::print(std::ostream &OS) const {
    OS << stringify_indent() << "RefExpr " << stringify_metadata(m_Metadata) 
       << " '" << m_Type->getName() << "' " << m_Name << "\n";
}

void AccessExpr::print(std::ostream &OS) const {
    OS << stringify_indent() << "AccessExpr " << stringify_metadata(m_Metadata) 
       << " '" << m_Type->getName() << "' " << m_Name << "\n";

    g_Indent++;
    m_Base->print(OS);
    g_Indent--;
}

void CallExpr::print(std::ostream &OS) const {
    OS << stringify_indent() << "CallExpr " << stringify_metadata(m_Metadata) 
       << " '" << m_Type->getName() << "' " << m_Name << "\n";

    g_Indent++;
    for (auto &arg : m_Args)
        arg->print(OS);

    for (auto &arg : m_TypeArgs)
        OS << arg->getName() << (arg == m_TypeArgs.back() ? "" : ", ");

    g_Indent--;
}

void MethodCallExpr::print(std::ostream &OS) const {
    OS << stringify_indent() << "MethodCallExpr " << stringify_metadata(m_Metadata) 
       << " '" << m_Type->getName() << "' " << m_Name << "\n";

    g_Indent++;
    m_Base->print(OS);

    for (auto &arg : m_Args)
        arg->print(OS);

    for (auto &arg : m_TypeArgs)
        OS << arg->getName() << (arg == m_TypeArgs.back() ? "" : ", ");

    g_Indent--;
}


void FieldInitExpr::print(std::ostream &OS) const {
    OS << stringify_indent() << "FieldInitExpr " << stringify_metadata(m_Metadata) 
       << " '" << m_Type->getName() << "' " << m_Name << "\n";

    g_Indent++;
    m_Expr->print(OS);
    g_Indent--;
}

void InitExpr::print(std::ostream &OS) const {
    OS << stringify_indent() << "InitExpr (" << m_Fields.size() << ") " 
       << stringify_metadata(m_Metadata) << "\n";

    g_Indent++;
    for (auto &F : m_Fields)
        F->print(OS);
    g_Indent--;
}

void SizeofExpr::print(std::ostream &OS) const {
    OS << stringify_indent() << "SizeofExpr " << stringify_metadata(m_Metadata) 
       << " '" << m_Type->getName() << "' " << m_Target->getName() << "\n";
}

void SubscriptExpr::print(std::ostream &OS) const {
    OS << stringify_indent() << "SubscriptExpr " << stringify_metadata(m_Metadata) 
       << " '" << m_Type->getName() << "'\n";

    g_Indent++;
    m_Base->print(OS);
    m_Index->print(OS);
    g_Indent--;
}

void TypeSpecExpr::print(std::ostream &OS) const {
    OS << stringify_indent() << "TypeSpecExpr " << stringify_metadata(m_Metadata) 
       << " '" << m_Type->getName() << "' " << m_Name << "\n";

    g_Indent++;
    m_Expr->print(OS);
    g_Indent--;
}

void UnitSpecExpr::print(std::ostream &OS) const {
    OS << stringify_indent() << "UnitSpecExpr " << stringify_metadata(m_Metadata) 
       << " '" << m_Type->getName() << "' " << m_Use->getName() << "\n";

    g_Indent++;
    m_Expr->print(OS);
    g_Indent--;
}

void UnaryExpr::print(std::ostream &OS) const {
    OS << stringify_indent() << "UnaryExpr " << stringify_metadata(m_Metadata) 
       << " '" << m_Type->getName() << "' " << "\n";

    g_Indent++;
    m_Expr->print(OS);
    g_Indent--;
}

void RuneSyscallExpr::print(std::ostream &OS) const {
    OS << stringify_indent() << "RuneSyscallExpr " << stringify_metadata(m_Metadata) 
       << " '" << m_Type->getName() << "' " << m_Num << "\n";

    g_Indent++;
    for (auto &arg : m_Args)
        arg->print(OS);
    g_Indent--;
}
