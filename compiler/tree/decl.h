#ifndef MEDDLE_DECL_H
#define MEDDLE_DECL_H

#include "expr.h"
#include "type.h"
#include "visitor.h"

#include <cassert>
#include <cstdint>
#include <unordered_map>

namespace meddle {

class Scope;
class Stmt;
class ParamDecl;
class StructDecl;
class TemplateDecl;
class FunctionTemplateSpecializationDecl;
class StructTemplateSpecializationDecl;

enum class Rune : uint8_t {
    Associated,
    NoMangle,
    Public,
};

struct Runes final {
    uint32_t bits = 0;

    void set(Rune rune) 
    { bits |= (1 << static_cast<uint32_t>(rune)); }

    void clear(Rune rune) 
    { bits &= ~(1 << static_cast<uint32_t>(rune)); }

    bool has(Rune rune) const 
    { return bits & (1 << static_cast<uint32_t>(rune)); }

    void clear() { bits = 0; }
};

class Decl {
protected:
    Runes m_Runes;
    Metadata m_Metadata;
    TranslationUnit *m_PUnit = nullptr;

public:
    Decl(const Runes &R, const Metadata &M) : m_Runes(R), m_Metadata(M) {}

    virtual ~Decl() = default;

    virtual void accept(Visitor *V) = 0;

    const Runes &getRunes() const { return m_Runes; }

    const Metadata &getMetadata() const { return m_Metadata; }

    TranslationUnit *getPUnit() const { return m_PUnit; }

    void setPUnit(TranslationUnit *U) { m_PUnit = U; }

    bool hasPublicRune() const { return m_Runes.has(Rune::Public); }
};

class NamedDecl : public Decl {
protected:
    String m_Name;

public:
    NamedDecl(const Runes &R, const Metadata &M, const String &N) 
      : Decl(R, M), m_Name(N) {}

    String getName() const { return m_Name; }
};

class FunctionDecl : public NamedDecl {
    friend class CGN;
    friend class NameResolution;
    friend class Sema;

protected:
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

class UseDecl final : public NamedDecl {
    friend class CGN;
    friend class NameResolution;
    friend class Sema;

    /// If this `use` imports only specific symbols, i.e.
    ///
    /// use { Foo, Bar } = ...
    std::vector<String> m_Symbols;
    String m_Path;
    TranslationUnit *m_Unit;

public:
    UseDecl(const Runes &R, const Metadata &M, const String &P)
      : NamedDecl(R, M, ""), m_Path(P), m_Unit(nullptr) {}

    UseDecl(const Runes &R, const Metadata &M, const String &P, const String &N)
      : NamedDecl(R, M, N), m_Path(P), m_Unit(nullptr) {}

    UseDecl(const Runes &R, const Metadata &M, const String &P, 
            std::vector<String> S)
      : NamedDecl(R, M, ""), m_Path(P), m_Symbols(S), m_Unit(nullptr) {}

    ~UseDecl() override = default;
    
    void accept(Visitor *V) override { V->visit(this); }

    bool isNamed() const { return !m_Name.empty(); }

    const String &getPath() const { return m_Path; }

    const std::vector<String> &getSymbols() const { return m_Symbols; }

    TranslationUnit *getUnit() const { return m_Unit; }

