#ifndef MEDDLE_CCODEGEN_H
#define MEDDLE_CCODEGEN_H

#include "stmt.h"
#include "type.h"
#include "unit.h"
#include "visitor.h"
#include "../core/options.h"

#include <fstream>

namespace meddle {

class CCGN : public Visitor {
    enum class Phase {
        Declare,
        Define,
    } phase;

    Options m_Opts;
    TranslationUnit *m_Unit;
    std::ofstream m_Cout;
    std::ofstream m_Hout;

    void emitCSeg(const String &seg);
    void emitCLn(const String &ln);
    void emitHSeg(const String &seg);
    void emitHLn(const String &ln);

    String translateType(Type *T);

public:
    CCGN(const Options &opts, TranslationUnit *U);

    void visit(TranslationUnit *unit) override;

    void visit(FunctionDecl *decl) override;
    void visit(VarDecl *decl) override;
    void visit(ParamDecl *decl) override;

    void visit(CompoundStmt *stmt) override;
    void visit(DeclStmt *stmt) override;
    void visit(IfStmt *stmt) override;
    void visit(RetStmt *stmt) override; 

    void visit(IntegerLiteral *expr) override;
    void visit(FloatLiteral *expr) override;
    void visit(CharLiteral *expr) override;
    void visit(RefExpr *expr) override;
};

} // namespace meddle

#endif // MEDDLE_CCODEGEN_H
