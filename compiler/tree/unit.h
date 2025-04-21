#ifndef MEDDLE_UNIT_H
#define MEDDLE_UNIT_H

#include "context.h"
#include "decl.h"
#include "scope.h"

namespace meddle {

class TranslationUnit final {
    File m_File;
    Context m_Context;
    Scope *m_Scope;
    std::vector<Decl *> m_Decls;

public:
    TranslationUnit(const File &F) 
      : m_File(F), m_Context(this), m_Scope(new Scope) {}

    ~TranslationUnit() {
        delete m_Scope;

        for (auto decl : m_Decls)
            delete decl;
    }

    template<typename vT>
    void accept(vT &visitor) {
        visitor.visit(this);
    }

    const File &getFile() const { return m_File; }

    Context *getContext() { return &m_Context; }

    Scope *getScope() const { return m_Scope; }

    const std::vector<Decl *> getDecls() const { return m_Decls; }

    void addDecl(Decl *D) { m_Decls.push_back(D); }
};

} // namespace meddle

#endif // MEDDLE_UNIT_H
