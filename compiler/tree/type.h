#ifndef MEDDLE_TREE_TYPE_H
#define MEDDLE_TREE_TYPE_H

#include "../core/metadata.h"

#include <cassert>
#include <string>
#include <vector>

using String = std::string;

namespace meddle {

class Context;
class Scope;
class EnumDecl;
class StructDecl;
class TemplateParamDecl;
class TemplateStructDecl;
class StructTemplateSpecializationDecl;

class DeferredType;
class PrimitiveType;
class ArrayType;
class PointerType;
class FunctionType;
class EnumType;
class StructType;
class TemplateParamType;
class TemplateStructType;

class Type {
    friend class Context;

protected:
    String m_Name;

    Type(const String &N) : m_Name(N) {}

public:
    static Type *get(Context *C, const String &name, const Scope *scope, 
                     const Metadata &md);

    virtual ~Type() = default;

    const String &getName() const { return m_Name; }

    virtual bool isArray() const { return false; }

    virtual ArrayType *asArray()
    { assert(false && "Cannot cast type to an array."); }

    virtual bool isPointer() const { return false; }

    virtual PointerType *asPointer()
    { assert(false && "Cannot cast type to a pointer."); }

    virtual bool isFunction() const { return false; }

    virtual FunctionType *asFunction()
    { assert(false && "Cannot cast type to a function."); }

    virtual bool isEnum() const { return false; }

    virtual EnumType *asEnum() 
    { assert(false && "Cannot cast type to an enum."); }

    virtual bool isStruct() const { return false; }

    virtual StructType *asStruct() 
    { assert(false && "Cannot cast type to a struct."); }

    virtual bool isDeferred() const { return false; }

    virtual DeferredType *asDeferred() 
    { assert(false && "Cannot cast type to a deferred type."); }
    
    virtual bool isSInt() const { return false; }

    virtual bool isSInt(unsigned N) const { return false; }

    virtual bool isUInt() const { return false; }

    virtual bool isUInt(unsigned N) const { return false; }

    virtual bool isFloat() const { return false; }

    virtual bool isFloat(unsigned N) const { return false; }

    virtual bool canCastTo(Type *T) const { return false; }

    virtual bool canImplCastTo(Type *T) const { return false; }

    virtual bool compare(Type *T) const { return false; }

    virtual bool isParamDependent() const { return false; }

    virtual bool isQualified() const { return true; }

    bool isVoid() const { return m_Name == "void"; }

    bool isBool() const { return m_Name == "bool"; }

    bool isChar() const { return m_Name == "char"; }

    bool isInt() const { return isSInt() || isUInt(); }

    bool isAggregate() const { return isArray() || isStruct(); }
};

class DeferredType final : public Type {
    friend class Context;
    friend class Type;

    Type *m_Underlying;
    const Scope *m_Scope;
    Metadata m_Metadata;

    DeferredType(const String &N, const Scope *S, const Metadata &M) 
      : Type(N), m_Scope(S), m_Metadata(M) {}

public:
    bool isArray() const override 
    { return m_Underlying && m_Underlying->isArray(); }

    ArrayType *asArray() override;

    bool isPointer() const override
    { return m_Underlying && m_Underlying->isPointer(); }

    PointerType *asPointer() override;

    bool isEnum() const override
    { return m_Underlying && m_Underlying->isEnum(); }

    EnumType *asEnum() override;

    bool isStruct() const override
    { return m_Underlying && m_Underlying->isStruct(); }

    StructType *asStruct() override;

    bool isDeferred() const override { return true; }

    DeferredType *asDeferred() override { return this; }

    Type *getUnderlying() const { return m_Underlying; }

    void setUnderlying(Type *T) { m_Underlying = T; }

    const Scope *getScope() const { return m_Scope; }

    const Metadata &getMetadata() const { return m_Metadata; }

    bool isQualified() const override { return m_Underlying != nullptr; }

    bool canCastTo(Type *T) const override {
        assert(m_Underlying && "Cannot cast unqualified deferred type.");
        return m_Underlying->canCastTo(T);
    }

