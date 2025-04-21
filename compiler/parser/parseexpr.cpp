#include "parser.h"
#include "../core/logger.h"

#include <string>

using namespace meddle;

Expr *Parser::parseExpr() {
    return parsePrimaryExpr();
}

Expr *Parser::parsePrimaryExpr() {
    if (match(LiteralKind::Integer))
        return parseIntLiteral();

    return nullptr;
}

IntegerLiteral *Parser::parseIntLiteral() {
    IntegerLiteral *I = new IntegerLiteral(
        m_Current->md, 
        m_Context->getI64Type(), 
        std::stoll(m_Current->value)
    );
    next();
    return I;
}
