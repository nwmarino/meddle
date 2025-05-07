#include "decl.h"
#include "expr.h"

using namespace meddle;

FunctionDecl *CallExpr::getCallee() const { 
    return static_cast<FunctionDecl *>(m_Ref);
}

UnitSpecExpr::UnitSpecExpr(const Metadata &M, UseDecl *U, RefExpr *E)
  : Expr(M, nullptr), m_Use(U), m_Expr(E), m_Unit(nullptr) {
    assert(U->isNamed() && "Cannot make specifier for unnamed use");
}

String UnitSpecExpr::getName() const {
    return m_Use->getName();
}
