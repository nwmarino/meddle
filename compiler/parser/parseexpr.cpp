#include "parser.h"
#include "../core/logger.h"

#include <string>

using namespace meddle;

Expr *Parser::parseExpr() {
    return parsePrimaryExpr();
}

Expr *Parser::parsePrimaryExpr() {
    if (match(TokenKind::Identifier))
        return parseIdentExpr();
    else if (match(LiteralKind::Integer))
        return parseInteger();
    else if (match(LiteralKind::Float))
        return parseFloat();
    else if (match(LiteralKind::Character))
        return parseChar();

    return nullptr;
}

Expr *Parser::parseIdentExpr() {
    return parseRef();
}

IntegerLiteral *Parser::parseInteger() {
    IntegerLiteral *I = new IntegerLiteral(
        m_Current->md, 
        m_Context->getI64Type(), 
        std::stoll(m_Current->value)
    );
    next();
    return I;
}

FloatLiteral *Parser::parseFloat() {
    FloatLiteral *F = new FloatLiteral(
        m_Current->md,
        m_Context->getF64Type(),
        std::stod(m_Current->value)
    );
    next();
    return F;
}

CharLiteral *Parser::parseChar() {
    CharLiteral *C = new CharLiteral(
        m_Current->md,
        m_Context->getCharType(),
        m_Current->value[0]
    );
    next();
    return C;
}

RefExpr *Parser::parseRef() {
    Metadata md = m_Current->md;
    String name = m_Current->value;

    NamedDecl *D = m_Scope->lookup(name);
    if (!D)
        fatal("unresolved reference: " + name, &md);

    next();
    return new RefExpr(md, nullptr, name, D);
}
