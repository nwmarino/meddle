#ifndef MEDDLE_SCOPE_H
#define MEDDLE_SCOPE_H

#include "decl.h"

namespace meddle {

class Scope final {
    Scope *m_Parent;
    std::vector<NamedDecl *> m_Decls;

public:
    Scope(Scope *P = nullptr) : m_Parent(P) {}

    Scope *getParent() const { return m_Parent; }

    const std::vector<NamedDecl *> getDecls() const { return m_Decls; }

    NamedDecl *lookup(const String &N) const;

    void addDecl(NamedDecl *D);
};

} // namespace meddle

#endif // MEDDLE_SCOPE_H
