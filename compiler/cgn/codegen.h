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

#include <string>
#include <unordered_map>

namespace meddle {

class CGN final : public Visitor {
    enum class ValueContext {
        LValue, RValue
    } m_VC;

    enum class Phase {
        Define, Declare
    } m_Phase;

    enum TypeClass {
        Unknown = 0,
        SInt,
        UInt,
        Float, 
        Pointer, 
        Aggregate,
    };

    Options m_Opts;
    TranslationUnit *m_Unit;
    mir::Segment *m_Segment;
    mir::Builder m_Builder;
    mir::Function *m_Function = nullptr;
    mir::Value *m_Value = nullptr;
    mir::Value *m_Place = nullptr;
    mir::BasicBlock *m_Merge = nullptr;
    mir::BasicBlock *m_Cond = nullptr;
    std::unordered_map<NamedDecl *, String> m_Mangled = {};

    String mangle_name(NamedDecl *D);

    mir::Type *cgn_type(Type *T);
    TypeClass type_class(Type *T) const;

    mir::Value *inject_cmp(mir::Value *V);

    void declare_function(FunctionDecl *FD);
    void define_function(FunctionDecl *FD);

    void cgn_aggregate_init(mir::Value *base, Expr *expr, Type *ty);

    void cgn_assign(BinaryExpr *BIN);
    void cgn_add_assign(BinaryExpr *BIN);
    void cgn_sub_assign(BinaryExpr *BIN);
    void cgn_mul_assign(BinaryExpr *BIN);
    void cgn_div_assign(BinaryExpr *BIN);
    void cgn_mod_assign(BinaryExpr *BIN);
    void cgn_and_assign(BinaryExpr *BIN);
    void cgn_or_assign(BinaryExpr *BIN);
    void cgn_xor_assign(BinaryExpr *BIN);
    void cgn_shl_assign(BinaryExpr *BIN);
    void cgn_shr_assign(BinaryExpr *BIN);
    
    void cgn_equals(BinaryExpr *BIN);
    void cgn_not_equals(BinaryExpr *BIN);
    void cgn_less(BinaryExpr *BIN);
    void cgn_less_eq(BinaryExpr *BIN);
    void cgn_greater(BinaryExpr *BIN);
    void cgn_greater_eq(BinaryExpr *BIN);

    void cgn_add(BinaryExpr *BIN);
    void cgn_sub(BinaryExpr *BIN);
    void cgn_mul(BinaryExpr *BIN);
    void cgn_div(BinaryExpr *BIN);
    void cgn_mod(BinaryExpr *BIN);
    void cgn_and(BinaryExpr *BIN);
    void cgn_or(BinaryExpr *BIN);
    void cgn_xor(BinaryExpr *BIN);
    void cgn_shl(BinaryExpr *BIN);
    void cgn_shr(BinaryExpr *BIN);
    
    void cgn_logic_and(BinaryExpr *BIN);
    void cgn_logic_or(BinaryExpr *BIN);

    void cgn_not(UnaryExpr *UN);
    void cgn_logic_not(UnaryExpr *UN);
    void cgn_neg(UnaryExpr *UN);
    void cgn_addrof(UnaryExpr *UN);
    void cgn_deref(UnaryExpr *UN);
    void cgn_inc(UnaryExpr *UN);
    void cgn_dec(UnaryExpr *UN);
    
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

    void visit(BoolLiteral *expr) override;
    void visit(IntegerLiteral *expr) override;
    void visit(FloatLiteral *expr) override;
    void visit(CharLiteral *expr) override;
    void visit(StringLiteral *expr) override;
    void visit(NilLiteral *expr) override;

    void visit(ArrayExpr *expr) override;
    void visit(BinaryExpr *expr) override;
    void visit(CastExpr *expr) override;
    void visit(ParenExpr *expr) override;
    void visit(RefExpr *expr) override;
    void visit(SizeofExpr *expr) override;
    void visit(SubscriptExpr *expr) override;
    void visit(UnaryExpr *expr) override;
};

} // namespace meddle

#endif // MEDDLE_CODEGEN_H
