#include "decl.h"
#include "scope.h"
#include "stmt.h"
#include "type.h"
#include "unit.h"
#include "../core/logger.h"

#include <unordered_map>

using namespace meddle;

FunctionDecl::FunctionDecl(const Runes &R, const Metadata &M, 
                           const String &N, FunctionType *T, Scope *S, 
                           std::vector<ParamDecl *> P, Stmt *B,
                           StructDecl *SP)
  : NamedDecl(R, M, N), m_Type(T), m_Scope(S), m_Params(std::move(P)),
    m_Body(B), m_Parent(SP) {
    for (auto &param : P)
        param->setParent(this);
}

FunctionDecl::~FunctionDecl() {
    if (m_Body)
        delete m_Body;

    for (auto &param : m_Params)
        delete param;

    m_Params.clear();
    delete m_Scope;
}

Type *FunctionDecl::getParamType(unsigned i) const {
    assert(i < m_Params.size() && "Index out of bounds.");
    return m_Params[i]->getType();
}

VarDecl::VarDecl(const Runes &R, const Metadata &M, const String &N, 
                 Type *T, Expr *I, bool mut, bool global)
  : NamedDecl(R, M, N), m_Type(T), m_Init(I), m_Mut(mut), m_Global(global) {}

ParamDecl::ParamDecl(const Runes &R, const Metadata &M, const String &N,
                     Type *T, unsigned I)
  : VarDecl(R, M, N, T, nullptr, true, false), m_Index(I), 
    m_Parent(nullptr) {}

StructDecl::StructDecl(const Runes &R, const Metadata &M, const String &N, 
                       Type *T, Scope *S, std::vector<FieldDecl *> F, 
                       std::vector<FunctionDecl *> Funcs)
  : TypeDecl(R, M, N, T), m_Scope(S), m_Fields(std::move(F)),
    m_Functions(std::move(Funcs)) {
    for (auto &field : m_Fields)
        field->setParent(this);
    for (auto &function : m_Functions)
        function->setParent(this);
}

StructDecl::~StructDecl() {
    for (auto *field : m_Fields)
        delete field;
    for (auto *function : m_Functions)
        delete function;

    m_Fields.clear();
    m_Functions.clear();
    delete m_Scope;
}

Type*
TemplateDecl::substType(Type *T, std::unordered_map<TemplateParamType *, Type *> M) const {
    Context *ctx = m_PUnit->getContext();

    if (T->isDeferred()) {
        return substType(T->asDeferred()->getUnderlying(), M);
    } else if (T->isArray()) {
        return ArrayType::get(ctx, substType(T->asArray()->getElement(), M), 
            T->asArray()->getSize());
    } else if (T->isPointer()) {
        return PointerType::get(ctx, substType(T->asPointer()->getPointee(), M));
    } else if (auto *TPT = dynamic_cast<TemplateParamType *>(T)) {
        auto it = M.find(TPT);
        if (it != M.end())
            return it->second;

        fatal("template parameter unfulfillfed: " + T->getName(), &m_Metadata);
    } else if (auto *TST = dynamic_cast<TemplateStructType *>(T)) {
        std::vector<Type *> substArgs;
        substArgs.reserve(TST->getArgs().size());
        bool changed = false;

        for (auto &arg : TST->getArgs()) {
            Type *newArg = substType(arg, M);
            substArgs.push_back(newArg);
            if (newArg != arg)
                changed = true;
        }

        if (!changed)
            return TST;

        return TemplateStructType::get(ctx, TST->getTemplateDecl(), substArgs);
    } else if (auto *DTST = dynamic_cast<DependentTemplateStructType *>(T)) {
        std::vector<Type *> nonDependents;
        nonDependents.reserve(DTST->getArgs().size());
        for (auto &arg : DTST->getArgs())
            nonDependents.push_back(substType(arg, M));

        return TemplateStructType::get(ctx, DTST->getTemplateDecl(), 
            nonDependents);
    }

    return T;
}

static std::unordered_map<TemplateParamType *, Type *> 
getMapping(const std::vector<TemplateParamDecl *> &params, 
           const std::vector<Type *> &args) {
    std::unordered_map<TemplateParamType *, Type *> mapping;
    mapping.reserve(args.size());
    for (unsigned i = 0; i != params.size(); ++i)
        mapping[static_cast<TemplateParamType *>(
            params.at(i)->getDefinedType())] = args.at(i);

    return mapping;
}

TemplateStructDecl::~TemplateStructDecl() {
    for (auto &spec : m_Specs)
        delete spec;

    m_Specs.clear();
}

StructTemplateSpecializationDecl*
TemplateStructDecl::findSpecialization(const std::vector<Type *> &args) const {
    for (auto &spec : m_Specs)
        if (spec->compareArgs(args))
            return spec;

    return nullptr;
}

StructTemplateSpecializationDecl*
TemplateStructDecl::fetchSpecialization(const std::vector<Type *> &args) {
    if (StructTemplateSpecializationDecl *spec = findSpecialization(args))
        return spec;

    return createSpecialization(args);
}

