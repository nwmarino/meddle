#include "decl.h"
#include "expr.h"

using namespace meddle;

FunctionDecl *CallExpr::getCallee() const { 
    return static_cast<FunctionDecl *>(m_Ref);
}
