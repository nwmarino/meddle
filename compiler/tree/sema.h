#ifndef MEDDLE_SEMA_H
#define MEDDLE_SEMA_H

#include "decl.h"
#include "expr.h"
#include "visitor.h"
#include "../core/options.h"

namespace meddle {

class Sema final : public Visitor {
    enum class LoopKind {
        None = 0,
        Until,
    } m_Loop = LoopKind::None;

    Options m_Opts;
    TranslationUnit *m_Unit;
    FunctionDecl *m_Function;

public:
    Sema(const Options &opts, TranslationUnit *U);

    void visit(TranslationUnit *unit) override;

    void visit(FunctionDecl *decl) override;
    void visit(VarDecl *decl) override;
    void visit(FieldDecl *decl) override;
    void visit(StructDecl *decl) override;

    void visit(BreakStmt *stmt) override;
    void visit(ContinueStmt *stmt) override;
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
    void visit(CallExpr *expr) override;
    void visit(CastExpr *expr) override;
    void visit(FieldInitExpr *expr) override;
    void visit(InitExpr *expr) override;
    void visit(MethodCallExpr *expr) override;
    void visit(ParenExpr *expr) override;
    void visit(SubscriptExpr *expr) override;
    void visit(TypeSpecExpr *expr) override;
    void visit(UnitSpecExpr *expr) override;
    void visit(UnaryExpr *expr) override;

    void visit(RuneSyscallExpr *expr) override;
};

} // namespace meddle

#endif // MEDDLE_SEMA_H
