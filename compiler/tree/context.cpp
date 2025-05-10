#include "context.h"
#include "decl.h"
#include "scope.h"
#include "type.h"
#include "../core/logger.h"

using namespace meddle;

Context::Context(TranslationUnit *U) : m_Unit(U) {
    m_Primitives.reserve(12);
    for (unsigned K = 0; K <= 12; ++K) {
        auto *T = new PrimitiveType(static_cast<PrimitiveType::Kind>(K));
        m_Primitives[T->getName()] = T;
    }
}

Context::~Context() {
    for (auto &[N, T] : m_Primitives)
        delete T;

    for (auto &[N, T] : m_Enums)
        delete T;
    
    for (auto &[N, T] : m_Structs)
        delete T;
    
    for (auto &[N, T] : m_StructSpecs)
        delete T;
    
    for (auto &[N, T] : m_Dependents)
        delete T;
    
    for (auto &[N, D] : m_Deferred)
        delete D;
    
    for (auto &FT : m_FunctionTypes)
        delete FT;

    m_Primitives.clear();
    m_Enums.clear();
    m_Structs.clear();
    m_StructSpecs.clear();
    m_Dependents.clear();
    m_Deferred.clear();
    m_FunctionTypes.clear();
    m_Externals.clear();
}

Type *Context::resolveType(const String &name, const Scope *scope, 
                           const Metadata &md, bool specialize) {
    if (NamedDecl *N = scope->lookup(name)) {
        if (TypeDecl *TD = dynamic_cast<TypeDecl *>(N))
            return TD->getDefinedType();

        return nullptr;
    }

    if (name.back() == '*') {
        String pointeeName = name.substr(0, name.size() - 1);
        Type *pointeeType = resolveType(pointeeName, scope, md, specialize);
        if (!pointeeType)
            return nullptr;

        return PointerType::get(this, pointeeType);
    }

    auto LBrack = name.find_last_of('[');
    auto RBrack = name.find_last_of(']');
    if (LBrack != std::string::npos && RBrack != std::string::npos) {
        String elementName = name.substr(0, LBrack);
        Type *elementType = resolveType(elementName, scope, md, specialize);
        if (!elementType)
            return nullptr;
        
        unsigned size = std::stoul(name.substr(LBrack + 1, RBrack - LBrack - 1));
        return ArrayType::get(this, elementType, size);
    }

    auto LAngle = name.find_last_of('<');
    auto RAngle = name.find_last_of('>');
    if (LAngle != std::string::npos && RAngle != std::string::npos) {
        String tmplName = name.substr(0, LAngle);
        StructDecl *tmpl = nullptr;
        if (NamedDecl *N = scope->lookup(tmplName)) {
            tmpl = dynamic_cast<StructDecl *>(N);
            if (!tmpl || !tmpl->isTemplate())
                fatal("specialization base type is not a template: " + tmplName, &md);
        }

        String args = name.substr(LAngle + 1, RAngle - LAngle - 1);
        std::vector<Type *> typeArgs;
        unsigned start = 0;
        unsigned end = 0;
        unsigned level = 0;

        for (unsigned i = 0; i <= args.length(); ++i) {
            char c = (i < args.length()) ? args[i] : ',';
            if (c == '<')
                level++;
            else if (c == '>')
                level--;

            else if (c == ',' && level == 0) {
                String argName = args.substr(start, i - start);
                argName.erase(0, argName.find_first_not_of(" \t"));
                argName.erase(argName.find_last_not_of(" \t") + 1);

                Type *argType = resolveType(argName, scope, md, specialize);
                if (!argType)
                    return nullptr;

                typeArgs.push_back(argType);
                start = i + 1;
            }
        }

        if (!specialize)
            return nullptr;

        for (auto &arg : typeArgs)
            if (arg->isParamDependent())
                return DependentTemplateStructType::get(this, tmpl, typeArgs);

        return tmpl->fetchSpecialization(typeArgs)->getDefinedType();
    }
    
    auto prim_it = m_Primitives.find(name);
    if (prim_it != m_Primitives.end())
        return prim_it->second;

    auto enum_it = m_Enums.find(name);
    if (enum_it != m_Enums.end())
        return enum_it->second;

    auto struct_it = m_Structs.find(name);
    if (struct_it != m_Structs.end())
        return struct_it->second;

    auto spec_it = m_StructSpecs.find(name);
    if (spec_it != m_StructSpecs.end())
        return spec_it->second;

    auto ext_it = m_Externals.find(name);
    if (ext_it != m_Externals.end())
        return ext_it->second;

    return nullptr;
}

void Context::importType(Type *T, const String &N) {
    assert(T && "Type cannot be null.");

    auto it = m_Externals.find(N.empty() ? T->getName() : N);
    if (it != m_Externals.end())
        fatal("multiple types with same name: " + N, nullptr);

    m_Externals[N.empty() ? T->getName() : N] = T;
}

void Context::sanitate() {
    // For each type result, try to resolve its concrete type from the pool.
    for (auto &[name, defer] : m_Deferred) {
        Type *concrete = resolveType(name, defer->getScope(), defer->getMetadata(), false);
        if (concrete)
            defer->setUnderlying(concrete);
    }

    for (auto &[name, defer] : m_Deferred) {
        Type *concrete = resolveType(name, defer->getScope(), defer->getMetadata(), true);
        if (!concrete)
            // If the type is not found, it is unresolved at this point.
            fatal("unresolved type: " + name, &defer->getMetadata());

        defer->setUnderlying(concrete);
    }
}
