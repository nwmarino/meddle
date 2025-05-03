#ifndef MEDDLE_NAME_RESOLUTION_H
#define MEDDLE_NAME_RESOLUTION_H

#include "scope.h"
#include "visitor.h"
#include "../core/options.h"

namespace meddle {

class NameResolution final : public Visitor {
    enum class Phase {
        Shallow, Recurse
    } m_Phase;
    Options m_Opts;
    TranslationUnit *m_Unit;
    Scope *m_Scope;

public:
    NameResolution(const Options &opts, TranslationUnit *U);

    void visit(TranslationUnit *unit) override;

    void visit(FunctionDecl *decl) override;
    void visit(VarDecl *decl) override;
    
    void visit(CompoundStmt *stmt) override;
    void visit(DeclStmt *stmt) override;
    void visit(ExprStmt *stmt) override;
    void visit(IfStmt *stmt) override;
    void visit(CaseStmt *stmt) override;
    void visit(MatchStmt *stmt) override;
    void visit(RetStmt *stmt) override; 
    void visit(UntilStmt *stmt) override;

    void visit(ArrayExpr *expr) override;
    void visit(BinaryExpr *expr) override;
    void visit(CastExpr *expr) override;
    void visit(ParenExpr *expr) override;
    void visit(RefExpr *expr) override;
    void visit(SizeofExpr *expr) override;
    void visit(UnaryExpr *expr) override;
};

} // namespace meddle

#endif // MEDDLE_NAME_RESOLUTION_H
