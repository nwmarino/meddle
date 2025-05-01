#ifndef MEDDLE_CODEGEN_H
#define MEDDLE_CODEGEN_H

#include "../tree/decl.h"
#include "../tree/expr.h"
#include "../tree/stmt.h"
#include "../tree/type.h"
#include "../tree/visitor.h"
#include "../core/options.h"
#include "../mir/builder.h"
#include "../mir/segment.h"
#include "../mir/value.h"

#include <unordered_map>

namespace meddle {

class CGN final : public Visitor {
    enum class ValueContext {
        LValue, RValue
    } m_VC;

    enum class Phase {
        Define, Declare
    } m_Phase;

    Options m_Opts;
    TranslationUnit *m_Unit;
    mir::Segment *m_Segment;
    mir::Builder m_Builder;
    mir::Function *m_Function = nullptr;
    mir::Value *m_Value = nullptr;
    mir::BasicBlock *m_Merge = nullptr;
    mir::BasicBlock *m_Cond = nullptr;
    std::unordered_map<NamedDecl *, String> m_Mangled = {};

    String mangle_name(NamedDecl *D);

    mir::Type *cgn_type(Type *T);

    mir::Value *inject_cmp(mir::Value *V);

    void declare_function(FunctionDecl *FD);
    void define_function(FunctionDecl *FD);

public:
    CGN(const Options &opts, TranslationUnit *U, mir::Segment *S);

    void visit(TranslationUnit *unit) override;

    void visit(FunctionDecl *decl) override;
    void visit(VarDecl *decl) override;
    void visit(ParamDecl *decl) override;

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

    void visit(IntegerLiteral *expr) override;
    void visit(FloatLiteral *expr) override;
    void visit(CharLiteral *expr) override;
    void visit(StringLiteral *expr) override;
    void visit(NilLiteral *expr) override;
    void visit(CastExpr *expr) override;
    void visit(RefExpr *expr) override;
};

} // namespace meddle

#endif // MEDDLE_CODEGEN_H
