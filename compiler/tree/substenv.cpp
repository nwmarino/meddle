#include "decl.h"
#include "substenv.h"
#include "type.h"

using namespace meddle;

Type *SubstEnv::substType(Context *ctx, Type *ty) {
    if (ty->isDeferred()) {
        DeferredType *defer = ty->asDeferred();
        return substType(ctx, ty->asDeferred()->getUnderlying());
    } else if (ty->isArray()) {
        return ArrayType::get(ctx, substType(ctx, ty->asArray()->getElement()), 
            ty->asArray()->getSize());
    } else if (ty->isPointer()) {
        return PointerType::get(ctx, substType(ctx, ty->asPointer()->getPointee()));
    } else if (auto *param = dynamic_cast<TemplateParamType *>(ty)) {
        return substParam(param);
    } else if (auto *spec = dynamic_cast<TemplateStructType *>(ty)) {
        std::vector<Type *> substArgs;
        substArgs.reserve(spec->getArgs().size());
        bool changed = false;

        for (auto &arg : spec->getArgs()) {
            Type *concArg = substType(ctx, arg);
            substArgs.push_back(concArg);
            if (concArg != arg)
                changed = true;
        }

        if (!changed)
            return spec;

        return TemplateStructType::get(ctx, spec->getTemplateDecl(), substArgs);
    } else if (auto *dep = dynamic_cast<DependentTemplateStructType *>(ty)) {
        std::vector<Type *> nonDependents;
        nonDependents.reserve(dep->getArgs().size());

        for (auto &arg : dep->getArgs())
            nonDependents.push_back(substType(ctx, arg));

        return TemplateStructType::get(ctx, dep->getTemplateDecl(), nonDependents);
    }

    return ty;
}

std::unordered_map<TemplateParamType *, Type *> 
meddle::getMapping(const std::vector<TemplateParamDecl *> &params, 
           const std::vector<Type *> &args) {
    std::unordered_map<TemplateParamType *, Type *> mapping;
    mapping.reserve(args.size());
    for (unsigned i = 0; i != params.size(); ++i)
        mapping[static_cast<TemplateParamType *>(
            params.at(i)->getDefinedType())] = args.at(i);

    return mapping;
}
