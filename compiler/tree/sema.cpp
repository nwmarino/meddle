#include "sema.h"
#include "expr.h"
#include "stmt.h"
#include "unit.h"
#include "../core/logger.h"

using namespace meddle;

/// Check that if a statement is not compounded then it is not declarative.
///
/// Specifically, this crashes on behaviour like:
///
/// if ...
///     mut x: ... = ... ;
static void checkCompoundedDeclStmt(Stmt *S) {
    if (dynamic_cast<CompoundStmt *>(S))
        return;

    if (dynamic_cast<DeclStmt *>(S))
        fatal("declarations must be in a compound statement", &S->getMetadata());
}

/// Perform a type check between an expected type and the actual one.
///
/// \returns `true` if the types mismatched but a cast is possible.
static bool typeCheck(Type *actual, Type *expected, const Metadata *md, 
                      String ctx = "") {
    if (actual->compare(expected))
        return false;

    if (actual->canCastTo(expected))
        return true;

    fatal((ctx.empty() ? "" : ctx + " ") + "type mismatch, got '" + 
          actual->getName() + "', expected '" + expected->getName() + "'", md);
}

Sema::Sema(const Options &opts, TranslationUnit *U) : m_Opts(opts), m_Unit(U) {
    m_Unit->accept(this);
}

void Sema::visit(TranslationUnit *U) {
    for (auto &D : U->getDecls()) D->accept(this);
}

void Sema::visit(FunctionDecl *decl) {
    m_Function = decl;
    for (auto &P : decl->getParams())
        P->accept(this);
    if (decl->getBody())
        decl->getBody()->accept(this);
    m_Function = nullptr;
}

void Sema::visit(VarDecl *decl) {
    if (decl->getInit())
        decl->getInit()->accept(this);

    if (typeCheck(
        decl->getInit()->getType(), 
        decl->getType(),
        &decl->getInit()->getMetadata(),
        "initializer"
    )) {
        decl->m_Init = new CastExpr(
            decl->getInit()->getMetadata(), 
            decl->getType(), 
            decl->getInit()
        );
    }
}

void Sema::visit(ParamDecl *decl) {

}

void Sema::visit(BreakStmt *stmt) {
    if (m_Loop == LoopKind::None)
        fatal("'break' statement outside loop'", &stmt->getMetadata());
}

void Sema::visit(ContinueStmt *stmt) {
    if (m_Loop == LoopKind::None)
        fatal("'continue' statement outside loop'", &stmt->getMetadata());
}

void Sema::visit(CompoundStmt *stmt) {
    for (auto &S : stmt->getStmts())
        S->accept(this);
}

void Sema::visit(DeclStmt *stmt) {
    stmt->getDecl()->accept(this);
}

void Sema::visit(ExprStmt *stmt) {
    stmt->getExpr()->accept(this);
}

void Sema::visit(IfStmt *stmt) {
    stmt->getCond()->accept(this);

    stmt->getThen()->accept(this);
    checkCompoundedDeclStmt(stmt->getThen());

    if (stmt->getElse()) {
        stmt->getElse()->accept(this);
        checkCompoundedDeclStmt(stmt->getElse());
    }
}

void Sema::visit(CaseStmt *stmt) {
    stmt->getPattern()->accept(this);
    checkCompoundedDeclStmt(stmt->getBody());
    stmt->getBody()->accept(this);
}

void Sema::visit(MatchStmt *stmt) {
    stmt->getPattern()->accept(this);
    for (auto &C : stmt->getCases()) {
        C->accept(this);

        if (typeCheck(
            C->getPattern()->getType(), 
            stmt->getPattern()->getType(), 
            &C->getMetadata(),
            "pattern"
        )) {
            C->m_Pattern = new CastExpr(
                C->getPattern()->getMetadata(), 
                stmt->getPattern()->getType(), 
                C->getPattern()
            );
        }
    }

    if (stmt->getDefault()) {
        stmt->getDefault()->accept(this);
        checkCompoundedDeclStmt(stmt->getDefault());
    }
}

void Sema::visit(RetStmt *stmt) {
    if (!m_Function)
        fatal("'ret' statement outside function", &stmt->getMetadata());

    if (stmt->getExpr()) {
        stmt->getExpr()->accept(this);

        if (typeCheck(stmt->getExpr()->getType(), 
            m_Function->getReturnType(), 
            &stmt->getMetadata(),
            "return"
        )) {
            stmt->m_Expr = new CastExpr(
                stmt->m_Expr->getMetadata(), 
                m_Function->getReturnType(), 
                stmt->m_Expr
            );  
        }
    } else if (!m_Function->getReturnType()->isVoid()) {
        fatal("function does not return 'void'", &stmt->getMetadata());
    }
}

void Sema::visit(UntilStmt *stmt) {
    LoopKind prev = m_Loop;
    m_Loop = LoopKind::Until;
    stmt->getCond()->accept(this);
    checkCompoundedDeclStmt(stmt->getBody());
    stmt->getBody()->accept(this);
    m_Loop = prev;
}

void Sema::visit(IntegerLiteral *expr) {}

void Sema::visit(FloatLiteral *expr) {}

void Sema::visit(CharLiteral *expr) {}

void Sema::visit(StringLiteral *expr) {}

void Sema::visit(NilLiteral *expr) {}

void Sema::visit(CastExpr *expr) {
    if (!expr->m_Expr->getType()->canCastTo(expr->getCast())) {
        fatal("cannot cast from '" + expr->m_Expr->getType()->getName() + 
              "' to '" + expr->getCast()->getName() + "'", &expr->getMetadata());
    }
}

void Sema::visit(RefExpr *expr) {}
