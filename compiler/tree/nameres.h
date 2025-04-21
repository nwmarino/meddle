#ifndef MEDDLE_NAME_RESOLUTION_H
#define MEDDLE_NAME_RESOLUTION_H

#include "stmt.h"
#include "unit.h"
#include "visitor.h"

namespace meddle {

class NameResolution : public Visitor<NameResolution> {
public:
    void visit(TranslationUnit *U);

    void visit(FunctionDecl *F);
    void visit(VarDecl *V);
    void visit(ParamDecl *P);

    void visit(RetStmt *R);

    void visit(IntegerLiteral *I);
};

} // namespace meddle

#endif // MEDDLE_NAME_RESOLUTION_H
