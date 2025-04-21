#ifndef MEDDLE_DECL_H
#define MEDDLE_DECL_H

#include "expr.h"
#include "type.h"

#include <cassert>

namespace meddle {

class Scope;
class Stmt;
class ParamDecl;

struct Attributes final {
    unsigned no_mangle:1;
};

class Decl {
protected:
    Attributes m_Attrs;
    Metadata m_Metadata;

public:
    Decl(const Attributes &A, const Metadata &M) : m_Attrs(A), m_Metadata(M) {}
    virtual ~Decl() = default;

    const Attributes &getAttributes() const { return m_Attrs; }

    const Metadata &getMetadata() const { return m_Metadata; }
};

class NamedDecl : public Decl {
protected:
    String m_Name;

public:
    NamedDecl(const Attributes &A, const Metadata &M, const String &N) 
      : Decl(A, M), m_Name(N) {}

    virtual ~NamedDecl() = default;

    String getName() const { return m_Name; }
};

class FunctionDecl final : public NamedDecl {
    FunctionType *m_Type;
    Scope *m_Scope;
    std::vector<ParamDecl *> m_Params;
    Stmt *m_Body;

public:
    FunctionDecl(const Attributes &A, const Metadata &M, const String &N, 
                 FunctionType *T, Scope *S, std::vector<ParamDecl *> P, 
                 Stmt *B);

    ~FunctionDecl() override;

    template<typename vT>
    void accept(vT &visitor) {
        visitor.visit(this);
    }

    FunctionType *getType() const { return m_Type; }

    Type *getReturnType() const { return m_Type->getReturnType(); }

    Scope *getScope() const { return m_Scope; }

    const std::vector<ParamDecl *> &getParams() const { return m_Params; }

    ParamDecl *getParam(unsigned i) const {
        assert(i < m_Params.size());
        return m_Params[i];
    }

    Type *getParamType(unsigned i) const;

    Stmt *getBody() const { return m_Body; }
};

class VarDecl : public NamedDecl {
protected:
    Type *m_Type;
    Expr *m_Init;
    bool m_Mut;
    bool m_Global;

public:
    VarDecl(const Attributes &A, const Metadata &M, const String &N, 
            Type *T, Expr *I, bool mut, bool global);

    ~VarDecl() override;

    template<typename vT>
    void accept(vT &visitor) {
        visitor.visit(this);
    }

    Type *getType() const { return m_Type; }

    Expr *getInit() const { return m_Init; }

    bool isMutable() const { return m_Mut; }

    bool isGlobal() const { return m_Global; }
};

class ParamDecl final : public VarDecl {
    unsigned m_Index;
    FunctionDecl *m_Parent;

public:
    ParamDecl(const Attributes &A, const Metadata &M, const String &N,
              Type *T, unsigned I);

    template<typename vT>
    void accept(vT &visitor) {
        visitor.visit(this);
    }

    unsigned getIndex() const { return m_Index; }

    FunctionDecl *getParent() const { return m_Parent; }

    void setParent(FunctionDecl *P) { m_Parent = P; }
};

} // namespace meddle

#endif // MEDDLE_DECL_H
