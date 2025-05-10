#include "decl.h"
#include "scope.h"
#include "stmt.h"
#include "substenv.h"
#include "type.h"
#include "unit.h"
#include "../core/logger.h"

#include <iostream>
#include <unordered_map>

using namespace meddle;

FunctionDecl::FunctionDecl(const Runes &runes, const Metadata &md, 
                           const String &name, FunctionType *ty, Scope *scope, 
                           std::vector<ParamDecl *> params, Stmt *body, 
                           std::vector<TemplateParamDecl *> tmplParams,
                           StructDecl *parent)
  : NamedDecl(runes, md, name), m_Type(ty), m_Scope(scope), m_Params(params),
    m_Body(body), m_TemplateParams(tmplParams), m_Parent(parent) {
    for (auto &param : params)
        param->setParent(this);
}

FunctionDecl::~FunctionDecl() {
    for (auto &param : m_Params)
        delete param;

    for (auto &param : m_TemplateParams)
        delete param;

    for (auto &spec : m_TemplateSpecs)
        delete spec;

    m_Params.clear();
    m_TemplateParams.clear();
    m_TemplateSpecs.clear();
    delete m_Scope;
    if (m_Body)
        delete m_Body;
}

ParamDecl *FunctionDecl::getParam(const String &name) const {
    for (auto &param : m_Params)
        if (param->getName() == name)
            return param;

    return nullptr;
}

Type *FunctionDecl::getParamType(unsigned i) const {
    assert(i < m_Params.size() && "Index out of bounds.");
    return m_Params[i]->getType();
}

TemplateParamDecl *FunctionDecl::getTemplateParam(const String &name) const {
    for (auto &param : m_TemplateParams)
        if (param->getName() == name)
            return param;

    return nullptr;
}

String FunctionDecl::getConcreteName(const std::vector<Type *> &args) const {
    String name = m_Name + "<";
    for (auto &ty : args)
        name += ty->getName() + (ty == args.back() ? "" : ", ");
    
    return name + ">";
}

FunctionTemplateSpecializationDecl*
FunctionDecl::findSpecialization(const std::vector<Type *> &args) const {
    for (auto &spec : m_TemplateSpecs)
        if (spec->compareArgs(args))
            return spec;

    return nullptr;
}

FunctionTemplateSpecializationDecl*
FunctionDecl::fetchSpecialization(const std::vector<Type *> &args) {
    if (auto *spec = findSpecialization(args))
        return spec;

    return createSpecialization(args);
}

FunctionTemplateSpecializationDecl*
FunctionDecl::createSpecialization(const std::vector<Type *> &args) {
    // Create a mapping between the parameterized types and the concrete type
    // arguments for the new specialization. This is for substituting types.
    SubstEnv *env = new SubstEnv(getMapping(m_TemplateParams, args));

    // Specialize the function type with the concrete arguments.
    Type *concreteRetTy = env->substType(m_PUnit->getContext(), 
        m_Type->getReturnType());
    std::vector<Type *> concreteParamTys;
    concreteParamTys.reserve(m_Type->getNumParams());
    for (auto &param : m_Type->getParams())
        concreteParamTys.push_back(env->substType(m_PUnit->getContext(), param));

    FunctionType *concreteFT = FunctionType::create(m_PUnit->getContext(), 
        concreteParamTys, concreteRetTy);

    // Specialize the function parameters with the concrete arguments.
    std::vector<ParamDecl *> concreteParams;
    concreteParams.reserve(getNumParams());
    for (unsigned i = 0; i != getNumParams(); ++i) {
        ParamDecl *param = m_Params.at(i);
        concreteParams.push_back(new ParamDecl(param->getRunes(),
            param->getMetadata(), param->getName(), concreteParamTys.at(i), i));
    }

    // Create a new scope for the empty, specialized function.
    auto scope = new Scope();
    for (auto &param : concreteParams)
        scope->addDecl(param);

    // Create the empty, specialized function.
    auto specialization = new FunctionTemplateSpecializationDecl(
        this,
        getConcreteName(args),
        concreteFT,
        scope,
        concreteParams,
        nullptr, // body
        args,
        env->getMapping()
    );
    specialization->setPUnit(m_PUnit);

    delete env;
    m_TemplateSpecs.push_back(specialization);
    return specialization;
}

VarDecl::VarDecl(const Runes &R, const Metadata &M, const String &N, 
                 Type *T, Expr *I, bool mut, bool global)
  : NamedDecl(R, M, N), m_Type(T), m_Init(I), m_Mut(mut), m_Global(global) {}

ParamDecl::ParamDecl(const Runes &R, const Metadata &M, const String &N,
                     Type *T, unsigned I)
  : VarDecl(R, M, N, T, nullptr, true, false), m_Index(I), 
    m_Parent(nullptr) {}

StructDecl::StructDecl(const Runes &runes, const Metadata &md, 
                       const String &name, Type *ty, Scope *scope, 
                       std::vector<FieldDecl *> fields, 
                       std::vector<FunctionDecl *> funcs, 
                       std::vector<TemplateParamDecl *> tmplParams)
  : TypeDecl(runes, md, name, ty), m_Scope(scope), m_Fields(fields),
    m_Functions(funcs), m_TemplateParams(tmplParams) {
    for (auto &field : m_Fields)
        field->setParent(this);
    for (auto &fn : m_Functions)
        fn->setParent(this);
}

StructDecl::~StructDecl() {
    for (auto &field : m_Fields)
        delete field;

    for (auto &fn : m_Functions)
        delete fn;

    for (auto &param : m_TemplateParams)
        delete param;

    for (auto &spec : m_TemplateSpecs)
        delete spec;

    m_Fields.clear();
    m_Functions.clear();
    m_TemplateParams.clear();
    m_TemplateSpecs.clear();
    delete m_Scope;
}

