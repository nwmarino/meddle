#include "type.h"

using namespace meddle;

static String getNameForPrimitiveType(PrimitiveType::Kind K) {
    switch (K) {
        case PrimitiveType::Kind::Void: return "void";
        case PrimitiveType::Kind::Bool: return "bool";
        case PrimitiveType::Kind::Char: return "char";
        case PrimitiveType::Kind::Int8: return "i8";
        case PrimitiveType::Kind::Int16: return "i16";
        case PrimitiveType::Kind::Int32: return "i32";
        case PrimitiveType::Kind::Int64: return "i64";
        case PrimitiveType::Kind::UInt8: return "u8";
        case PrimitiveType::Kind::UInt16: return "u16";
        case PrimitiveType::Kind::UInt32: return "u32";
        case PrimitiveType::Kind::UInt64: return "u64";
        case PrimitiveType::Kind::Float32: return "f32";
        case PrimitiveType::Kind::Float64: return "f64";
    }
}

static bool hasSignedness(PrimitiveType::Kind K) {
    switch (K) {
        case PrimitiveType::Kind::Int8:
        case PrimitiveType::Kind::Int16:
        case PrimitiveType::Kind::Int32:
        case PrimitiveType::Kind::Int64:
            return true;
        default:
            return false;
    }
}

static String getNameForFunctionType(const std::vector<Type *> P, Type *R) {
    String N = "(";
    for (auto &T : P) N += T->getName() + (T == P.back() ? "" : ", ");
    return N + ")" + (R ? " -> " + R->getName() : "");
}

PrimitiveType::PrimitiveType(PrimitiveType::Kind K) 
  : Type(getNameForPrimitiveType(K)), m_Kind(K), m_Signed(hasSignedness(K)) {}

FunctionType::FunctionType(std::vector<Type *> P, Type *R)
  : Type(getNameForFunctionType(P, R)), m_Params(P), m_Ret(R) {}
