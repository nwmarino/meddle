#include "nameres.h"
#include "decl.h"
#include "expr.h"
#include "stmt.h"
#include "type.h"
#include "unit.h"
#include "../core/logger.h"

using namespace meddle;

NameResolution::NameResolution(const Options &opts, TranslationUnit *U) 
  : m_Opts(opts), m_Unit(U), m_Scope(U->getScope()) {
    U->accept(this);
}

void NameResolution::visit(TranslationUnit *U) {
    for (auto &D : U->getDecls())
        D->accept(this);
}

void NameResolution::visit(FunctionDecl *decl) {
    decl->setPUnit(m_Unit);

    m_Scope = decl->getScope();
    decl->getBody()->accept(this);
    m_Scope = m_Scope->getParent();
}

void NameResolution::visit(VarDecl *decl) {
    decl->setPUnit(m_Unit);

    if (decl->getInit())
        decl->getInit()->accept(this);
    
    if (!decl->m_Type) {
        assert(decl->getInit() != nullptr && "Cannot infer type.");
        decl->m_Type = decl->getInit()->getType();
    }
}

void NameResolution::visit(FieldDecl *decl) {
    decl->setPUnit(m_Unit);

    if (decl->getInit())
        decl->getInit()->accept(this);
    
    if (!decl->m_Type) {
        assert(decl->getInit() != nullptr && "Cannot infer type.");
        decl->m_Type = decl->getInit()->getType();
    }
}

void NameResolution::visit(StructDecl *decl) {
    decl->setPUnit(m_Unit);
    m_Scope = decl->getScope();

    for (auto &F : decl->getFields())
        F->accept(this);

    for (auto &F : decl->getFunctions())
        F->accept(this);

    m_Scope = m_Scope->getParent();
}

void NameResolution::visit(CompoundStmt *stmt) {
    m_Scope = stmt->getScope();
    
    for (auto &S : stmt->getStmts())
        S->accept(this);

    m_Scope = m_Scope->getParent();
}

void NameResolution::visit(DeclStmt *stmt) {
    stmt->getDecl()->accept(this);
}

void NameResolution::visit(ExprStmt *stmt) {
    stmt->getExpr()->accept(this);
}

void NameResolution::visit(IfStmt *stmt) {
    stmt->getCond()->accept(this);
    stmt->getThen()->accept(this);
    if (stmt->getElse())
        stmt->getElse()->accept(this);
}

void NameResolution::visit(CaseStmt *stmt) {
    stmt->getPattern()->accept(this);
    stmt->getBody()->accept(this);
}

void NameResolution::visit(MatchStmt *stmt) {
    stmt->getPattern()->accept(this);
    for (auto &C : stmt->getCases())
        C->accept(this);

    if (stmt->getDefault())
        stmt->getDefault()->accept(this);
}

void NameResolution::visit(RetStmt *stmt) {
    if (stmt->getExpr())
        stmt->getExpr()->accept(this);
}

void NameResolution::visit(UntilStmt *stmt) {
    stmt->getCond()->accept(this);
    stmt->getBody()->accept(this);
}

void NameResolution::visit(AccessExpr *expr) {
    expr->getBase()->accept(this);

    StructType *st = nullptr;
    if (expr->getBase()->getType()->isStruct())
        st = expr->getBase()->getType()->asStruct();
    else if (expr->getBase()->getType()->isPointer()) {
        auto *pointee = expr->getBase()->getType()->asPointer()->getPointee();
        if (!pointee->isStruct())
            fatal("access base is a pointer, but not a pointer to a struct", 
                &expr->getMetadata());

        st = pointee->asStruct();
    } else {
        fatal("expected struct type on base for '.' access", &expr->getMetadata());
    }

    StructDecl *structure = static_cast<StructDecl *>(st->getDecl());
    FieldDecl *field = structure->getField(expr->getName());
    if (!field)
        fatal("field '" + expr->getName() + "' does not exist in struct '" + 
            structure->getName() + "'", &expr->getMetadata());

    expr->m_Ref = field;
    expr->m_Type = field->getType();
}

void NameResolution::visit(ArrayExpr *expr) {
    for (auto &E : expr->getElements())
        E->accept(this);

    expr->m_Type = ArrayType::get(m_Unit->getContext(), 
        expr->getElements()[0]->getType(), expr->getElements().size());
}

void NameResolution::visit(BinaryExpr *expr) {
    expr->getLHS()->accept(this);
    expr->getRHS()->accept(this);
}

void NameResolution::visit(CallExpr *expr) {
    NamedDecl *D = m_Scope->lookup(expr->getName());
    if (!D)
        fatal("unresolved function reference: " + expr->getName(), 
            &expr->getMetadata());

    FunctionDecl *F = dynamic_cast<FunctionDecl *>(D);
    if (!F)
        fatal("reference exists, but is not a function: " + expr->getName(), 
            &expr->getMetadata());
    
    if (F->isTemplate()) {
        if (expr->m_TypeArgs.empty()) {
            fatal("callee " + F->getName() + 
                " is a template, but no type arguments were provided in call", 
                &expr->getMetadata());
        }

        if (expr->getNumTypeArgs() != F->getNumTemplateParams()) {
            fatal("callee " + F->getName() + 
                " is a template, but the number of type arguments does not match", 
                &expr->getMetadata());
        }

        expr->m_Ref = F->fetchSpecialization(expr->m_TypeArgs);
    } else {
        expr->m_Ref = F;
    }

    expr->m_Type = static_cast<FunctionDecl *>(expr->getRef())->getReturnType();

    for (auto &A : expr->getArgs())
        A->accept(this);
}

