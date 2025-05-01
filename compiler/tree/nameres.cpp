#include "nameres.h"
#include "decl.h"
#include "expr.h"
#include "stmt.h"
#include "unit.h"
#include "../core/logger.h"

using namespace meddle;

static Type *unwrapType(Type *T) {
    assert(T && "Cannot unwrap null types.");
    
    if (T->isQualified())
        return T;

    return static_cast<TypeResult *>(T)->getUnderlying();
}

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

    decl->getBody()->accept(this);
    m_Scope = m_Scope->getParent();
}

void NameResolution::visit(VarDecl *decl) {
    if (decl->getInit())
        decl->getInit()->accept(this);

    if (!decl->m_Type) {
        assert(decl->getInit() != nullptr && "Cannot infer type.");
        decl->m_Type = decl->getInit()->getType();
    } else {
        decl->m_Type = unwrapType(decl->getType());
    }
}

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

void NameResolution::visit(CastExpr *expr) {
    expr->getExpr()->accept(this);
    expr->m_Type = unwrapType(expr->m_Type);
}

void NameResolution::visit(ParenExpr *expr) {
    expr->m_Expr->accept(this);
    expr->m_Type = expr->getExpr()->getType();
}

void NameResolution::visit(RefExpr *expr) {
    NamedDecl *ND = expr->getRef();
    assert(ND && "Reference not resolved.");

    if (auto *VD = dynamic_cast<VarDecl *>(ND))
        expr->m_Type = VD->getType();
    else
        fatal("bad reference", &expr->getMetadata());
}

void NameResolution::visit(SizeofExpr *expr) {
    expr->m_Target = unwrapType(expr->getTarget());
}
