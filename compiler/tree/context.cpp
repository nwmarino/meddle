#include "context.h"
#include "type.h"
#include "../core/logger.h"

using namespace meddle;

Context::Context(TranslationUnit *U) : m_Unit(U) {
    m_Types.reserve(10);
    for (unsigned K = 0; K <= 12; ++K) {
        auto *T = new PrimitiveType(static_cast<PrimitiveType::Kind>(K));
        m_Types[T->getName()] = T;
    }
}

void Context::addType(Type *T) {
    if (Type *existing = m_Types[T->getName()]) {
        // If a type exists with the same name already and is not a type
        // reference, then the type is a duplicate.
        TypeResult *TR = dynamic_cast<TypeResult *>(existing);
        if (!TR) {
            fatal(
                "type already exists: " + existing->getName(), 
                &TR->getMetadata()
            );
        }

        // The referenced type has been declared, make the type result concrete.
        TR->setUnderlying(T);
        m_Results.push_back(TR);
    }

    m_Types[T->getName()] = T;
}

Type *Context::getType(const String &N) {
    auto it = m_Types.find(N);
    if (it != m_Types.end())
        return it->second;

    //NamedDecl *D = m_Unit->getScope()->lookup(N);
    // if (D and D is a type def)
    //   return D's type

    // check external types

    if (N.back() == '*') {
        Type *pointee = getType(N.substr(0, N.size() - 1));
        if (!pointee)
            return nullptr;

        Type *T = new PointerType(pointee);
        addType(T);
        return T;
    }

    auto LBrack = N.find('[');
    auto RBrack = N.find(']');
    if (LBrack && RBrack) {
        Type *element = getType(N.substr(0, LBrack));
        if (!element)
            return nullptr;

        Type *T = new ArrayType(element, std::stoul(N.substr(LBrack + 1, RBrack - LBrack - 1)));
        addType(T);
        return T;
    }

    return nullptr;
}

ArrayType *Context::getArrayType(Type *E, unsigned long S) {
    String name = E->getName() + "[" + std::to_string(S) + "]";
    if (Type *T = m_Types[name])
        return static_cast<ArrayType *>(T);

    ArrayType *AT = new ArrayType(E, S);
    m_Types[name] = AT;
    return AT;
}

PointerType *Context::getPointerType(Type *P) {
    String name = P->getName() + "*";
    if (Type *T = m_Types[name])
        return static_cast<PointerType *>(T);

    PointerType *PT = new PointerType(P);
    m_Types[name] = PT;
    return PT;
}

Type *Context::produceType(const String &N, const Metadata &M) {
    if (Type *T = getType(N))
        return T;

    TypeResult *TR = new TypeResult(N, M);
    m_Types[N] = TR;
    return TR;
}

void Context::reconstructFunctionType(FunctionType *FT) {
    if (!FT->getReturnType()->isQualified())
        FT->setReturnType(
            static_cast<TypeResult *>(FT->getReturnType())->getUnderlying());

    for (unsigned i = 0, n = FT->getNumParams(); i != n; ++i) {
        Type *P = FT->getParamType(i);
        if (P->isQualified())
            continue;

        TypeResult *TR = static_cast<TypeResult *>(P);
        FT->setParamType(TR->getUnderlying(), i);
    }
}

void Context::sanitate() {
    // Move all type results from the main type pool to the designated results.
    for (auto it = m_Types.begin(); it != m_Types.end(); ) {
        if (TypeResult *TR = dynamic_cast<TypeResult *>(it->second)) {
            m_Results.push_back(TR);
            it = m_Types.erase(it);
        } else
            ++it;
    }

    // For each type result, try to resolve its concrete type from the pool.
    for (auto *R : m_Results) {
        Type *newTy = getType(R->getName());
        if (!newTy) {
            // If the type is not found, it is unresolved at this point.
            fatal(
                "unresolved type: " + R->getName(), &R->getMetadata()
            );
        }

        R->setUnderlying(newTy);
    }

    // For each recognized function type, unwrap any nested type results.
    for (auto &FT : m_FunctionTypes)
        reconstructFunctionType(FT);
}
