#ifndef MEDDLE_VISITOR_H
#define MEDDLE_VISITOR_H

namespace meddle {

class TranslationUnit;

class FunctionDecl;
class VarDecl;
class ParamDecl;

class BreakStmt;
class ContinueStmt;
class CompoundStmt;
class DeclStmt;
class ExprStmt;
class IfStmt;
class CaseStmt;
class MatchStmt;
class RetStmt;
class UntilStmt;

class Expr;
class IntegerLiteral;
class FloatLiteral;
class CharLiteral;
class RefExpr;

class Visitor {
public:
    virtual void visit(TranslationUnit *unit) = 0;

    virtual void visit(FunctionDecl *decl) = 0;
    virtual void visit(VarDecl *decl) = 0;
    virtual void visit(ParamDecl *decl) = 0;

    virtual void visit(BreakStmt *stmt) = 0;
    virtual void visit(ContinueStmt *stmt) = 0;
    virtual void visit(CompoundStmt *stmt) = 0;
    virtual void visit(DeclStmt *stmt) = 0;
    virtual void visit(ExprStmt *stmt) = 0;
    virtual void visit(IfStmt *stmt) = 0;
    virtual void visit(CaseStmt *stmt) = 0;
    virtual void visit(MatchStmt *stmt) = 0;
    virtual void visit(RetStmt *stmt) = 0;
    virtual void visit(UntilStmt *stmt) = 0;

    virtual void visit(IntegerLiteral *expr) = 0;
    virtual void visit(FloatLiteral *expr) = 0;
    virtual void visit(CharLiteral *expr) = 0;
    virtual void visit(RefExpr *expr) = 0;
};

} // namespace meddle

#endif // MEDDLE_VISITOR_H
