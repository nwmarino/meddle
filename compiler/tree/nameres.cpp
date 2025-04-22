#include "nameres.h"
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

}

void NameResolution::visit(ParamDecl *decl) {

}

void NameResolution::visit(CompoundStmt *stmt) {
    for (auto &S : stmt->getStmts())
        S->accept(this);
}

void NameResolution::visit(RetStmt *stmt) {

}

void NameResolution::visit(IntegerLiteral *expr) {

}
