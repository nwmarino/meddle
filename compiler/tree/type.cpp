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

bool PrimitiveType::isSInt() const {
    return m_Kind == Kind::Int8 ||
           m_Kind == Kind::Int16 ||
           m_Kind == Kind::Int32 ||
           m_Kind == Kind::Int64;
}

bool PrimitiveType::isSInt(unsigned N) const {
    switch (m_Kind) {
        case Kind::Int8: return N == 8;
        case Kind::Int16: return N == 16;
        case Kind::Int32: return N == 32;
        case Kind::Int64: return N == 64;
        default: return false;
    }
}

bool PrimitiveType::isUInt() const {
    return m_Kind == Kind::UInt8 ||
           m_Kind == Kind::UInt16 ||
           m_Kind == Kind::UInt32 ||
           m_Kind == Kind::UInt64;
}

bool PrimitiveType::isUInt(unsigned N) const {
    switch (m_Kind) {
        case Kind::UInt8: return N == 8;
        case Kind::UInt16: return N == 16;
        case Kind::UInt32: return N == 32;
        case Kind::UInt64: return N == 64;
        default: return false;
    }
}

bool PrimitiveType::isFloat(unsigned N) const {
    switch (m_Kind) {
        case Kind::Float32: return N == 32;
        case Kind::Float64: return N == 64;
        default: return false;
    }
}

bool PrimitiveType::canCastTo(Type *T) const {
    assert(T && "Type cannot be null.");
    if (auto *PT = dynamic_cast<PrimitiveType *>(T))
        return isVoid() == T->isVoid();
    else if (auto *PT = dynamic_cast<PointerType *>(T))
        return isSInt() || isUInt();

    return false;
}

bool PrimitiveType::compare(Type *T) const {
    assert(T && "Type cannot be null.");
    if (auto *PT = dynamic_cast<PrimitiveType *>(T))
        return m_Kind == PT->m_Kind;

    return false;
}

bool ArrayType::canCastTo(Type *T) const {
    assert(T && "Type cannot be null.");
    if (auto *AT = dynamic_cast<ArrayType *>(T))
        return m_Element->canCastTo(AT->m_Element) && m_Size == AT->m_Size;
    else if (auto *PT = dynamic_cast<PointerType *>(T))
        return m_Element->canCastTo(PT->getPointee());
    else
        return false;
}

bool ArrayType::compare(Type *T) const {
    assert(T && "Type cannot be null.");
    if (auto *AT = dynamic_cast<ArrayType *>(T))
        return m_Element->compare(AT->m_Element) && m_Size == AT->m_Size;

    return false;
}

bool PointerType::canCastTo(Type *T) const {
    assert(T && "Type cannot be null.");
    return T->isPointer() || T->isUInt() || T->isSInt();
}

bool PointerType::compare(Type *T) const {
    assert(T && "Type cannot be null.");
    if (auto *PT = dynamic_cast<PointerType *>(T))
        return m_Pointee->compare(PT->getPointee());
    return false;
}

FunctionType::FunctionType(std::vector<Type *> P, Type *R)
  : Type(getNameForFunctionType(P, R)), m_Params(P), m_Ret(R) {}

bool FunctionType::compare(Type *T) const {
    assert(T && "Type cannot be null.");
    FunctionType *FT = dynamic_cast<FunctionType *>(T);
    if (!FT)
        return false;

    if (m_Params.size() != FT->m_Params.size())
        return false;

    for (unsigned i = 0; i < m_Params.size(); ++i)
        if (!m_Params[i]->compare(FT->m_Params[i]))
            return false;

    return m_Ret->compare(FT->m_Ret);
}
