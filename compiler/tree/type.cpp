#include "context.h"
#include "decl.h"
#include "scope.h"
#include "type.h"
#include "../core/logger.h"

#include <cassert>

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

Type *Type::get(Context *C, const String &name, const Scope *scope, 
                const Metadata &md) {
    assert(C && "Context cannot be null.");
    assert(!name.empty() && "Name cannot be empty.");
    assert(scope && "Scope cannot be null.");

    if (NamedDecl *N = scope->lookup(name)) {
        if (auto *TD = dynamic_cast<TypeDecl *>(N))
            return TD->getDefinedType();

        fatal("named declaration is not a type: " + name, &md);
    }

    auto prim_it = C->m_Primitives.find(name);
    if (prim_it != C->m_Primitives.end())
        return prim_it->second;

    auto unresolved_it = C->m_Deferred.find(name);
    if (unresolved_it != C->m_Deferred.end())
        return unresolved_it->second;

    DeferredType *defer = new DeferredType(name, scope, md);
    return C->m_Deferred[name] = defer;
}

ArrayType *DeferredType::asArray() {
    assert(m_Underlying && m_Underlying->isArray() && 
           "Underlying type is not an array.");
    return static_cast<ArrayType *>(m_Underlying);
}

PointerType *DeferredType::asPointer() {
    assert(m_Underlying && m_Underlying->isPointer() && 
           "Underlying type is not a pointer.");
    return static_cast<PointerType *>(m_Underlying);
}

EnumType *DeferredType::asEnum() {
    assert(m_Underlying && m_Underlying->isEnum() && 
           "Underlying type is not an enum.");
    return static_cast<EnumType *>(m_Underlying);
}

StructType *DeferredType::asStruct() {
    assert(m_Underlying && m_Underlying->isStruct() && 
           "Underlying type is not a struct.");
    return static_cast<StructType *>(m_Underlying);
}

PrimitiveType::PrimitiveType(PrimitiveType::Kind K) 
  : Type(getNameForPrimitiveType(K)), m_Kind(K), m_Signed(hasSignedness(K)) {}

PrimitiveType *PrimitiveType::get(Context *C, const Kind &K) {
    assert(C && "Context cannot be null.");

    auto it = C->m_Primitives.find(getNameForPrimitiveType(K));
    assert(it != C->m_Primitives.end() && "Primitive type not found in context.");
    return it->second;
}

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

    if (auto *prim = dynamic_cast<PrimitiveType *>(T))
        return isVoid() == T->isVoid();
    else if (auto *ptr = dynamic_cast<PointerType *>(T))
        return isSInt() || isUInt();
    else if (auto *defer = dynamic_cast<DeferredType *>(T))
        return canCastTo(defer->getUnderlying());

    return false;
}

bool PrimitiveType::canImplCastTo(Type *T) const {
    assert(T && "Type cannot be null.");

    if (auto *defer = dynamic_cast<DeferredType *>(T))
        return canImplCastTo(defer->getUnderlying());

    auto *prim = dynamic_cast<PrimitiveType *>(T);
    if (!prim)
        return false;

    // Cannot implicitly cast: float -> integer.
    if (isFloat() && T->isInt())
        return false;

    return isVoid() == T->isVoid();
}

bool PrimitiveType::compare(Type *T) const {
    assert(T && "Type cannot be null.");

    if (auto *prim = dynamic_cast<PrimitiveType *>(T))
        return m_Kind == prim->m_Kind;
    else if (auto *defer = dynamic_cast<DeferredType *>(T))
        return compare(defer->getUnderlying());

    return false;
}

ArrayType *ArrayType::get(Context *C, Type *Elem, unsigned Sz) {
    assert(C && "Context cannot be null.");
    assert(Elem && "Element type cannot be null.");
    assert(Elem->isQualified() && "Element type must be qualified.");
    assert(Sz > 0 && "Size must be greater than zero.");

    String name = Elem->getName() + "[" + std::to_string(Sz) + "]";
    auto it = C->m_Arrays.find(name);
    if (it != C->m_Arrays.end())
        return it->second;

    return C->m_Arrays[name] = new ArrayType(Elem, Sz);
}

