#ifndef MEDDLE_NAME_RESOLUTION_H
#define MEDDLE_NAME_RESOLUTION_H

#include "visitor.h"
#include "../core/options.h"

namespace meddle {

class NameResolution : public Visitor {
    Options m_Opts;
    TranslationUnit *m_Unit;

public:
    NameResolution(const Options &opts, TranslationUnit *U);

    void visit(TranslationUnit *unit) override;

    void visit(FunctionDecl *decl) override;
    void visit(VarDecl *decl) override;
    void visit(ParamDecl *decl) override;

    void visit(CompoundStmt *stmt) override;
    void visit(DeclStmt *stmt) override;
    void visit(RetStmt *stmt) override; 

    void visit(IntegerLiteral *expr) override;
    void visit(RefExpr *expr) override;
};

} // namespace meddle

#endif // MEDDLE_NAME_RESOLUTION_H
