#include "nameres.h"
#include "decl.h"
#include "expr.h"
#include "stmt.h"
#include "unit.h"
#include "../core/logger.h"

using namespace meddle;

NameResolution::NameResolution(const Options &opts, TranslationUnit *U) 
  : m_Opts(opts), m_Unit(U), m_Scope(U->getScope()) {
    U->accept(this);
}

void NameResolution::visit(TranslationUnit *U) {
    for (auto &D : U->getDecls())
        D->accept(this);
}

void NameResolution::visit(FunctionDecl *decl) {
    m_Scope = decl->getScope();
    for (auto &P : decl->getParams())
        P->accept(this);

    decl->m_Body->accept(this);
    m_Scope = m_Scope->getParent();
}

void NameResolution::visit(VarDecl *decl) {
    if (decl->getInit())
        decl->getInit()->accept(this);
}

void NameResolution::visit(ParamDecl *decl) {

}

void NameResolution::visit(BreakStmt *stmt) {}

void NameResolution::visit(ContinueStmt *stmt) {}

void NameResolution::visit(CompoundStmt *stmt) {
    m_Scope = stmt->getScope();
    for (auto &S : stmt->getStmts())
        S->accept(this);
    m_Scope = m_Scope->getParent();
}

void NameResolution::visit(DeclStmt *stmt) {
    stmt->getDecl()->accept(this);
}

void NameResolution::visit(ExprStmt *stmt) {
    stmt->getExpr()->accept(this);
}

void NameResolution::visit(IfStmt *stmt) {
    stmt->getCond()->accept(this);
    stmt->getThen()->accept(this);
    if (stmt->getElse())
        stmt->getElse()->accept(this);
}

void NameResolution::visit(CaseStmt *stmt) {
    stmt->getPattern()->accept(this);
    stmt->getBody()->accept(this);
}

void NameResolution::visit(MatchStmt *stmt) {
    stmt->getPattern()->accept(this);
    for (auto &C : stmt->getCases())
        C->accept(this);
    if (stmt->getDefault())
        stmt->getDefault()->accept(this);
}

void NameResolution::visit(RetStmt *stmt) {
    if (stmt->getExpr())
        stmt->getExpr()->accept(this);
}

void NameResolution::visit(UntilStmt *stmt) {
    stmt->getCond()->accept(this);
    stmt->getBody()->accept(this);
}

void NameResolution::visit(IntegerLiteral *expr) {}

void NameResolution::visit(FloatLiteral *expr) {}

void NameResolution::visit(CharLiteral *expr) {}

void NameResolution::visit(StringLiteral *expr) {}

void NameResolution::visit(NilLiteral *expr) {}

void NameResolution::visit(CastExpr *expr) {
    expr->m_Expr->accept(this);
}

void NameResolution::visit(RefExpr *expr) {
    NamedDecl *ND = expr->getRef();
    assert(ND && "Reference not resolved.");

    if (auto *VD = dynamic_cast<VarDecl *>(ND))
        expr->m_Type = VD->getType();
    else
        fatal("bad reference", &expr->getMetadata());
}
