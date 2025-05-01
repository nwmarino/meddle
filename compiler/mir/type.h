#ifndef MEDDLE_MIR_TYPE_H
#define MEDDLE_MIR_TYPE_H

#include <cassert>
#include <string>
#include <vector>

using String = std::string;

namespace mir {

class Segment;

enum class TypeKind {
    I1, I8, I16, I32, I64,
    F32, F64,
    Array,
    Function,
    Pointer,
    Struct,
};

class Type {
    String m_Name;
    TypeKind m_TypeKind;

public:
    Type(String N, TypeKind K) : m_Name(N), m_TypeKind(K) {}

    virtual ~Type() = default;

    String get_name() const { return m_Name; }

    TypeKind get_ty_kind() const { return m_TypeKind; }

    virtual bool is_array_ty() const { return false; }

    virtual bool is_pointer_ty() const { return false; }

    virtual bool is_integer_ty() const { return false; }

    virtual bool is_integer_ty(unsigned N) const { return false; }

    virtual bool is_float_ty() const { return false; }
    
    virtual bool is_float_ty(unsigned N) const { return false; }

    virtual bool is_function_ty() const { return false; }

    virtual bool is_struct_ty() const { return false; }
    
    virtual bool is_void_ty() const { return false; }
};

class ArrayType final : public Type {
    friend class Builder;
    friend class Segment;

    Type *m_Element;
    unsigned m_Size;

    ArrayType(Type *E, unsigned S) 
      : Type(E->get_name() + "[" + std::to_string(S) + "]", TypeKind::Array), 
        m_Element(E), m_Size(S) {}

public:
    static ArrayType *get(Segment *S, Type *E, unsigned Sz);
    
    bool is_array_ty() const override { return true; }

    Type *get_element() const { return m_Element; }

    unsigned get_size() const { return m_Size; }
};

class IntegerType final : public Type {
    friend class Builder;
    friend class Segment;

public:
    enum class Kind {
        Int1,
        Int8,
        Int16,
        Int32,
        Int64,
        //Int128,
    };

    String get_kind_name(Kind K) const {
        switch (K) {
        case Kind::Int1: return "i1";
        case Kind::Int8: return "i8";
        case Kind::Int16: return "i16";
        case Kind::Int32: return "i32";
        case Kind::Int64: return "i64";
        //case Kind::Int128: return "i128";
        }
    }

private:
    Kind m_Kind;

    IntegerType(Kind K);

public:
    bool is_integer_ty() const override { return true; }

    bool is_integer_ty(unsigned N) const override {
        switch (m_Kind) {
        case Kind::Int1: return N == 1;
        case Kind::Int8: return N == 8;
        case Kind::Int16: return N == 16;
        case Kind::Int32: return N == 32;
        case Kind::Int64: return N == 64;
        //case Kind::Int128: return N == 128;
        }
        return false;
    }

    Kind get_kind() const { return m_Kind; }
};

class FloatType final : public Type {
    friend class Builder;
    friend class Segment;

public:
    enum class Kind {
        Float32,
        Float64,
        //Float128,
    };

    String get_kind_name(Kind K) const {
        switch (K) {
        case Kind::Float32: return "f32";
        case Kind::Float64: return "f64";
        //case Kind::Float128: return "f128";
        }
    }

private:
    Kind m_Kind;

    FloatType(Kind K);

public:
    Kind get_kind() const { return m_Kind; }

    bool is_float_ty() const override { return true; }

    bool is_float_ty(unsigned N) const override {
        switch (m_Kind) {
        case Kind::Float32: return N == 32;
        case Kind::Float64: return N == 64;
        //case Kind::Float128: return N == 128;
        }
        return false;
    }
};

class FunctionType final : public Type {
    friend class Builder;
    friend class Segment;

    std::vector<Type *> m_Params;
    Type *m_Ret;

    FunctionType(Type *R, std::vector<Type *> P);

public:
    static FunctionType *get(Segment *S, std::vector<Type *> Ps, Type *R);

    bool is_function_ty() const override { return true; }

    Type *get_return_type() const { return m_Ret; }    

    const std::vector<Type *> &get_param_types() const { return m_Params; }
};

class PointerType final : public Type {
    friend class Builder;
    friend class Segment;

    Type *m_Pointee;

    PointerType(Type *P) 
        : Type(P->get_name() + "*", TypeKind::Pointer), m_Pointee(P) {}

public:
    static PointerType *get(Segment *S, Type *P);

    bool is_pointer_ty() const override { return true; }

    Type *get_pointee() const { return m_Pointee; }
};

class StructType final : public Type {
    friend class Builder;
    friend class Segment;

    std::vector<Type *> m_Members;

    StructType(String N, std::vector<Type *> M) 
        : Type(N, TypeKind::Struct), m_Members(M) {}

public:
    static StructType *get(Segment *S, String N);
    static StructType *create(Segment *S, String N, std::vector<Type *> Ms);

    bool is_struct_ty() const override { return true; }

    const std::vector<Type *> &get_members() const { return m_Members; }

    Type *get_member(unsigned i) const {
        assert(i < m_Members.size() && "Index out of range.");
        return m_Members.at(i);
    }
};

class VoidType final : public Type {
    friend class Builder;
    friend class Segment;

    VoidType() : Type("void", TypeKind::I32) {}

public:
    bool is_void_ty() const override { return true; }
};

} // namespace mir

#endif // MEDDLE_MIR_TYPE_H