    bool canImplCastTo(Type *T) const override {
        assert(m_Underlying && "Cannot cast unqualified deferred type.");
        return m_Underlying->canImplCastTo(T);
    }

    bool compare(Type *T) const override {
        assert(m_Underlying && "Cannot compare unqualified deferred type.");
        return m_Underlying->compare(T);
    }
};

class PrimitiveType final : public Type {
    friend class Context;

public:
    enum class Kind {
        Void,
        Bool,
        Char,
        Int8,
        Int16,
        Int32,
        Int64,
        UInt8,
        UInt16,
        UInt32,
        UInt64,
        Float32,
        Float64
    };

private:
    Kind m_Kind;
    bool m_Signed;

    PrimitiveType(Kind K);
    
public:
    static PrimitiveType *get(Context *C, const Kind &K);

    Kind getKind() const { return m_Kind; }

    bool isSigned() const { return m_Signed; }

    bool isSInt() const override;

    bool isSInt(unsigned N) const override;

    bool isUInt() const override;

    bool isUInt(unsigned N) const override;

    bool isFloat() const override
    { return m_Kind == Kind::Float32 || m_Kind == Kind::Float64; }

    bool isFloat(unsigned N) const override;

    bool canCastTo(Type *T) const override;
    
    bool canImplCastTo(Type *T) const override;

    bool compare(Type *T) const override;
};

class ArrayType final : public Type {
    friend class Context;

    Type *m_Element;
    unsigned m_Size;

    ArrayType(Type *E, unsigned S)
      : Type(E->getName() + "[" + std::to_string(S) + "]"), m_Element(E), 
        m_Size(S) {}

public:
    static ArrayType *get(Context *C, Type *Elem, unsigned Sz);

    bool isArray() const override { return true; }

    ArrayType *asArray() override { return this; }

    Type *getElement() const { return m_Element; }

    unsigned getSize() const { return m_Size; }

    bool canCastTo(Type *T) const override;

    bool canImplCastTo(Type *T) const override;

    bool compare(Type *T) const override;

    bool isParamDependent() const override 
    { return m_Element->isParamDependent(); }
};

class PointerType final : public Type {
    friend class Context;

    Type *m_Pointee;

    PointerType(Type *P) : Type(P->getName() + "*"), m_Pointee(P) {}

public:
    static PointerType *get(Context *C, Type *Pt);

    bool isPointer() const override { return true; }

    PointerType *asPointer() override { return this; }

    Type *getPointee() const { return m_Pointee; }

    bool canCastTo(Type *T) const override;

    bool canImplCastTo(Type *T) const override;

    bool compare(Type *T) const override;

    bool isParamDependent() const override 
    { return m_Pointee->isParamDependent(); }
};

class FunctionType final : public Type {
    friend class Context;
    
    std::vector<Type *> m_Params;
    Type *m_Ret;

    FunctionType(std::vector<Type *> P, Type *R);

public:
    static FunctionType *create(Context *C, std::vector<Type *> Params, Type *Ret);

    bool isFunction() const override { return true; }

    FunctionType *asFunction() override { return this; }
    
    Type *getParamType(unsigned i) const {
        assert(i <= m_Params.size());
        return m_Params[i];
    }

    const std::vector<Type *> &getParams() const { return m_Params; } 

    unsigned getNumParams() const { return m_Params.size(); }

    void setParamType(Type *T, unsigned i) {
        assert(i <= m_Params.size() && "Index out of range.");
        m_Params[i] = T;
    }

    Type *getReturnType() const { return m_Ret; }

    void setReturnType(Type *T) { m_Ret = T; }

    bool compare(Type *T) const override;
};

class EnumType final : public Type {
    friend class Context;

    Type *m_Underlying;
    EnumDecl *m_Decl;

    EnumType(const String &N, Type *U, EnumDecl *E = nullptr)
      : Type(N), m_Underlying(U), m_Decl(E) {}

public:
    static EnumType *create(Context *C, const String &name, Type *underlying, 
                            EnumDecl *decl = nullptr);

    bool isEnum() const override { return true; }

