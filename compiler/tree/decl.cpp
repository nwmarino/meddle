#include "decl.h"
#include "scope.h"
#include "stmt.h"

using namespace meddle;

FunctionDecl::FunctionDecl(const Attributes &A, const Metadata &M, 
                           const String &N, FunctionType *T, Scope *S, 
                           std::vector<ParamDecl *> P, Stmt *B)
    : NamedDecl(A, M, N), m_Type(T), m_Scope(S), m_Params(std::move(P)),
      m_Body(B) 
{
    for (auto &Pr : P)
        Pr->setParent(this);
}

FunctionDecl::~FunctionDecl() {
    for (auto &P : m_Params)
        delete P;

    if (m_Body)
        delete m_Body;
    
    delete m_Scope;
}

Type *FunctionDecl::getParamType(unsigned i) const {
    assert(i <= m_Params.size());
    return m_Params[i]->getType();
}

VarDecl::VarDecl(const Attributes &A, const Metadata &M, const String &N, 
                 Type *T, Expr *I, bool mut, bool global)
    : NamedDecl(A, M, N), m_Type(T), m_Init(I), m_Mut(mut), m_Global(global) {}

VarDecl::~VarDecl() {
    delete m_Init;
}

ParamDecl::ParamDecl(const Attributes &A, const Metadata &M, const String &N,
                     Type *T, unsigned I)
    : VarDecl(A, M, N, T, nullptr, true, false), m_Index(I), 
      m_Parent(nullptr) {}
