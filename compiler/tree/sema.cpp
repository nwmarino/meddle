#include "sema.h"
#include "expr.h"
#include "stmt.h"
#include "unit.h"
#include "../core/logger.h"

using namespace meddle;

static void checkCompoundedDeclStmt(Stmt *S) {
    if (dynamic_cast<CompoundStmt *>(S))
        return;

    if (dynamic_cast<DeclStmt *>(S))
        fatal("declarations must be in a compound statement", &S->getMetadata());
}

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
    if (decl->getInit())
        decl->getInit()->accept(this);
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

void Sema::visit(IfStmt *stmt) {
    stmt->getCond()->accept(this);

    stmt->getThen()->accept(this);
    checkCompoundedDeclStmt(stmt->getThen());

    if (stmt->getElse()) {
        stmt->getElse()->accept(this);
        checkCompoundedDeclStmt(stmt->getElse());
    }
}

void Sema::visit(RetStmt *stmt) {
    if (stmt->getExpr())
        stmt->getExpr()->accept(this);
}

void Sema::visit(IntegerLiteral *expr) {

}

void Sema::visit(FloatLiteral *expr) {

}

void Sema::visit(CharLiteral *expr) {
    
}

void Sema::visit(RefExpr *expr) {
    NamedDecl *ref = expr->getRef();
    if (auto *VD = dynamic_cast<VarDecl *>(ref))
        expr->m_Type = VD->getType();
}
