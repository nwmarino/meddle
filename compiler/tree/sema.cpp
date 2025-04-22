#include "sema.h"
#include "expr.h"
#include "stmt.h"
#include "unit.h"

using namespace meddle;

Sema::Sema(const Options &opts, TranslationUnit *U) : m_Opts(opts), m_Unit(U) {
    m_Unit->accept(this);
}

void Sema::visit(TranslationUnit *U) {
    for (auto &D : U->getDecls())
        D->accept(this);
}

void Sema::visit(FunctionDecl *decl) {
    for (auto &P : decl->getParams())
        P->accept(this);

    decl->m_Body->accept(this);
}

void Sema::visit(VarDecl *decl) {

}

void Sema::visit(ParamDecl *decl) {

}

void Sema::visit(CompoundStmt *stmt) {
    for (auto &S : stmt->getStmts())
        S->accept(this);
}

void Sema::visit(DeclStmt *stmt) {
    stmt->getDecl()->accept(this);
}

void Sema::visit(RetStmt *stmt) {
    stmt->getExpr()->accept(this);
}

void Sema::visit(IntegerLiteral *expr) {

}

void Sema::visit(RefExpr *expr) {
    NamedDecl *ref = expr->getRef();
    if (auto *VD = dynamic_cast<VarDecl *>(ref))
        expr->m_Type = VD->getType();
}