    EnumType *asEnum() override { return this; }

    bool isUInt() const override { return m_Underlying->isUInt(); }

    bool isUInt(unsigned N) const override { return m_Underlying->isUInt(N); }

    bool isSInt() const override { return m_Underlying->isSInt(); }

    bool isSInt(unsigned N) const override { return m_Underlying->isSInt(N); }

    bool canCastTo(Type *T) const override;

    bool canImplCastTo(Type *T) const override;

    bool compare(Type *T) const override;

    Type *getUnderlying() const { return m_Underlying; }

    EnumDecl *getDecl() const { return m_Decl; }

    void setDecl(EnumDecl *E) { m_Decl = E; }
};

class StructType : public Type {
    friend class Context;

protected:
    std::vector<Type *> m_Fields;
    StructDecl *m_Decl;

    StructType(const String &N, std::vector<Type *> F, StructDecl *D = nullptr) 
        : Type(N), m_Fields(F), m_Decl(D) {}

public:
    static StructType *create(Context *C, const String &name, 
                              std::vector<Type *> fields, 
                              StructDecl *decl = nullptr);

    bool isStruct() const override { return true; }

    StructType *asStruct() override { return this; }

    bool compare(Type *T) const override;

    bool isParamDependent() const override {
        for (auto &F : m_Fields)
            if (F->isParamDependent())
                return true;

        return false;
    }

    const std::vector<Type *> &getFields() const { return m_Fields; }

    Type *getField(unsigned i) const {
        assert(i < m_Fields.size() && "Index out of range.");
        return m_Fields.at(i);
    }

    unsigned getNumFields() const { return m_Fields.size(); }

    StructDecl *getDecl() const { return m_Decl; }

    void setDecl(StructDecl *S) { m_Decl = S; }
};

class TemplateParamType final : public Type {
    friend class Context;

    TemplateParamDecl *m_Decl;

public:
    TemplateParamType(const String &N, TemplateParamDecl *D)
      : Type(N), m_Decl(D) {}

    bool compare(Type *T) const override;

    bool isParamDependent() const override { return true; }

    TemplateParamDecl *getDecl() const { return m_Decl; }

    void setDecl(TemplateParamDecl *T) { m_Decl = T; }

    unsigned getIndex() const;
};

/// Represents instantiated structure types not dependent on a parameterized
/// type, i.e. `Box<i32, i64>`.
class TemplateStructType final : public StructType {
    friend class Context;

    std::vector<Type *> m_Args;

    TemplateStructType(const String &name, std::vector<Type *> fields, 
                       std::vector<Type *> args, 
                       StructTemplateSpecializationDecl *decl = nullptr);

public:
    static TemplateStructType *get(Context *ctx, StructDecl *tmpl,
                                   std::vector<Type *> args);
    static TemplateStructType *create(Context *ctx, const String &name, 
                                      std::vector<Type *> fields, 
                                      std::vector<Type *> args, 
                                      StructTemplateSpecializationDecl *decl = nullptr);

    bool compare(Type *T) const override;

    StructDecl *getTemplateDecl() const;

    StructTemplateSpecializationDecl *getSpecializedDecl() const;

    const std::vector<Type *> &getArgs() const { return m_Args; }
};

/// Represesnts pseudo-instantiated structure types that are still dependent
/// on a parameterized type, i.e. `Box<T>` in a context where `T` is a
/// parameter type.
class DependentTemplateStructType final : public Type {
    friend class Context;

    StructDecl *m_Tmpl;
    std::vector<Type *> m_Args;

    DependentTemplateStructType(const String &name, StructDecl *tmpl, 
                                std::vector<Type *> args);

public:
    static DependentTemplateStructType *get(Context *ctx, StructDecl *tmpl, 
                                            const std::vector<Type *> &args);

    bool compare(Type *T) const override;

    bool isParamDependent() const override { return true; }

    StructDecl *getTemplateDecl() const { return m_Tmpl; }

    const std::vector<Type *> &getArgs() const { return m_Args; }
};

} // namespace meddle

#endif // MEDDLE_TREE_TYPE_H
