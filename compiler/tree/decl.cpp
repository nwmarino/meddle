#include "decl.h"
#include "scope.h"
#include "stmt.h"

using namespace meddle;

FunctionDecl::FunctionDecl(const Runes &R, const Metadata &M, 
                           const String &N, FunctionType *T, Scope *S, 
                           std::vector<ParamDecl *> P, Stmt *B,
                           StructDecl *SP)
    : NamedDecl(R, M, N), m_Type(T), m_Scope(S), m_Params(std::move(P)),
      m_Body(B), m_Parent(SP) 
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

VarDecl::VarDecl(const Runes &R, const Metadata &M, const String &N, 
                 Type *T, Expr *I, bool mut, bool global)
    : NamedDecl(R, M, N), m_Type(T), m_Init(I), m_Mut(mut), m_Global(global) {}

ParamDecl::ParamDecl(const Runes &R, const Metadata &M, const String &N,
                     Type *T, unsigned I)
    : VarDecl(R, M, N, T, nullptr, true, false), m_Index(I), 
      m_Parent(nullptr) {}

StructDecl::StructDecl(const Runes &R, const Metadata &M, const String &N, 
                       StructType *T, Scope *S, std::vector<FieldDecl *> F, 
                       std::vector<FunctionDecl *> Funcs)
    : TypeDecl(R, M, N, T), m_Scope(S), m_Fields(std::move(F)),
      m_Functions(std::move(Funcs))
{
    for (auto &F : m_Fields)
        F->setParent(this);
    for (auto &F : m_Functions)
        F->setParent(this);
}

StructDecl::~StructDecl() {
    for (auto *F : m_Fields)
        delete F;
    for (auto *F : m_Functions)
        delete F;

    delete m_Scope;
}