void NameResolution::visit(CastExpr *expr) {
    expr->getExpr()->accept(this);
}

void NameResolution::visit(FieldInitExpr *expr) {
    expr->getExpr()->accept(this);
    expr->m_Type = expr->getExpr()->getType();
}

void NameResolution::visit(InitExpr *expr) {
    for (auto &F : expr->getFields())
        F->accept(this);

    if (!expr->getType()->isStruct())
        fatal("expected struct type for aggregate initializer", 
            &expr->getMetadata());

    StructDecl *_struct = expr->getType()->asStruct()->getDecl();
    for (auto &fldExpr : expr->getFields()) {
        FieldDecl *fldDecl = _struct->getField(fldExpr->getName());
        if (!fldDecl)
            fatal("field '" + fldExpr->getName() + "' does not exist in struct '" + 
                _struct->getName() + "'", &expr->getMetadata());

        fldExpr->m_Ref = fldDecl;
        fldExpr->accept(this);
    }
}

void NameResolution::visit(MethodCallExpr *expr) {
    expr->getBase()->accept(this);

    StructType *ty = nullptr;
    if (expr->getBase()->getType()->isStruct()) {
        ty = expr->getBase()->getType()->asStruct();
    } else if (expr->getBase()->getType()->isPointer()) {
        auto *pointee = expr->getBase()->getType()->asPointer()->getPointee();
        if (!pointee->isStruct())
            fatal("method call base is a pointer, but not a pointer to a struct", 
                &expr->getMetadata());

        ty = pointee->asStruct();
    } else {
        fatal("expected struct type on base for method call", &expr->getMetadata());
    }

    StructDecl *_struct = ty->getDecl();
    FunctionDecl *method = _struct->getFunction(expr->getName());
    if (!method)
        fatal("method '" + expr->getName() + "' does not exist in struct '" + 
            _struct->getName() + "'", &expr->getMetadata());

    expr->m_Ref = method;
    expr->m_Type = method->getReturnType();

    for (auto &arg : expr->getArgs())
        arg->accept(this);
}

void NameResolution::visit(ParenExpr *expr) {
    expr->m_Expr->accept(this);
    expr->m_Type = expr->getExpr()->getType();
}

void NameResolution::visit(RefExpr *expr) {
    NamedDecl *named = expr->getRef();
    if (!named) {
        named = m_Scope->lookup(expr->getName());
        if (!named)
            fatal("unresolved reference: " + expr->getName(), 
                &expr->getMetadata());
    }
    
    if (auto *var = dynamic_cast<VarDecl *>(named))
        expr->m_Type = var->getType();
    else if (auto *enumvar = dynamic_cast<EnumVariantDecl *>(named))
        expr->m_Type = enumvar->getType();
    else if (auto *field = dynamic_cast<FieldDecl *>(named))
        expr->m_Type = field->getType();
    else
        fatal("reference exists, but is not a variable: " + expr->getName(), 
            &expr->getMetadata());

    expr->m_Ref = named;
}

void NameResolution::visit(SubscriptExpr *expr) {
    expr->getBase()->accept(this);
    expr->getIndex()->accept(this);
}

void NameResolution::visit(TypeSpecExpr *expr) {
    Type *T = m_Unit->getContext()->resolveType(expr->getName(), m_Scope, expr->getMetadata());
    if (!T)
        fatal("unresolved type reference: " + expr->getName(), 
            &expr->getMetadata());

    if (!T->isStruct())
        fatal("expected struct type for type specifier", 
            &expr->getMetadata());
    
    StructDecl *_struct = T->asStruct()->getDecl();
    expr->m_Ref = _struct; 

    if (!dynamic_cast<CallExpr *>(expr->getExpr()))
        fatal("expected call expression after '::' operator on structure", 
            &expr->getMetadata());

    Scope *old_scope = m_Scope;
    m_Scope = _struct->getScope();
    expr->getExpr()->accept(this);
    m_Scope = old_scope;
    
    expr->m_Type = expr->getExpr()->getType();
}

void NameResolution::visit(UnitSpecExpr *expr) {
    expr->setUnit(expr->getUse()->getUnit());
    assert(expr->getUnit() && "Use unresolved.");

    Scope *old_scope = m_Scope;
    m_Scope = expr->getUnit()->getScope();

    expr->getExpr()->accept(this);

    if (!expr->getExpr()->getRef()->hasPublicRune())
        fatal("specified declaration exists, but not marked public: " + 
            expr->getExpr()->getName(), &expr->getMetadata());

    expr->m_Type = expr->getExpr()->getType();
    m_Scope = old_scope;
}

void NameResolution::visit(UnaryExpr *expr) {
    expr->getExpr()->accept(this);
    expr->m_Type = expr->getExpr()->getType();
}

void NameResolution::visit(RuneSyscallExpr *expr) {
    for (auto &arg : expr->getArgs())
        arg->accept(this);
}
