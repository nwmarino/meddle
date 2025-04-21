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
};

class TypeResult final : public Type {
    Type *m_Underlying;
    Metadata m_Metadata;

public:
    TypeResult(const String &N, const Metadata &M) : Type(N), m_Metadata(M) {}

    Type *getUnderlying() const { return m_Underlying; }

    void setUnderlying(Type *T) { m_Underlying = T;}

    const Metadata &getMetadata() const { return m_Metadata; }
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
    };

private:
    Kind m_Kind;
    bool m_Signed;

public:
    PrimitiveType(Kind K);
    
    Kind getKind() const { return m_Kind; }

    bool isSigned() const { return m_Signed; }
};

/*
class ArrayType final : public Type {
    Type *m_Element;
    unsigned m_Size;
};

class PointerType final : public Type {
    Type *m_Pointee;
};
*/

class FunctionType final : public Type {
    std::vector<Type *> m_Params;
    Type *m_Ret;

public:
    FunctionType(std::vector<Type *> P, Type *R);

    Type *getParamType(unsigned i) const {
        assert(i <= m_Params.size());
        return m_Params[i];
    }

    void setParamType(Type *T, unsigned i) {
        assert(i <= m_Params.size());
        m_Params[i] = T;
    }

    Type *getReturnType() const { return m_Ret; }

    void setReturnType(Type *T) { m_Ret = T; }
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
