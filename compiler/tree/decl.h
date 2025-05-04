#ifndef MEDDLE_DECL_H
#define MEDDLE_DECL_H

#include "expr.h"
#include "type.h"
#include "visitor.h"

#include <cassert>
#include <cstdint>

namespace meddle {

class Scope;
class Stmt;
class ParamDecl;

enum class Rune : uint8_t {
    NoMangle,
    Scoped,
};

struct Runes final {
    uint32_t bits = 0;

    void set(Rune attr) {
        bits |= (1 << static_cast<uint32_t>(attr));
    }

    void clear(Rune attr) {
        bits &= ~(1 << static_cast<uint32_t>(attr));
    }

    bool has(Rune attr) const {
        return bits & (1 << static_cast<uint32_t>(attr));
    }

    void clear() {
        bits = 0;
    }
};

class Decl {
protected:
    Runes m_Runes;
    Metadata m_Metadata;

public:
    Decl(const Runes &R, const Metadata &M) : m_Runes(R), m_Metadata(M) {}

    virtual ~Decl() = default;

    virtual void accept(Visitor *V) = 0;

    const Runes &getRunes() const { return m_Runes; }

    const Metadata &getMetadata() const { return m_Metadata; }
};

class NamedDecl : public Decl {
protected:
    String m_Name;

public:
    NamedDecl(const Runes &R, const Metadata &M, const String &N) 
      : Decl(R, M), m_Name(N) {}

    String getName() const { return m_Name; }
};

class FunctionDecl final : public NamedDecl {
    friend class CGN;
    friend class NameResolution;
    friend class Sema;

    FunctionType *m_Type;
    Scope *m_Scope;
    std::vector<ParamDecl *> m_Params;
    Stmt *m_Body;

public:
    FunctionDecl(const Runes &R, const Metadata &M, const String &N, 
                 FunctionType *T, Scope *S, std::vector<ParamDecl *> P, 
                 Stmt *B);

    ~FunctionDecl() override;

    void accept(Visitor *V) override { V->visit(this); }

    FunctionType *getType() const { return m_Type; }

    Type *getReturnType() const { return m_Type->getReturnType(); }

    Scope *getScope() const { return m_Scope; }

    const std::vector<ParamDecl *> &getParams() const { return m_Params; }

    unsigned getNumParams() const { return m_Params.size(); }

    ParamDecl *getParam(unsigned i) const {
        assert(i < m_Params.size());
        return m_Params[i];
    }

    Type *getParamType(unsigned i) const;

    Stmt *getBody() const { return m_Body; }

    bool empty() const { return m_Body == nullptr; }
};

class VarDecl : public NamedDecl {
    friend class CGN;
    friend class NameResolution;
    friend class Sema;

protected:
    Type *m_Type;
    Expr *m_Init;
    bool m_Mut;
    bool m_Global;

public:
    VarDecl(const Runes &R, const Metadata &M, const String &N, 
            Type *T, Expr *I, bool mut, bool global);

    ~VarDecl() override {
        if (hasInit())
            delete m_Init;
    }

    void accept(Visitor *V) override { V->visit(this); }

    Type *getType() const { return m_Type; }

    Expr *getInit() const { return m_Init; }

    bool hasInit() const { return m_Init != nullptr; }

    bool isMutable() const { return m_Mut; }

    bool isGlobal() const { return m_Global; }
};

class ParamDecl final : public VarDecl {
    friend class CGN;
    friend class NameResolution;
    friend class Sema;

    unsigned m_Index;
    FunctionDecl *m_Parent;

public:
    ParamDecl(const Runes &A, const Metadata &M, const String &N,
              Type *T, unsigned I);

    void accept(Visitor *V) override { V->visit(this); }

    unsigned getIndex() const { return m_Index; }

    FunctionDecl *getParent() const { return m_Parent; }

    void setParent(FunctionDecl *P) { m_Parent = P; }
};

class TypeDecl : public NamedDecl {
protected:
    Type *m_Type;

public:
    TypeDecl(const Runes &A, const Metadata &M, const String &N, Type *T)
      : NamedDecl(A, M, N), m_Type(T) {}

    virtual ~TypeDecl() = default;

    Type *getDefinedType() const { return m_Type; }

    void setDefinedType(Type *T) { m_Type = T; }
};

class EnumVariantDecl final : public NamedDecl {
    friend class CGN;
    friend class NameResolution;
    friend class Sema;

    Type *m_Type;
    long m_Value;

public:
    EnumVariantDecl(const Runes &R, const Metadata &M, const String &N, 
                    Type *T, long V)
      : NamedDecl(R, M, N), m_Type(T), m_Value(V) {}

    void accept(Visitor *V) override { V->visit(this); }

    Type *getType() const { return m_Type; }

    long getValue() const { return m_Value; }
};

class EnumDecl final : public TypeDecl {
    friend class CGN;
    friend class NameResolution;
    friend class Sema;

    std::vector<EnumVariantDecl *> m_Variants;

public:
    EnumDecl(const Runes &R, const Metadata &M, const String &N, 
             EnumType *T, std::vector<EnumVariantDecl *> V)
      : TypeDecl(R, M, N, T), m_Variants(V) {}

    ~EnumDecl() override {
        for (auto *V : m_Variants)
            delete V;
        m_Variants.clear();
    }

    void accept(Visitor *V) override { V->visit(this); }

    const std::vector<EnumVariantDecl *> &getVariants() const { return m_Variants; }

    unsigned getNumVariants() const { return m_Variants.size(); }

    EnumVariantDecl *getVariant(unsigned i) const {
        assert(i < m_Variants.size() && "Index out of range.");
        return m_Variants[i];
    }

    EnumVariantDecl *getVariant(const String &N) const {
        for (auto *V : m_Variants)
            if (V->getName() == N)
                return V;

        return nullptr;
    }
};

} // namespace meddle

#endif // MEDDLE_DECL_H