bool ArrayType::canCastTo(Type *T) const {
    assert(T && "Type cannot be null.");

    if (auto *arr = dynamic_cast<ArrayType *>(T))
        return m_Element->canCastTo(arr->getElement()) && m_Size == arr->m_Size;
    else if (auto *ptr = dynamic_cast<PointerType *>(T))
        return m_Element->canCastTo(ptr->getPointee());
    else if (auto *defer = dynamic_cast<DeferredType *>(T))
        return canCastTo(defer->getUnderlying());
    else
        return false;
}

bool ArrayType::canImplCastTo(Type *T) const {
    assert(T && "Type cannot be null.");

    if (auto *arr = dynamic_cast<ArrayType *>(T))
        return m_Element->canImplCastTo(arr->getElement()) && m_Size == arr->m_Size;
    else if (auto *ptr = dynamic_cast<PointerType *>(T))
        return m_Element->canImplCastTo(ptr->getPointee());
    else if (auto *defer = dynamic_cast<DeferredType *>(T))
        return canImplCastTo(defer->getUnderlying());
    else
        return false;
}

bool ArrayType::compare(Type *T) const {
    assert(T && "Type cannot be null.");

    if (auto *arr = dynamic_cast<ArrayType *>(T))
        return m_Element->compare(arr->m_Element) && m_Size == arr->m_Size;
    else if (auto *defer = dynamic_cast<DeferredType *>(T))
        return compare(defer->getUnderlying());

    return false;
}

PointerType *PointerType::get(Context *C, Type *Pt) {
    assert(C && "Context cannot be null.");
    assert(Pt && "Pointee type cannot be null.");
    assert(Pt->isQualified() && "Pointee type must be qualified.");

    String name = Pt->getName() + "*";
    auto it = C->m_Pointers.find(name);
    if (it != C->m_Pointers.end())
        return it->second;

    return C->m_Pointers[name] = new PointerType(Pt);
}

bool PointerType::canCastTo(Type *T) const {
    assert(T && "Type cannot be null.");
    return T->isPointer() || T->isUInt() || T->isSInt();
}

bool PointerType::canImplCastTo(Type *T) const {
    assert(T && "Type cannot be null.");
    return m_Pointee->isVoid();
}

bool PointerType::compare(Type *T) const {
    assert(T && "Type cannot be null.");
    if (auto *ptr = dynamic_cast<PointerType *>(T))
        return m_Pointee->compare(ptr->getPointee());
    else if (auto *defer = dynamic_cast<DeferredType *>(T))
        return compare(defer->getUnderlying());
    
    return false;
}

FunctionType::FunctionType(std::vector<Type *> P, Type *R)
  : Type(getNameForFunctionType(P, R)), m_Params(P), m_Ret(R) {}

FunctionType *FunctionType::create(Context *C, std::vector<Type *> Params, 
                                   Type *Ret) {
    assert(C && "Context cannot be null.");
    assert(Ret && "Return type cannot be null.");

    FunctionType *FT = new FunctionType(Params, Ret);
    C->m_FunctionTypes.push_back(FT);
    return FT;
}

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

EnumType *EnumType::create(Context *C, const String &name, Type *underlying, 
                           EnumDecl *decl) {
    assert(C && "Context cannot be null.");
    assert(!name.empty() && "Name cannot be empty.");
    assert(underlying && "Underlying type cannot be null.");
    assert(underlying->isQualified() && "Underlying enum type must be qualified.");
    assert(underlying->isInt() && "Underlying enum type must be an integer.");

    auto enum_it = C->m_Enums.find(name);
    if (enum_it != C->m_Enums.end())
        fatal("duplicate enum type: " + name, 
            decl ? &decl->getMetadata() : nullptr);

    return C->m_Enums[name] = new EnumType(name, underlying, decl);
}

bool EnumType::canCastTo(Type *T) const {
    assert(T && "Type cannot be null.");
    return m_Underlying->canCastTo(T);
}

bool EnumType::canImplCastTo(Type *T) const {
    assert(T && "Type cannot be null.");
    return m_Underlying->canImplCastTo(T);
}

bool EnumType::compare(Type *T) const {
    assert(T && "Type cannot be null.");
    
    if (this == T)
        return true;
    else if (T->isDeferred())
        return compare(T->asDeferred()->getUnderlying());
    else
        return false;
}

