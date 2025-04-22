#ifndef MEDDLE_VISITOR_H
#define MEDDLE_VISITOR_H

namespace meddle {

class TranslationUnit;

class FunctionDecl;
class VarDecl;
class ParamDecl;

class CompoundStmt;
class DeclStmt;
class RetStmt;

class Expr;
class IntegerLiteral;
class RefExpr;

class Visitor {
public:
    virtual void visit(TranslationUnit *unit) = 0;

    virtual void visit(FunctionDecl *decl) = 0;
    virtual void visit(VarDecl *decl) = 0;
    virtual void visit(ParamDecl *decl) = 0;

    virtual void visit(CompoundStmt *stmt) = 0;
    virtual void visit(DeclStmt *stmt) = 0;
    virtual void visit(RetStmt *stmt) = 0;

    virtual void visit(IntegerLiteral *expr) = 0;
    virtual void visit(RefExpr *expr) = 0;
};

} // namespace meddle

#endif // MEDDLE_VISITOR_H