FieldDecl *StructDecl::getField(const String &name) const {
    for (auto &field : m_Fields)
        if (field->getName() == name)
            return field;

    return nullptr;
}

FunctionDecl *StructDecl::getFunction(const String &name) const {
    for (auto &fn : m_Functions)
        if (fn->getName() == name)
            return fn;

    return nullptr;
}

TemplateParamDecl *StructDecl::getTemplateParam(const String &name) const {
    for (auto &param : m_TemplateParams)
        if (param->getName() == name)
            return param;

    return nullptr;
}

String StructDecl::getConcreteName(const std::vector<Type *> &args) const {
    String name = m_Name + "<";
    for (auto &ty : args)
        name += ty->getName() + (ty == args.back() ? "" : ", ");

    return name + ">";
}

StructTemplateSpecializationDecl*
StructDecl::findSpecialization(const std::vector<Type *> &args) const {
    for (auto &spec : m_TemplateSpecs)
        if (spec->compareArgs(args))
            return spec;

    return nullptr;
}

StructTemplateSpecializationDecl*
StructDecl::fetchSpecialization(const std::vector<Type *> &args) {
    if (auto *spec = findSpecialization(args))
        return spec;

    return createSpecialization(args);
}

StructTemplateSpecializationDecl*
StructDecl::createSpecialization(const std::vector<Type *> &args) {
    // Create a mapping between the parameterized types and the concrete type
    // arguments for the new specialization. This is for substituting types.
    SubstEnv *env = new SubstEnv(getMapping(m_TemplateParams, args));
    
    // Specialize the struct type with the concrete arguments.
    StructType *tmplTy = m_Type->asStruct();
    std::vector<Type *> concreteFieldTys;
    concreteFieldTys.reserve(tmplTy->getNumFields());
    for (auto &field : tmplTy->getFields())
        concreteFieldTys.push_back(env->substType(m_PUnit->getContext(), field));

    // Create a new scope for the specialized structure.
    auto structScope = new Scope();

    // Specialize the structure fields with the concrete arguments.
    std::vector<FieldDecl *> concreteFields;
    concreteFields.reserve(concreteFieldTys.size());
    for (unsigned i = 0; i != concreteFieldTys.size(); ++i) {
        FieldDecl *tmplField = m_Fields.at(i);
        FieldDecl *concField = new FieldDecl(
            tmplField->getRunes(),
            tmplField->getMetadata(),
            tmplField->getName(),
            concreteFieldTys.at(i),
            i
        );
        concreteFields.push_back(concField);
        structScope->addDecl(concField);
    }

    std::vector<FunctionDecl *> concreteFunctions;
    concreteFunctions.reserve(getNumFunctions());
    for (auto &fn : m_Functions) {
        FunctionType *tmplFnTy = fn->getType();

        Type *concreteRetTy = env->substType(m_PUnit->getContext(), 
            tmplFnTy->getReturnType());
        std::vector<Type *> concreteParamTys;
        concreteParamTys.reserve(tmplFnTy->getNumParams());
        for (auto &param : tmplFnTy->getParams())
            concreteParamTys.push_back(env->substType(m_PUnit->getContext(), 
                param));

        FunctionType *concreteFT = FunctionType::create(m_PUnit->getContext(), 
            concreteParamTys, concreteRetTy);

        auto fnScope = new Scope(structScope);

        std::vector<ParamDecl *> concreteParams;
        concreteParams.reserve(fn->getNumParams());
        for (unsigned i = 0; i != fn->getNumParams(); ++i) {
            ParamDecl *tmplParam = fn->getParam(i);
            ParamDecl *concParam = new ParamDecl(
                tmplParam->getRunes(),
                tmplParam->getMetadata(),
                tmplParam->getName(),
                concreteParamTys.at(i),
                i
            );
            concreteParams.push_back(concParam);
            fnScope->addDecl(concParam);
        }

        FunctionDecl *concFunction = new FunctionDecl(
            fn->getRunes(),
            fn->getMetadata(),
            fn->getName(),
            concreteFT,
            fnScope,
            concreteParams,
            nullptr // body
        );
        concFunction->setPUnit(m_PUnit);

        concreteFunctions.push_back(concFunction);
        structScope->addDecl(concFunction);

        if (env->getParent()) {
            SubstEnv *old = env;
            env = env->getParent();
            delete old;
        }
    }

    String name = getConcreteName(args);

    TemplateStructType *concTy = TemplateStructType::create(
        m_PUnit->getContext(), name, concreteFieldTys, args);

    auto specialization = new StructTemplateSpecializationDecl(
        this,
        getConcreteName(args),
        concTy,
        structScope,
        concreteFields,
        concreteFunctions,
        args,
        env->getMapping()
    );
    specialization->setPUnit(m_PUnit);

    concTy->setDecl(specialization);
    delete env;
    m_TemplateSpecs.push_back(specialization);
    return specialization;
}

static bool compareArgs(const std::vector<Type *> &args1, 
                        const std::vector<Type *> &args2) {
    if (args1.size() != args2.size())
        return false;

    for (unsigned i = 0, n = args1.size(); i != n; ++i)
        if (!args1[i]->compare(args2[i]))
            return false;

    return true;
}

bool 
FunctionTemplateSpecializationDecl::compareArgs(const std::vector<Type *> &args) const {
    return ::compareArgs(m_Args, args);
}

bool
StructTemplateSpecializationDecl::compareArgs(const std::vector<Type *> &args) const { 
    return ::compareArgs(m_Args, args); 
}