StructType *StructType::create(Context *C, const String &name, 
                               std::vector<Type *> fields, 
                               StructDecl *decl) {
    assert(C && "Context cannot be null.");
    assert(!name.empty() && "Name cannot be empty.");
    assert(!fields.empty() && "Fields cannot be empty.");

    auto struct_it = C->m_Structs.find(name);
    if (struct_it != C->m_Structs.end())
        fatal("duplicate struct type: " + name, 
            decl ? &decl->getMetadata() : nullptr);

    return C->m_Structs[name] = new StructType(name, fields, decl);
}

bool StructType::compare(Type *T) const {
    assert(T && "Type cannot be null.");

    if (this == T)
        return true;
    else if (T->isDeferred())
        return compare(T->asDeferred()->getUnderlying());
    else
        return false;
}

bool TemplateParamType::compare(Type *T) const {
    assert(T && "Type cannot be null.");

    if (this == T)
        return true;
    else if (auto *other = dynamic_cast<TemplateParamType *>(T))
        return getDecl() == other->getDecl();

    return false;
}

unsigned TemplateParamType::getIndex() const {
    assert(m_Decl && "Parameter type declaration cannot be null.");
    return m_Decl->getIndex();
}

TemplateStructType::TemplateStructType(const String &N, std::vector<Type *> F, 
                                       std::vector<Type *> A, 
                                       StructTemplateSpecializationDecl *D)
  : StructType(N, F, D), m_Args(A) { D->setDefinedType(this); }

TemplateStructType *TemplateStructType::get(Context *C, TemplateStructDecl *tmpl,
                                            std::vector<Type *> args) {
    assert(C && "Context cannot be null.");
    assert(tmpl && "Template struct declaration cannot be null.");
    assert(!args.empty() && "Template type arguments cannot be empty.");

    for (auto &A : args)
        assert(!A->isParamDependent() && "Template type arguments cannot be dependent.");

    auto spec_it = C->m_StructSpecs.find(tmpl->getName());
    if (spec_it != C->m_StructSpecs.end())
        return dynamic_cast<TemplateStructType *>(spec_it->second);

    return static_cast<TemplateStructType *>(
        tmpl->fetchSpecialization(args)->getDefinedType());
}

TemplateStructType *TemplateStructType::create(Context *C, std::vector<Type *> fields, 
                                               StructTemplateSpecializationDecl *decl,
                                               std::vector<Type *> args) {
    assert(C && "Context cannot be null.");
    assert(decl && "Template struct specialization declaration cannot be null.");
    assert(!args.empty() && "Template type arguments cannot be empty.");

    if (get(C, decl->getTemplateDecl(), args))
        fatal("duplicate template specialization: " + decl->getName(), 
            &decl->getMetadata());

    return C->m_StructSpecs[decl->getName()] = 
        new TemplateStructType(decl->getName(), fields, args, decl);
}

bool TemplateStructType::compare(Type *T) const {
    assert(T && "Type cannot be null.");
    
    if (this == T)
        return true;
    else if (T->isDeferred())
        return compare(T->asDeferred()->getUnderlying());
    else
        return false;
}

TemplateStructDecl*
TemplateStructType::getTemplateDecl() const {
    return static_cast<StructTemplateSpecializationDecl *>(m_Decl)->getTemplateDecl();
}

StructTemplateSpecializationDecl*
TemplateStructType::getSpecializedDecl() const {
    return static_cast<StructTemplateSpecializationDecl *>(m_Decl);
}

DependentTemplateStructType::DependentTemplateStructType(const String &N, 
                                                         TemplateStructDecl *T, 
                                                         std::vector<Type *> A)
  : Type(N), m_Tmpl(T), m_Args(A) {}

DependentTemplateStructType*
DependentTemplateStructType::get(Context *C, TemplateStructDecl *tmpl, 
                                 const std::vector<Type *> &args) {
    assert(C && "Context cannot be null.");
    assert(tmpl && "Template struct declaration cannot be null.");
    assert(!args.empty() && "Template type arguments cannot be empty.");

    auto dep_it = C->m_Dependents.find(tmpl->getConcreteName(args));
    if (dep_it != C->m_Dependents.end())
        return dynamic_cast<DependentTemplateStructType *>(dep_it->second);

    return nullptr;
}

bool DependentTemplateStructType::compare(Type *T) const {
    assert(T && "Type cannot be null.");

    if (this == T)
        return true;
    else if (T->isDeferred())
        return compare(T->asDeferred()->getUnderlying());
    else
        return false;
}
