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
    friend class UnitManager;
    
    String m_ID;
    File m_File;
    Context m_Context;
    Scope *m_Scope;
    std::vector<Decl *> m_Decls = {};
    std::vector<UseDecl *> m_Uses = {};
    std::vector<NamedDecl *> m_Imports = {};
    std::vector<NamedDecl *> m_Exports = {};

public:
    TranslationUnit(const String &ID, const File &F) 
      : m_ID(ID), m_File(F), m_Context(this), m_Scope(new Scope) {}

    ~TranslationUnit() {
        for (auto &U : m_Uses)
            delete U;
        m_Uses.clear();

        for (auto &D : m_Decls)
            delete D;
        m_Decls.clear();

        m_Exports.clear();

        delete m_Scope;
    }

    void accept(Visitor *V) { V->visit(this); }

    const String &getID() const { return m_ID; }

    const File &getFile() const { return m_File; }

    Context *getContext() { return &m_Context; }

    Scope *getScope() const { return m_Scope; }

    const std::vector<Decl *> &getDecls() const { return m_Decls; }

    const std::vector<UseDecl *> &getUses() const { return m_Uses; }

    void addDecl(Decl *D) { 
        D->setPUnit(this); 
        m_Decls.push_back(D); 
    }

    void addUse(UseDecl *U) { 
        U->setPUnit(this);
        m_Uses.push_back(U); 
    }

    void addImport(NamedDecl *D) { m_Imports.push_back(D); }

    void addExport(NamedDecl *D) { m_Exports.push_back(D); }

    const std::vector<NamedDecl *> &getImports() const { return m_Imports; }

    const std::vector<NamedDecl *> &getExports() const { return m_Exports; }

    void print(std::ostream &OS) const;
};

} // namespace meddle

#endif // MEDDLE_UNIT_H
