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
class StructDecl;

enum class Rune : uint8_t {
    Associated,
    NoMangle,
};

struct Runes final {
    uint32_t bits = 0;

    void set(Rune rune) {
        bits |= (1 << static_cast<uint32_t>(rune));
    }

    void clear(Rune rune) {
        bits &= ~(1 << static_cast<uint32_t>(rune));
    }

    bool has(Rune rune) const {
        return bits & (1 << static_cast<uint32_t>(rune));
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
    StructDecl *m_Parent;

public:
    FunctionDecl(const Runes &R, const Metadata &M, const String &N, 
                 FunctionType *T, Scope *S, std::vector<ParamDecl *> P, 
                 Stmt *B, StructDecl *SP = nullptr);

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

    StructDecl *getParent() const { return m_Parent; }

    void setParent(StructDecl *P) { m_Parent = P; }

    bool hasParent() const { return m_Parent != nullptr; }

    bool isMethod() const 
    { return hasParent() && !m_Runes.has(Rune::Associated); }
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

    const std::vector<EnumVariantDecl *> &getVariants() const 
    { return m_Variants; }

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

class FieldDecl final : public NamedDecl {
    friend class CGN;
    friend class NameResolution;
    friend class Sema;

    Type *m_Type;
    Expr *m_Init;
    unsigned m_Index;
    StructDecl *m_Parent;

public:
    FieldDecl(const Runes &R, const Metadata &M, const String &N, Type *T, 
              unsigned Idx, Expr *I = nullptr, StructDecl *P = nullptr)
      : NamedDecl(R, M, N), m_Type(T), m_Init(I), m_Index(Idx), m_Parent(P) {}

    ~FieldDecl() override {
        if (m_Init)
            delete m_Init;
    }

    void accept(Visitor *V) override { V->visit(this); }

    Type *getType() const { return m_Type; }

    Expr *getInit() const { return m_Init; }

    bool hasInit() const { return m_Init != nullptr; }

    unsigned getIndex() const { return m_Index; }

    StructDecl *getParent() const { return m_Parent; }

    void setParent(StructDecl *P) { m_Parent = P; }
};

class StructDecl final : public TypeDecl {
    friend class CGN;
    friend class NameResolution;
    friend class Sema;

    Scope *m_Scope;
    std::vector<FieldDecl *> m_Fields;
    std::vector<FunctionDecl *> m_Functions;

public:
    StructDecl(const Runes &R, const Metadata &M, const String &N, 
               StructType *T, Scope *S, std::vector<FieldDecl *> F, 
               std::vector<FunctionDecl *> Funcs);

    ~StructDecl() override;

    void accept(Visitor *V) override { V->visit(this); }

    Scope *getScope() const { return m_Scope; }

    const std::vector<FieldDecl *> &getFields() const { return m_Fields; }

    unsigned getNumFields() const { return m_Fields.size(); }

    FieldDecl *getField(unsigned i) const {
        assert(i < m_Fields.size());
        return m_Fields[i];
    }

    FieldDecl *getField(const String &N) const {
        for (auto *F : m_Fields)
            if (F->getName() == N)
                return F;

        return nullptr;
    }

    const std::vector<FunctionDecl *> &getFunctions() const 
    { return m_Functions; }

    FunctionDecl *getFunction(const String &N) const {
        for (auto *F : m_Functions)
            if (F->getName() == N)
                return F;

        return nullptr;
    }

    unsigned getNumFunctions() const { return m_Functions.size(); }

    FunctionDecl *getFunction(unsigned i) const {
        assert(i < m_Functions.size());
        return m_Functions[i];
    }
};

} // namespace meddle

#endif // MEDDLE_DECL_H
