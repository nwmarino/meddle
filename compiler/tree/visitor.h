#ifndef MEDDLE_VISITOR_H
#define MEDDLE_VISITOR_H

namespace meddle {

class TranslationUnit;

class FunctionDecl;
class VarDecl;
class ParamDecl;
class EnumVariantDecl;
class EnumDecl;

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

class BoolLiteral;
class IntegerLiteral;
class FloatLiteral;
class CharLiteral;
class StringLiteral;
class NilLiteral;

class ArrayExpr;
class BinaryExpr;
class CastExpr;
class ParenExpr;
class RefExpr;
class CallExpr;
class SizeofExpr;
class SubscriptExpr;
class TypeSpecExpr;
class UnaryExpr;

class Visitor {
public:
    virtual void visit(TranslationUnit *unit) {};

    virtual void visit(FunctionDecl *decl) {}
    virtual void visit(VarDecl *decl) {}
    virtual void visit(ParamDecl *decl) {}
    virtual void visit(EnumVariantDecl *decl) {}
    virtual void visit(EnumDecl *decl) {}

    virtual void visit(BreakStmt *stmt) {}
    virtual void visit(ContinueStmt *stmt) {}
    virtual void visit(CompoundStmt *stmt) {}
    virtual void visit(DeclStmt *stmt) {}
    virtual void visit(ExprStmt *stmt) {}
    virtual void visit(IfStmt *stmt) {}
    virtual void visit(CaseStmt *stmt) {}
    virtual void visit(MatchStmt *stmt) {}
    virtual void visit(RetStmt *stmt) {}
    virtual void visit(UntilStmt *stmt) {}

    virtual void visit(BoolLiteral *expr) {}
    virtual void visit(IntegerLiteral *expr) {}
    virtual void visit(FloatLiteral *expr) {}
    virtual void visit(CharLiteral *expr) {}
    virtual void visit(StringLiteral *expr) {}
    virtual void visit(NilLiteral *expr) {}

    virtual void visit(ArrayExpr *expr) {}
    virtual void visit(BinaryExpr *expr) {}
    virtual void visit(CallExpr *expr) {}
    virtual void visit(CastExpr *expr) {}
    virtual void visit(ParenExpr *expr) {}
    virtual void visit(RefExpr *expr) {}
    virtual void visit(SizeofExpr *expr) {}
    virtual void visit(SubscriptExpr *expr) {}
    virtual void visit(TypeSpecExpr *expr) {}
    virtual void visit(UnaryExpr *expr) {}
};

} // namespace meddle

#endif // MEDDLE_VISITOR_H
