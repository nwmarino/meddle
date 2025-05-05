#include "nameres.h"
#include "decl.h"
#include "expr.h"
#include "stmt.h"
#include "type.h"
#include "unit.h"
#include "../core/logger.h"

using namespace meddle;

static Type *unwrapType(Type *T) {
    assert(T && "Cannot unwrap null types.");

    if (T->isQualified())
        return T;

    return static_cast<TypeResult *>(T)->getUnderlying();
}

NameResolution::NameResolution(const Options &opts, TranslationUnit *U) 
  : m_Opts(opts), m_Unit(U), m_Scope(U->getScope()) {
    m_Unit->getContext()->sanitate();
    U->accept(this);
}

void NameResolution::visit(TranslationUnit *U) {
    m_Phase = Phase::Shallow;
    for (auto &D : U->getDecls())
        D->accept(this);

    m_Phase = Phase::Recurse;
    for (auto &D : U->getDecls())
        D->accept(this);
}

void NameResolution::visit(FunctionDecl *decl) {
    if (m_Phase == Phase::Shallow) {
        auto *FT = static_cast<FunctionType *>(decl->getType());
        for (unsigned i = 0, n = FT->getNumParams(); i != n; ++i)
            decl->getParam(i)->m_Type = FT->getParamType(i);
    } else if (m_Phase == Phase::Recurse) {
        m_Scope = decl->getScope();
        decl->getBody()->accept(this);
        m_Scope = m_Scope->getParent();
    }
}

void NameResolution::visit(VarDecl *decl) {
    if (m_Phase == Phase::Shallow) {
        if (decl->m_Type)
            decl->m_Type = unwrapType(decl->getType());
    } else if (m_Phase == Phase::Recurse) {
        if (decl->getInit())
            decl->getInit()->accept(this);
        
        if (!decl->m_Type) {
            assert(decl->getInit() != nullptr && "Cannot infer type.");
            decl->m_Type = decl->getInit()->getType();
        }
    }
}

void NameResolution::visit(FieldDecl *decl) {
    if (m_Phase == Phase::Shallow) {
        if (decl->m_Type)
            decl->m_Type = unwrapType(decl->getType());
    } else if (m_Phase == Phase::Recurse) {
        if (decl->getInit())
            decl->getInit()->accept(this);
        
        if (!decl->m_Type) {
            assert(decl->getInit() != nullptr && "Cannot infer type.");
            decl->m_Type = decl->getInit()->getType();
        }
    }
}

void NameResolution::visit(StructDecl *decl) {
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
    if (expr->getBase()->getType()->isStruct()) {
        st = static_cast<StructType *>(expr->getBase()->getType());
    } else if (expr->getBase()->getType()->isPointer()) {
        auto *pt = static_cast<PointerType *>(expr->getBase()->getType());
        auto *pte = pt->getPointee();

        if (!pte->isStruct()) {
            fatal("access base is a pointer, but not a pointer to a struct", 
                &expr->getMetadata());
        }

        st = static_cast<StructType *>(pte);
    } else {
        fatal("expected struct type on base for '.' access", &expr->getMetadata());
    }

    StructDecl *sd = static_cast<StructDecl *>(st->getDecl());
    FieldDecl *fld = sd->getField(expr->getName());
    if (!fld) {
        fatal("field '" + expr->getName() + "' does not exist in struct '" + 
            sd->getName() + "'", &expr->getMetadata());
    }

    expr->m_Ref = fld;
    expr->m_Type = fld->getType();
}

void NameResolution::visit(ArrayExpr *expr) {
    for (auto &E : expr->getElements())
        E->accept(this);

    expr->m_Type = m_Unit->getContext()->getArrayType(
        expr->getElements()[0]->getType(), 
        expr->getElements().size()
    );
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

    expr->m_Ref = F;
    expr->m_Type = F->getReturnType();
        
    for (auto &A : expr->getArgs())
        A->accept(this);
}

void NameResolution::visit(CastExpr *expr) {
    expr->getExpr()->accept(this);
    expr->m_Type = unwrapType(expr->m_Type);
}

void NameResolution::visit(ParenExpr *expr) {
    expr->m_Expr->accept(this);
    expr->m_Type = expr->getExpr()->getType();
}

void NameResolution::visit(RefExpr *expr) {
    NamedDecl *ND = expr->getRef();
    if (!ND) {
        ND = m_Scope->lookup(expr->getName());
        if (!ND) {
            fatal("unresolved reference: " + expr->getName(), 
                &expr->getMetadata());
        }
    }

    expr->m_Ref = ND;
    
    if (auto *VD = dynamic_cast<VarDecl *>(ND)) {
        expr->m_Type = VD->getType();
    } else if (auto *EVD = dynamic_cast<EnumVariantDecl *>(ND)) {
        expr->m_Type = EVD->getType();
    } else if (auto *FD = dynamic_cast<FieldDecl *>(ND)) {
        expr->m_Type = FD->getType();
    } else {
        fatal("reference exists, but is not a variable: " + expr->getName(), 
            &expr->getMetadata());
    }
}

void NameResolution::visit(SizeofExpr *expr) {
    expr->m_Target = unwrapType(expr->getTarget());
}

void NameResolution::visit(SubscriptExpr *expr) {
    expr->getBase()->accept(this);
    expr->getIndex()->accept(this);
}

void NameResolution::visit(TypeSpecExpr *expr) {
    NamedDecl *ND = m_Scope->lookup(expr->getName());
    if (!ND) {
        fatal("unresolved type reference: " + expr->getName(), 
            &expr->getMetadata());
    }

    TypeDecl *TD = dynamic_cast<TypeDecl *>(ND);
    if (!TD) {
        fatal("reference exists, but is not a type: " + expr->getName(), 
            &expr->getMetadata());
    }

    if (auto *D = dynamic_cast<EnumDecl *>(TD)) {
        auto *R = dynamic_cast<RefExpr *>(expr->getExpr());
        if (!R) {
            fatal("expected reference expression after '::' operator", 
                &expr->getMetadata());
        }

        R->m_Ref = D->getVariant(R->getName());
        if (!R->m_Ref) {
            fatal("unresolved enum variant: " + R->getName(), 
                &expr->getMetadata());
        }
    } // else if struct, set m_Scope to struct type and pass over.

    expr->getExpr()->accept(this);
    expr->m_Type = expr->getExpr()->getType();
}

void NameResolution::visit(UnaryExpr *expr) {
    expr->getExpr()->accept(this);
    expr->m_Type = expr->getExpr()->getType();
}
