#include "nameres.h"
#include "expr.h"
#include "stmt.h"
#include "unit.h"

using namespace meddle;

NameResolution::NameResolution(const Options &opts, TranslationUnit *U) 
  : m_Opts(opts), m_Unit(U) {
    U->accept(this);
}

void NameResolution::visit(TranslationUnit *U) {
    for (auto &D : U->getDecls())
        D->accept(this);
}

void NameResolution::visit(FunctionDecl *decl) {
    for (auto &P : decl->getParams())
        P->accept(this);

    decl->m_Body->accept(this);
}

void NameResolution::visit(VarDecl *decl) {
    if (decl->getInit())
        decl->getInit()->accept(this);
}

void NameResolution::visit(ParamDecl *decl) {

}

void NameResolution::visit(CompoundStmt *stmt) {
    for (auto &S : stmt->getStmts())
        S->accept(this);
}

void NameResolution::visit(DeclStmt *stmt) {
    stmt->getDecl()->accept(this);
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

void NameResolution::visit(IntegerLiteral *expr) {

}

void NameResolution::visit(FloatLiteral *expr) {
    
}

void NameResolution::visit(CharLiteral *expr) {
    
}

void NameResolution::visit(RefExpr *expr) {

}
