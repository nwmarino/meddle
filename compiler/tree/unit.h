#ifndef MEDDLE_UNIT_H
#define MEDDLE_UNIT_H

#include "context.h"
#include "decl.h"
#include "nameres.h"
#include "scope.h"

namespace meddle {

class TranslationUnit final {
    friend class CCGN;
    friend class NameResolution;
    friend class Sema;
    
    File m_File;
    Context m_Context;
    Scope *m_Scope;
    std::vector<Decl *> m_Decls;

public:
    TranslationUnit(const File &F) 
      : m_File(F), m_Context(this), m_Scope(new Scope) {}

    ~TranslationUnit() {
        for (auto &D : m_Decls)
            delete D;

        delete m_Scope;
    }

    void accept(Visitor *V) { V->visit(this); }

    const File &getFile() const { return m_File; }

    Context *getContext() { return &m_Context; }

    Scope *getScope() const { return m_Scope; }

    const std::vector<Decl *> getDecls() const { return m_Decls; }

    void addDecl(Decl *D) { m_Decls.push_back(D); }
};

} // namespace meddle

#endif // MEDDLE_UNIT_H
