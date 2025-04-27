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
    else if (match(LiteralKind::String))
        return parse_str();

    return nullptr;
}

Expr *Parser::parse_ident() {
    if (match_keyword("cast"))
        return parse_cast();

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

StringLiteral *Parser::parse_str() {
    StringLiteral *S = new StringLiteral(
        m_Current->md,
        m_Context->getArrayType(m_Context->getCharType(), m_Current->value.size() + 1),
        m_Current->value
    );
    next();
    return S;
}

CastExpr *Parser::parse_cast() {
    Metadata md = m_Current->md;
    Type *T = nullptr;
    Expr *E = nullptr;
    next(); // 'cast'

    if (!match(TokenKind::Left))
        fatal("expected '<' after 'cast' keyword", &m_Current->md);
    next(); // '<'

    T = parse_type(true);
    
    if (!match(TokenKind::Right))
        fatal("expected '>' after 'cast' type", &m_Current->md);
    next(); // '>'

    E = parse_expr();
    if (!E)
        fatal("expected cast sub-expression", &m_Current->md);

    return new CastExpr(md, T, E);
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
