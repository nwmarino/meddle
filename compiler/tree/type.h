#ifndef MEDDLE_TREE_TYPE_H
#define MEDDLE_TREE_TYPE_H

#include "../core/metadata.h"

#include <cassert>
#include <string>
#include <vector>

using String = std::string;

namespace meddle {

class Type {
protected:
    String m_Name;

public:
    Type(const String &N) : m_Name(N) {}
    virtual ~Type() = default;

    String getName() const { return m_Name; }

    bool isVoid() const { return m_Name == "void"; }

    bool isBool() const { return m_Name == "bool"; }

    bool isChar() const { return m_Name == "char"; }
    
    bool isArray() const { return m_Name.back() == ']'; }

    bool isPointer() const { return m_Name.back() == '*'; }
    
    virtual bool isSInt() const { return false; }

    virtual bool isSInt(unsigned N) const { return false; }

    virtual bool isUInt() const { return false; }

    virtual bool isUInt(unsigned N) const { return false; }

    virtual bool isFloat() const { return false; }

    virtual bool isFloat(unsigned N) const { return false; }

    virtual bool canCastTo(Type *T) const { return false; }

    virtual bool compare(Type *T) const { return false; }

    virtual bool isQualified() const { return true; }

    virtual unsigned getSizeInBits() const { return 0; }

    unsigned getSizeInBytes() const { return getSizeInBits() / 8; }
};

class TypeResult final : public Type {
    Type *m_Underlying;
    Metadata m_Metadata;

public:
    TypeResult(const String &N, const Metadata &M) : Type(N), m_Metadata(M) {}

    Type *getUnderlying() const { return m_Underlying; }

    void setUnderlying(Type *T) { m_Underlying = T;}

    const Metadata &getMetadata() const { return m_Metadata; }

    bool isQualified() const override { return m_Underlying != nullptr; }

    bool canCastTo(Type *T) const override
    { assert(false && "Cannot cast this non-concrete type."); }

    bool compare(Type *T) const override 
    { assert(false && "Cannot compare this non-concrete type."); }
};

class PrimitiveType final : public Type {
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

public:
    PrimitiveType(Kind K);
    
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

    bool compare(Type *T) const override;

    unsigned getSizeInBits() const override;
};

class ArrayType final : public Type {
    Type *m_Element;
    unsigned m_Size;

public: 
    ArrayType(Type *E, unsigned S)
      : Type(E->getName() + "[" + std::to_string(S) + "]"), m_Element(E), 
        m_Size(S) {}

    Type *getElement() const { return m_Element; }

    unsigned getSize() const { return m_Size; }

    bool canCastTo(Type *T) const override;

    bool compare(Type *T) const override;

    unsigned getSizeInBits() const override 
    { return m_Element->getSizeInBits() * m_Size; }
};

class PointerType final : public Type {
    Type *m_Pointee;

public:
    PointerType(Type *P) : Type(P->getName() + "*"), m_Pointee(P) {}

    Type *getPointee() const { return m_Pointee; }

    bool canCastTo(Type *T) const override;

    bool compare(Type *T) const override;

    unsigned getSizeInBits() const override { return 64; }
};

class FunctionType final : public Type {
    std::vector<Type *> m_Params;
    Type *m_Ret;

public:
    FunctionType(std::vector<Type *> P, Type *R);

    Type *getParamType(unsigned i) const {
        assert(i <= m_Params.size());
        return m_Params[i];
    }

    const std::vector<Type *> &getParams() const { return m_Params; } 

    unsigned getNumParams() const { return m_Params.size(); }

    void setParamType(Type *T, unsigned i) {
        assert(i <= m_Params.size());
        m_Params[i] = T;
    }

    Type *getReturnType() const { return m_Ret; }

    void setReturnType(Type *T) { m_Ret = T; }

    bool compare(Type *T) const override;
};

/*
class EnumType final : public Type {
    Type *m_Underlying;
};

class StructType final : public Type {
    std::vector<Type *> m_Fields;
};

class TemplateParamType final : public Type {

};

class TemplateStructType final : public Type {
    std::vector<Type *> m_Args;
};

class DependentTemplateStructType final : public Type {
    std::vector<Type *> m_Args;
};
*/

} // namespace meddle

#endif // MEDDLE_TREE_TYPE_H
