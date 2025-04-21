#include "scope.h"
#include "../core/logger.h"

using namespace meddle;

NamedDecl *Scope::lookup(const String &N) const {
    for (auto &D : m_Decls)
        if (D->getName() == N)
            return D;

    if (m_Parent)
        return m_Parent->lookup(N);

    return nullptr;
}

void Scope::addDecl(NamedDecl *D) {
    if (lookup(D->getName()))
        fatal("duplicate declaration: " + D->getName(), &D->getMetadata());

    m_Decls.push_back(D);
}
