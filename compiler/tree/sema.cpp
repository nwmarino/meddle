#include "sema.h"
#include "decl.h"
#include "expr.h"
#include "stmt.h"
#include "type.h"
#include "unit.h"
#include "../core/logger.h"
#include "visitor.h"

using namespace meddle;

enum class TypeCheckMode {
    Exact,
    AllowImplicit,
    Loose
};

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
                      String ctx = "", 
                      TypeCheckMode mode = TypeCheckMode::AllowImplicit) {
    if (actual->compare(expected))
        return false;

    switch (mode) {
    case TypeCheckMode::Exact:
        break;

    case TypeCheckMode::AllowImplicit:
        if (actual->canImplCastTo(expected))
            return true;

        break;

    case TypeCheckMode::Loose:
        if (actual->canImplCastTo(expected))
            return true;
        else if (actual->isPointer() && expected->isPointer())
            return false;
        else if (actual->isInt() && expected->isPointer())
            return false;
        else if (actual->isPointer() && expected->isInt())
            return false;

        break;
    }

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

    if (decl->isMethod()) {
        // Semantically validate method functions.
        //
        // The first parameter of a method should be named `self`, and should
        // be typed with a pointer to the type of the parent structure.

        if (decl->getNumParams() < 1) 
            fatal("method must have at least one parameter", &decl->getMetadata());

        ParamDecl *F = decl->getParam(0);
        if (F->getName() != "self") {
            fatal("first parameter of method must be named 'self'", 
                  &F->getMetadata());
        }

        if (!F->getType()->isPointer()) {
            fatal("first parameter of method must be a pointer type", 
                &F->getMetadata());
        }
      
        PointerType *PT = dynamic_cast<PointerType *>(F->getType());
        assert(PT);
        if (!PT->getPointee()->compare(decl->getParent()->getDefinedType())) {
            fatal("first parameter of method must be a pointer to the parent struct type", 
                &F->getMetadata());
        }
    }

    if (decl->getBody())
        decl->getBody()->accept(this);
    
    m_Function = nullptr;
}