    void setUnit(TranslationUnit *U) { m_Unit = U; }
};

class TypeDecl : public NamedDecl {
protected:
    Type *m_Type;

public:
    TypeDecl(const Runes &R, const Metadata &M, const String &N, Type *T)
      : NamedDecl(R, M, N), m_Type(T) {}

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

class StructDecl : public TypeDecl {
    friend class CGN;
    friend class NameResolution;
    friend class Sema;

protected:
    Scope *m_Scope;
    std::vector<FieldDecl *> m_Fields;
    std::vector<FunctionDecl *> m_Functions;

public:
    StructDecl(const Runes &R, const Metadata &M, const String &N, 
               Type *T, Scope *S, std::vector<FieldDecl *> F, 
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

class TemplateParamDecl final : public TypeDecl {
    friend class CGN;
    friend class NameResolution;
    friend class Sema;

    TemplateDecl *m_Parent;
    unsigned m_Index;

public:
    TemplateParamDecl(const Runes &R, const Metadata &M, const String &N, 
                      TemplateParamType *T, TemplateDecl *P, unsigned I)
      : TypeDecl(R, M, N, T), m_Parent(P), m_Index(I) {}

    ~TemplateParamDecl() override = default;

    void accept(Visitor *V) override { V->visit(this); }

    TemplateDecl *getParentTemplate() const { return m_Parent; }

    void setParentTemplate(TemplateDecl *P) { m_Parent = P; }

    unsigned getIndex() const { return m_Index; }
};

class TemplateDecl : public NamedDecl {
    friend class CGN;
    friend class NameResolution;
    friend class Sema;

protected:
    NamedDecl *m_Tmpl;
    std::vector<TemplateParamDecl *> m_Params;

    Type*
    substType(Type *T, std::unordered_map<TemplateParamType *, Type *>) const;

public:
    TemplateDecl(const Runes &R, const Metadata &M, const String &N, 
                 NamedDecl *T, std::vector<TemplateParamDecl *> P)
      : NamedDecl(R, M, N), m_Tmpl(T), m_Params(P) {}

    ~TemplateDecl() override {
        for (auto &P : m_Params)
            delete P;
        m_Params.clear();
        delete m_Tmpl;
    }

    NamedDecl *getTemplatedDecl() const { return m_Tmpl; }

    TemplateParamDecl *getParam(unsigned i) const {
        assert(i < m_Params.size() && "Index out of range.");
        return m_Params.at(i);
    }

    const std::vector<TemplateParamDecl *> &getParams() const
    { return m_Params; }

    String getConcreteName(const std::vector<Type *> &args) const {
        String result = m_Tmpl->getName() + "<";
        for (unsigned i = 0; i < args.size(); ++i)
            result += args.at(i)->getName() + 
                (i != args.size() - 1 ? ", " : "");
    
        return result;
    }
};

class TemplateStructDecl final : public TemplateDecl {
    friend class CGN;
    friend class NameResolution;
    friend class Sema;

    std::vector<StructTemplateSpecializationDecl *> m_Specs = {};

public:
    TemplateStructDecl(const Runes &R, const Metadata &M, const String &N,
                       StructDecl *S, std::vector<TemplateParamDecl *> P)
      : TemplateDecl(R, M, N, S, P) {}

    ~TemplateStructDecl() override;

    void accept(Visitor *V) override { V->visit(this); }

    StructDecl *getTemplatedStruct() const 
    { return static_cast<StructDecl *>(m_Tmpl); }

    StructTemplateSpecializationDecl*
    findSpecialization(const std::vector<Type *> &args) const;

    StructTemplateSpecializationDecl*
    fetchSpecialization(const std::vector<Type *> &args);

    StructTemplateSpecializationDecl*
    createSpecialization(const std::vector<Type *> &args);
};

class TemplateFunctionDecl final : public TemplateDecl {
    friend class CGN;
    friend class NameResolution;
    friend class Sema;

    std::vector<FunctionTemplateSpecializationDecl *> m_Specs;

public:
    TemplateFunctionDecl(const Runes &R, const Metadata &M, const String &N,
                          FunctionDecl *F, std::vector<TemplateParamDecl *> P)
      : TemplateDecl(R, M, N, F, P) {}

    ~TemplateFunctionDecl() override;

    void accept(Visitor *V) override { V->visit(this); }

    FunctionDecl *getTemplatedFunction() const 
    { return static_cast<FunctionDecl *>(m_Tmpl); }

    FunctionTemplateSpecializationDecl*
    findSpecialization(const std::vector<Type *> &args) const;

    FunctionTemplateSpecializationDecl*
    fetchSpecialization(const std::vector<Type *> &args);

    FunctionTemplateSpecializationDecl*
    createSpecialization(const std::vector<Type *> &args);
};

class FunctionTemplateSpecializationDecl final : public FunctionDecl {
    friend class CGN;
    friend class NameResolution;
    friend class Sema;

    TemplateFunctionDecl *m_Tmpl;
    std::vector<Type *> m_Args;
    std::unordered_map<TemplateParamType *, Type *> m_Mapping;

public:
    FunctionTemplateSpecializationDecl(TemplateFunctionDecl *TS, const String &N,
                                       FunctionType *T, Scope *S, 
                                       std::vector<ParamDecl *> P, 
                                       Stmt *B, std::vector<Type *> A,
                                       std::unordered_map<TemplateParamType *, Type *> M)
      : FunctionDecl(TS->getRunes(), TS->getMetadata(), N, T, S, P, B), 
        m_Tmpl(TS), m_Args(A), m_Mapping(M) {}

    void accept(Visitor *V) override { V->visit(this); }

    TemplateFunctionDecl *getTemplateDecl() const { return m_Tmpl; }

    const std::vector<Type *> &getArgs() const { return m_Args; }

    bool compareArgs(const std::vector<Type *> &args) const;
};

class StructTemplateSpecializationDecl final : public StructDecl {
    friend class CGN;
    friend class NameResolution;
    friend class Sema;

    TemplateStructDecl *m_Tmpl;
    std::vector<Type *> m_Args;
    std::unordered_map<TemplateParamType *, Type *> m_Mapping;

public:
    StructTemplateSpecializationDecl(TemplateStructDecl *TS, const String &N, 
                                     TemplateStructType *T, Scope *S, 
                                     std::vector<FieldDecl *> Fields, 
                                     std::vector<FunctionDecl *> Funcs, 
                                     std::vector<Type *> Args,
                                     std::unordered_map<TemplateParamType *, Type *> M)
      : StructDecl(TS->getRunes(), TS->getMetadata(), N, T, S, Fields, Funcs), 
        m_Tmpl(TS), m_Args(Args), m_Mapping(M) {}

    void accept(Visitor *V) override { V->visit(this); }

    TemplateStructDecl *getTemplateDecl() const { return m_Tmpl; }

    const std::vector<Type *> &getArgs() const { return m_Args; }

    bool compareArgs(const std::vector<Type *> &args) const;
};

} // namespace meddle

#endif // MEDDLE_DECL_H
