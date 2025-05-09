#ifndef MEDDLE_SUBSTENV_H
#define MEDDLE_SUBSTENV_H

#include "../tree/type.h"

#include <cassert>
#include <unordered_map>

namespace meddle {

class SubstEnv final {
    SubstEnv *m_Parent;
    std::unordered_map<TemplateParamType *, Type *> m_Mapping;

public:
    SubstEnv(std::unordered_map<TemplateParamType *, Type *> map, 
             SubstEnv *parent = nullptr)
      : m_Parent(parent), m_Mapping(map) {}

    const std::unordered_map<TemplateParamType *, Type *> getMapping() const
    { return m_Mapping; }

    SubstEnv *getParent() const { return m_Parent; }

    void setParent(SubstEnv *parent) { m_Parent = parent; }

    Type *substType(Context *ctx, Type *ty);

    Type *substParam(TemplateParamType *param) {
        auto it = m_Mapping.find(param);
        if (it != m_Mapping.end())
            return it->second;

        if (m_Parent)
            return m_Parent->substParam(param);

        assert(false && "Parameter type not found in substitution environment.");
    }
};

std::unordered_map<TemplateParamType *, Type *> 
getMapping(const std::vector<TemplateParamDecl *> &params, 
           const std::vector<Type *> &args);

} // namespace meddle

#endif // MEDDLE_SUBSTENV_H