void Sema::visit(VarDecl *decl) {
    if (decl->getInit()) {
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
}

void Sema::visit(FieldDecl *decl) {
    if (decl->hasInit())
        decl->getInit()->accept(this);
}

void Sema::visit(StructDecl *decl) {
    for (auto &F : decl->getFields())
        F->accept(this);

    for (auto &F : decl->getFunctions())
        F->accept(this);
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

void Sema::visit(ArrayExpr *expr) {
    ArrayType *AT = static_cast<ArrayType *>(expr->getType());
    for (unsigned i = 0, n = expr->getElements().size(); i != n; ++i) {
        Expr *Elem = expr->getElements().at(i);
        Elem->accept(this);

        if (typeCheck(
            Elem->getType(), 
            AT->getElement(), 
            &Elem->getMetadata(), 
            "array element"
        )) {
            expr->m_Elements[i] = new CastExpr(
                Elem->getMetadata(), 
                expr->getType(), 
                Elem
            );
        }
    }
}

void Sema::visit(BinaryExpr *expr) {
    expr->getLHS()->accept(this);
    expr->getRHS()->accept(this);

    Type *LTy = expr->getLHS()->getType();
    Type *RTy = expr->getRHS()->getType();

    TypeCheckMode mode = TypeCheckMode::AllowImplicit;
    if (BinaryExpr::supportsPtrArith(expr->getKind()))
        mode = TypeCheckMode::Loose;

    if (typeCheck(RTy, LTy, &expr->getMetadata(), "operator", mode)) {
        expr->m_RHS = new CastExpr(
            expr->getRHS()->getMetadata(), 
            LTy, 
            expr->getRHS()
        );
    }

    if (expr->isComparison()) {
        expr->m_Type = m_Unit->getContext()->getBoolType();
        return;
    }

    expr->m_Type = LTy;
    if (!expr->isAssignment())
        return;

    if (!expr->getLHS()->isLValue())
        fatal("cannot assign to non-lvalue", &expr->getMetadata());

    VarDecl *LVal = nullptr;
    if (auto *E = dynamic_cast<RefExpr *>(expr->getLHS())) {
        LVal = dynamic_cast<VarDecl *>(E->getRef());
    } else if (auto *E = dynamic_cast<UnaryExpr *>(expr->getLHS())) {
        if (E->getKind() == UnaryExpr::Kind::Dereference) {
            // Unary expressions are only lvalues for dereferences `*`.
            RefExpr *RE = dynamic_cast<RefExpr *>(E->getExpr());
            assert(RE && "Non-referencing deference expression.");
            LVal = dynamic_cast<VarDecl *>(RE->getRef());
        }
    } else if (auto *E = dynamic_cast<SubscriptExpr *>(expr->getLHS())) {
        RefExpr *RE = dynamic_cast<RefExpr *>(E->getBase());
        assert(RE && "Non-referencing subscript expression.");
        LVal = dynamic_cast<VarDecl *>(RE->getRef());
    }

    assert(LVal && "LHS must be a reference.");
    if (!LVal->isMutable())
        fatal("cannot reassign immutable variable", &expr->getMetadata());
}

void Sema::visit(CallExpr *expr) {
    FunctionDecl *callee = expr->getCallee();

    // Check that the argument count matches the functions parameter count.
    if (expr->getNumArgs() != callee->getNumParams()) {
        fatal("function call argument count mismatch, got " + 
            std::to_string(expr->getNumArgs()) + ", expected" + 
            std::to_string(callee->getNumParams()), &expr->getMetadata());
    }

    // Pass over each argument and type check it with the corresponding param.
    for (unsigned i = 0; i != expr->getNumArgs(); ++i) {
        Expr *arg = expr->getArg(i);
        arg->accept(this);

        ParamDecl *param = callee->getParam(i);

        if (typeCheck(arg->getType(), param->getType(), &arg->getMetadata(), "argument")) {
            expr->m_Args[i] = new CastExpr(
                arg->getMetadata(), 
                param->getType(), 
                arg
            );
        }
    }
}

void Sema::visit(CastExpr *expr) {
    if (!expr->m_Expr->getType()->canCastTo(expr->getCast())) {
        fatal("cannot cast from '" + expr->m_Expr->getType()->getName() + 
              "' to '" + expr->getCast()->getName() + "'", &expr->getMetadata());
    }
}

void Sema::visit(FieldInitExpr *expr) {
    expr->getExpr()->accept(this);
    expr->m_Type = expr->getExpr()->getType();

    FieldDecl *fld = static_cast<FieldDecl *>(expr->getRef());

    if (typeCheck(expr->getType(), fld->getType(), &expr->getMetadata(), "field initializer")) {
        expr->m_Expr = new CastExpr(
            expr->getExpr()->getMetadata(), 
            fld->getType(), 
            expr->getExpr()
        );
    }
}

void Sema::visit(InitExpr *expr) {
    for (auto &F : expr->getFields())
        F->accept(this);
}

void Sema::visit(MethodCallExpr *expr) {
    FunctionDecl *callee = expr->getCallee();

    if (!callee->isMethod()) {
        fatal("method call on non-method function: " + callee->getName(), 
            &expr->getMetadata());
    }

    // Check that the argument count matches the methods parameter count.
    //
    // We do +1 here because the `self` parameter is implicitly filled in from
    // the base expression.
    if (expr->getNumArgs() + 1 != callee->getNumParams()) {
        fatal("method call argument count mismatch, got " + 
            std::to_string(expr->getNumArgs() + 1) + ", expected" + 
            std::to_string(callee->getNumParams()), &expr->getMetadata());
    }

    // Pass over each argument and type check it with the corresponding param.
    for (unsigned i = 0; i != expr->getNumArgs(); ++i) {
        Expr *arg = expr->getArg(i);
        arg->accept(this);

        // Params are offset by 1 because, again, the first parameter is `self`.
        ParamDecl *param = callee->getParam(i + 1);

        if (typeCheck(arg->getType(), param->getType(), &arg->getMetadata(), "argument")) {
            expr->m_Args[i] = new CastExpr(
                arg->getMetadata(), 
                param->getType(), 
                arg
            );
        }
    }
}

void Sema::visit(ParenExpr *expr) {
    expr->m_Expr->accept(this);
}

void Sema::visit(SubscriptExpr *expr) {
    expr->getBase()->accept(this);
    expr->getIndex()->accept(this);

    if (expr->getBase()->isAggregateInit())
        fatal("cannot subscript into an array initializer", 
              &expr->getMetadata());

    Type *baseTy = expr->getBase()->getType();
    if (baseTy->isArray()) {
        expr->m_Type = static_cast<ArrayType *>(baseTy)->getElement();
    } else if (baseTy->isPointer()) {
        expr->m_Type = static_cast<PointerType *>(baseTy)->getPointee();
    } else {
        fatal("subscript [] base type must be an array or pointer, got '" + 
            baseTy->getName() + "'", &expr->getMetadata());
    } 

    if (!expr->getIndex()->getType()->isInt())
        fatal("subscript [] index type must be an integer, got '" +
              expr->getIndex()->getType()->getName() + "'", &expr->getMetadata());
}

void Sema::visit(TypeSpecExpr *expr) {
    expr->getExpr()->accept(this);
}

void Sema::visit(UnaryExpr *expr) {
    expr->getExpr()->accept(this);

    switch (expr->getKind()) {
    case UnaryExpr::Kind::Unknown:
        assert(false && "Unknown unary operator.");

    case UnaryExpr::Kind::Logic_Not:
        expr->m_Type = m_Unit->getContext()->getBoolType();
        break;

    case UnaryExpr::Kind::Bitwise_Not:
    case UnaryExpr::Kind::Negate:
        expr->m_Type = expr->getExpr()->getType();
        break;

    case UnaryExpr::Kind::Address_Of:
        if (!expr->getExpr()->isLValue())
            fatal("cannot apply '&' to non-lvalue", &expr->getMetadata());

        expr->m_Type = m_Unit->getContext()->getPointerType(
            expr->getExpr()->getType());
        
        break;

    case UnaryExpr::Kind::Dereference:
        if (!expr->getExpr()->getType()->isPointer())
            fatal("cannot apply '*' operator to non-pointer type", 
                  &expr->getMetadata());

        expr->m_Type = static_cast<PointerType *>(
            expr->getExpr()->getType())->getPointee();

        break;

    case UnaryExpr::Kind::Increment:
    case UnaryExpr::Kind::Decrement:
        if (!expr->getExpr()->isLValue())
            fatal("cannot apply unary operator to non-lvalue", 
                  &expr->getMetadata());

        VarDecl *LVal = nullptr;
        if (auto *E = dynamic_cast<RefExpr *>(expr->getExpr())) {
            LVal = dynamic_cast<VarDecl *>(E->getRef());
        }
              
        assert(LVal && "LHS must be a reference.");
        if (!LVal->isMutable())
            fatal("cannot reassign immutable variable", &expr->getMetadata());
        break;
    }
}
