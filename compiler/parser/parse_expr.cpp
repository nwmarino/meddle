#include "parser.h"
#include "../core/logger.h"

#include <string>

using namespace meddle;

Expr *Parser::parse_expr() {
    return parse_primary();
}

Expr *Parser::parse_primary() {
    if (match(TokenKind::Identifier))
        return parse_ident();
    else if (match(LiteralKind::Integer))
        return parse_int();
    else if (match(LiteralKind::Float))
        return parse_fp();
    else if (match(LiteralKind::Character))
        return parse_char();

    return nullptr;
}

Expr *Parser::parse_ident() {
    return parse_ref();
}

IntegerLiteral *Parser::parse_int() {
    IntegerLiteral *I = new IntegerLiteral(
        m_Current->md, 
        m_Context->getI64Type(), 
        std::stoll(m_Current->value)
    );
    next();
    return I;
}

FloatLiteral *Parser::parse_fp() {
    FloatLiteral *F = new FloatLiteral(
        m_Current->md,
        m_Context->getF64Type(),
        std::stod(m_Current->value)
    );
    next();
    return F;
}

CharLiteral *Parser::parse_char() {
    CharLiteral *C = new CharLiteral(
        m_Current->md,
        m_Context->getCharType(),
        m_Current->value[0]
    );
    next();
    return C;
}

RefExpr *Parser::parse_ref() {
    Metadata md = m_Current->md;
    String name = m_Current->value;

    NamedDecl *D = m_Scope->lookup(name);
    if (!D)
        fatal("unresolved reference: " + name, &md);

    next();
    return new RefExpr(md, nullptr, name, D);
}