StructTemplateSpecializationDecl*
TemplateStructDecl::createSpecialization(const std::vector<Type *> &args) {
    Context *ctx = m_PUnit->getContext();

    // Create a mapping between the parameterized types and the concrete type
    // arguments for the new specialization. This is for substituting types.
    auto mapping = getMapping(m_Params, args);
    
    // Specialize the struct type with the concrete arguments.
    StructType *tmplTy = getTemplatedStruct()->getDefinedType()->asStruct();
    std::vector<Type *> concreteFieldTys;
    concreteFieldTys.reserve(tmplTy->getNumFields());
    for (auto &field : tmplTy->getFields())
        concreteFieldTys.push_back(substType(field, mapping));

    // Create a new scope for the specialized structure.
    auto structScope = new Scope();

    // Specialize the structure fields with the concrete arguments.
    std::vector<FieldDecl *> concreteFields;
    concreteFields.reserve(concreteFieldTys.size());
    for (unsigned i = 0; i != concreteFieldTys.size(); ++i) {
        FieldDecl *tmplField = getTemplatedStruct()->getField(i);
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
    concreteFunctions.reserve(getTemplatedStruct()->getNumFunctions());
    for (auto &tmplFunction : getTemplatedStruct()->getFunctions()) {
        FunctionType *tmplFnTy = tmplFunction->getType();

        Type *concreteRetTy = substType(tmplFnTy->getReturnType(), mapping);
        std::vector<Type *> concreteParamTys;
        concreteParamTys.reserve(tmplFnTy->getNumParams());
        for (auto &param : tmplFnTy->getParams())
            concreteParamTys.push_back(substType(param, mapping));

        FunctionType *concreteFT = FunctionType::create(ctx, concreteParamTys, 
            concreteRetTy);

        auto fnScope = new Scope(structScope);

        std::vector<ParamDecl *> concreteParams;
        concreteParams.reserve(tmplFunction->getNumParams());
        for (unsigned i = 0; i != tmplFunction->getNumParams(); ++i) {
            ParamDecl *tmplParam = tmplFunction->getParam(i);
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
            tmplFunction->getRunes(),
            tmplFunction->getMetadata(),
            tmplFunction->getName(),
            concreteFT,
            fnScope,
            concreteParams,
            nullptr // body
        );

        concreteFunctions.push_back(concFunction);
        structScope->addDecl(concFunction);
    }

    TemplateStructType *concTy = TemplateStructType::create(ctx, concreteFieldTys, nullptr, args);
    auto specialization = new StructTemplateSpecializationDecl(
        this,
        getConcreteName(args),
        concTy,
        structScope,
        concreteFields,
        concreteFunctions,
        args,
        mapping
    );
    concTy->setDecl(specialization);

    m_Specs.push_back(specialization);
    return specialization;
}

TemplateFunctionDecl::~TemplateFunctionDecl() {
    for (auto &spec : m_Specs)
        delete spec;

    m_Specs.clear();
}

FunctionTemplateSpecializationDecl*
TemplateFunctionDecl::findSpecialization(const std::vector<Type *> &args) const {
    for (auto &S : m_Specs)
        if (S->compareArgs(args))
            return S;

    return nullptr;
}

FunctionTemplateSpecializationDecl*
TemplateFunctionDecl::fetchSpecialization(const std::vector<Type *> &args) {
    if (FunctionTemplateSpecializationDecl *spec = findSpecialization(args))
        return spec;

    return createSpecialization(args);
}

FunctionTemplateSpecializationDecl*
TemplateFunctionDecl::createSpecialization(const std::vector<Type *> &args) {
    FunctionDecl *FN = static_cast<FunctionDecl *>(m_Tmpl);
    FunctionType *FT = FN->getType();

    // Create a mapping between the parameterized types and the concrete type
    // arguments for the new specialization. This is for substituting types.
    auto mapping = getMapping(m_Params, args);
    
    // Specialize the function type with the concrete arguments.
    Type *concreteRetTy = substType(FT->getReturnType(), mapping);
    std::vector<Type *> concreteParamTys;
    concreteParamTys.reserve(FT->getNumParams());
    for (auto &P : FT->getParams())
        concreteParamTys.push_back(substType(P, mapping));

    FunctionType *concreteFT = FunctionType::create(m_PUnit->getContext(), 
        concreteParamTys, concreteRetTy);

    // Specialize the function parameters with the concrete arguments.
    std::vector<ParamDecl *> concreteParams;
    concreteParams.reserve(FN->getNumParams());
    for (unsigned i = 0; i != FN->getNumParams(); ++i) {
        ParamDecl *P = FN->getParam(i);
        concreteParams.push_back(new ParamDecl(P->getRunes(),
            P->getMetadata(), P->getName(), concreteParamTys.at(i), i));
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
        mapping
    );

    m_Specs.push_back(specialization);
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
