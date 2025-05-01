#include "segment.h"
#include "type.h"

using namespace mir;

static TypeKind get_integer_ty_kind(IntegerType::Kind K) {
    switch (K) {
    case mir::IntegerType::Kind::Int1: return TypeKind::I1;
    case mir::IntegerType::Kind::Int8: return TypeKind::I8;
    case mir::IntegerType::Kind::Int16: return TypeKind::I16;
    case mir::IntegerType::Kind::Int32: return TypeKind::I32;
    case mir::IntegerType::Kind::Int64: return TypeKind::I64;
    default: assert(false && "Invalid integer type size.");
    }
}

static TypeKind get_fp_ty_kind(FloatType::Kind K) {
    switch (K) {
    case mir::FloatType::Kind::Float32: return TypeKind::F32;
    case mir::FloatType::Kind::Float64: return TypeKind::F64;
    default: assert(false && "Invalid floating point type size.");
    }
}

static String get_function_ty_name(const std::vector<Type *> P, Type *R) {
    String N = "(";
    for (auto &T : P) N += T->get_name() + (T == P.back() ? "" : ", ");
    return N + ")" + (R ? " -> " + R->get_name() : "");
}

ArrayType *ArrayType::get(Segment *S, Type *E, unsigned Sz) {
    Type *T = S->m_Types[E->get_name() + "[" + std::to_string(Sz) + "]"];
    if (T)
        return static_cast<ArrayType *>(T);

    ArrayType *AT = new ArrayType(E, Sz);
    S->m_Types[AT->get_name()] = AT;
    return AT;
}

IntegerType::IntegerType(Kind K) 
    : Type(get_kind_name(K), get_integer_ty_kind(K)), m_Kind(K) {}

FloatType::FloatType(Kind K) 
    : Type(get_kind_name(K), get_fp_ty_kind(K)), m_Kind(K) {}

FunctionType::FunctionType(Type *R, std::vector<Type *> P) 
    : Type(get_function_ty_name(P, R), TypeKind::Function), m_Params(P), 
      m_Ret(R) {}

FunctionType *FunctionType::get(Segment *S, std::vector<Type *> Ps, Type *R) {
    Type *T = S->m_Types[get_function_ty_name(Ps, R)];
    if (T)
        return static_cast<FunctionType *>(T);

    FunctionType *FT = new FunctionType(R, Ps);
    S->m_Types[FT->get_name()] = FT;
    return FT;
}

PointerType *PointerType::get(Segment *S, Type *P) {
    Type *T = S->m_Types[P->get_name() + "*"];
    if (T)
        return static_cast<PointerType *>(T);
    
    PointerType *PT = new PointerType(P);
    S->m_Types[PT->get_name()] = PT;
    return PT;
}

StructType *StructType::get(Segment *S, String N) {
    Type *T = S->m_Types[N];
    if (T) {
        if (!T->is_struct_ty())
            assert(false && "Type is not a struct type.");
        
        return static_cast<StructType *>(T);
    }
    
    return nullptr;
}

StructType *StructType::create(Segment *S, String N, std::vector<Type *> Ms) {
    assert(!get(S, N) && "Struct type already exists.");

    StructType *ST = new StructType(N, Ms);
    S->m_Types[N] = ST;
    return ST;
}
