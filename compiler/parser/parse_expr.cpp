#include "parser.h"
#include "../core/logger.h"

#include <string>

using namespace meddle;

Expr *Parser::parse_expr() {
    Expr *base = parse_unary_prefix();
    if (!base)
        fatal("expected expression", &m_Current->md);

    return parse_binary(base, 0);
}

Expr *Parser::parse_primary() {
    if (match(TokenKind::Identifier))
        return parse_ident();
    else if (match(TokenKind::SetBrack))
        return parse_array();
    else if (match(TokenKind::SetParen))
        return parse_paren();
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
    else if (match_keyword("nil"))
        return parse_nil();
    else if (match_keyword("true") || match_keyword("false"))
        return parse_bool();
    else if (match_keyword("sizeof"))
        return parse_sizeof();

    next(); // identifier

    if (match(TokenKind::SetParen))
        return parse_call();
    else if (match(TokenKind::Path))
        return parse_spec();

    backtrack(1);
    return parse_ref();
}

BoolLiteral *Parser::parse_bool() {
    BoolLiteral *B = new BoolLiteral(
        m_Current->md,
        m_Context->getBoolType(),
        m_Current->value == "true"
    );
    next();
    return B;
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

NilLiteral *Parser::parse_nil() {
    NilLiteral *N = new NilLiteral(
        m_Current->md,
        m_Context->getPointerType(m_Context->getVoidType())
    );
    next();
    return N;
}

ArrayExpr *Parser::parse_array() {
    Metadata md = m_Current->md;
    std::vector<Expr *> Elements;
    next(); // '['

    while (!match(TokenKind::EndBrack)) {
        Expr *Elem = parse_expr();
        if (!Elem)
            fatal("expected array element", &m_Current->md);

        Elements.push_back(Elem);

        if (match(TokenKind::EndBrack))
            break;

        if (!match(TokenKind::Comma))
            fatal("expected ',' or ']' in array literal", &m_Current->md);
        next(); // ','
    }

    if (Elements.empty())
        fatal("array enclosed by [, ] cannot be empty", &m_Current->md);

    next(); // ']'
    return new ArrayExpr(md, nullptr, Elements);
}

Expr *Parser::parse_binary(Expr *B, int precedence) {
    while (1) {
        int tokPrec = get_bin_precedence();
        if (tokPrec < precedence)
            break;

        BinaryExpr::Kind op = get_bin_operator();
        if (op == BinaryExpr::Kind::Unknown)
            break;

        next(); // Eat the operator.
        Expr *rhs = parse_unary_prefix();
        if (!rhs)
            fatal("expected right hand side expression", &m_Current->md);

        int nextPrec = get_bin_precedence();
        if (tokPrec < nextPrec) {
            rhs = parse_binary(rhs, precedence + 1);
            if (!rhs)
                fatal("expected binary expression", &m_Current->md);
        }

        B = new BinaryExpr(
            m_Current->md, 
            B->getType(), 
            op, 
            B, 
            rhs
        );
    }

    return B;
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

ParenExpr *Parser::parse_paren() {
    Metadata md = m_Current->md;

    next(); // '('

    Expr *E = parse_expr();
    if (!E)
        fatal("expected expression after '('", &m_Current->md);

    if (!match(TokenKind::EndParen))
        fatal("expected ')' after expression", &m_Current->md);

    next(); // ')'
    return new ParenExpr(md, E);
}

RefExpr *Parser::parse_ref() {
    Metadata md = m_Current->md;
    String name = m_Current->value;

    NamedDecl *D = m_Scope->lookup(name);
    if (!D && !m_AllowUnresolved)
        fatal("unresolved reference: " + name, &md);

    next(); // identifier
    return new RefExpr(md, nullptr, name, D);
}

CallExpr *Parser::parse_call() {
    if (match(TokenKind::SetParen))
        backtrack(1);

    Metadata md = m_Current->md;
    std::vector<Expr *> Args;
    String callee;

    if (!match(TokenKind::Identifier))
        fatal("expected callee identifier", &md);

    callee = m_Current->value;
    next(); // identifier

    if (!match(TokenKind::SetParen))
        fatal("expected '(' after function name", &m_Current->md);
    next(); // '('

    while (!match(TokenKind::EndParen)) {
        Expr *Arg = parse_expr();
        if (!Arg)
            fatal("expected function argument", &m_Current->md);

        Args.push_back(Arg);

        if (match(TokenKind::EndParen))
            break;

        if (!match(TokenKind::Comma))
            fatal("expected ',' or ')' in function call", &m_Current->md);
        next(); // ','
    }

    next(); // ')'
    return new CallExpr(md, nullptr, callee, nullptr, Args);
}

SizeofExpr *Parser::parse_sizeof() {
    Metadata md = m_Current->md;
    next(); // 'sizeof'

    if (!match(TokenKind::Left))
        fatal("expected '<' after 'sizeof' keyword", &m_Current->md);
    next(); // '<'

    Type *T = parse_type(true);

    if (!match(TokenKind::Right))
        fatal("expected '>' after 'sizeof' type", &m_Current->md);
    next(); // '>'

    return new SizeofExpr(md, m_Context->getU64Type(), T);
}

TypeSpecExpr *Parser::parse_spec() {
    if (match(TokenKind::Path))
        backtrack(1);

    Metadata md = m_Current->md;
    String name;
    Expr *E = nullptr;

    if (!match(TokenKind::Identifier))
        fatal("expected callee identifier", &md);

    name = m_Current->value;
    next(); // identifier

    if (!match(TokenKind::Path))
        fatal("expected '::'", &m_Current->md);
    next(); // '::'

    if (!match(TokenKind::Identifier))
        fatal("expected identifier after '::' operator", &m_Current->md);

    m_AllowUnresolved = true;
    E = parse_ident();
    if (!E)
        fatal("expected expression after '::' operator", &m_Current->md);

    m_AllowUnresolved = false;
    RefExpr *RE = dynamic_cast<RefExpr *>(E);
    if (!RE)
        fatal("expected reference expression after '::' operator", &m_Current->md);

    return new TypeSpecExpr(md, name, RE);
}

Expr *Parser::parse_unary_prefix() {
    UnaryExpr::Kind op = get_un_operator();
    if (UnaryExpr::isPrefixOp(op)) {
        Metadata md = m_Current->md;
        next();

        Expr *E = parse_unary_prefix();
        if (!E)
            fatal("expected unary prefix expression", &m_Current->md);

        return new UnaryExpr(md, nullptr, op, E, false);
    } else
        return parse_unary_postfix();
}

Expr *Parser::parse_unary_postfix() {
    Expr *E = parse_primary();
    if (!E)
        fatal("expected expression", &m_Current->md);

    while (1) {
        UnaryExpr::Kind op = get_un_operator();
        if (UnaryExpr::isPostfixOp(op)) {
            // Operator is a recognized postfix unary operator.
            Metadata md = m_Current->md;
            next();

            E = new UnaryExpr(md, nullptr, op, E, true);
        } else if (match(TokenKind::SetBrack)) {
            // Token is not an operator, but a subscript beginning '['.
            Metadata md = m_Current->md;
            next(); // '['

            Expr *Idx = parse_expr();
            if (!Idx)
                fatal("expected subscript index", &m_Current->md);

            if (!match(TokenKind::EndBrack))
                fatal("expected ']' after subscript index", &m_Current->md);
            next(); // ']'

            E = new SubscriptExpr(md, nullptr, E, Idx);
        } else
            break;
    }

    return E;
}
