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
    else if (match(TokenKind::Sign))
        return parse_rune_expr();
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

    NamedDecl *D = m_Scope->lookup(m_Current->value);
    if (auto *use = dynamic_cast<UseDecl *>(D)) {
        // use::ident
        // use::ident(...)
        // use::ident { ... }
        // ...
        return parse_use_spec(use);
    } else if (auto *var = dynamic_cast<VarDecl *>(D)) {
        // ident
        return parse_ref();
    }

    unsigned identPos = save_pos();
    next(); // identifier

    switch (m_Current->kind) {
    case TokenKind::Left:
        // In cases of template declaration references, i.e. the call
        // `foo<i32>(...)`, the left angle `<` needs to be differentiated from
        // a less-than operator.
        restore_pos(identPos);

        if (D && dynamic_cast<VarDecl *>(D)) {
            // Since variables cannot be forward referenced, we can assume
            // it's part of a comparison.
            return parse_ref();
        }

        // The reference is either forward or a non-variable. Since the
        // following tokens could be either a:
        //
        // 1. Template Type Specifier `::`
        //   box<i32>::ident...
        // 2. Template Type Initializer `{`
        //   box<i32> { ... }
        // 3. Template Function Call `(`
        //   box<i32>(...)
        //
        // We can start by treating the chunk as a type, and see what follows.
        parse_type_name();

        switch (m_Current->kind) {
        case TokenKind::Path:
            restore_pos(identPos);
            return parse_spec();

        case TokenKind::SetBrace:
            restore_pos(identPos);
            return parse_init();

        case TokenKind::SetParen:
            restore_pos(identPos);
            return parse_call();

        default:
            fatal("expected a template initializer or type specifier", &m_Current->md);
        }

    case TokenKind::SetParen:
        // ident(...)
        restore_pos(identPos);
        return parse_call();

    case TokenKind::SetBrace:
        // ident { ... }
        restore_pos(identPos);
        return parse_init();

    case TokenKind::Path:
        // type::ident
        restore_pos(identPos);
        return parse_spec();

    default:
        // ident
        restore_pos(identPos);
        return parse_ref();
    }
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
        ArrayType::get(m_Context, m_Context->getCharType(), m_Current->value.size() + 1),
        m_Current->value
    );
    next();
    return S;
}

NilLiteral *Parser::parse_nil() {
    NilLiteral *N = new NilLiteral(
        m_Current->md,
        PointerType::get(m_Context, m_Context->getVoidType())
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

    T = parse_type();
    
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
    Metadata md = m_Current->md;
    std::vector<Expr *> Args;
    std::vector<Type *> TArgs;
    String callee;

    if (!match(TokenKind::Identifier))
        fatal("expected callee identifier", &md);

    callee = m_Current->value;
    next(); // identifier

    if (match(TokenKind::Left)) {
        next(); // '<'

        while (!match(TokenKind::Right)) {
            Type *targ = parse_type();
            if (!targ)
                fatal("expected type argument", &m_Current->md);

            TArgs.push_back(targ);

            if (match(TokenKind::Right))
                break;

            if (!match(TokenKind::Comma))
                fatal("expected ',' or '>' in function call type arguments", &m_Current->md);
            next(); // ','
        }

        if (TArgs.empty())
            fatal("function call type arguments cannot be empty", &m_Current->md);

        next(); // '>'
    }

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
    return new CallExpr(md, nullptr, callee, nullptr, Args, TArgs);
}

SizeofExpr *Parser::parse_sizeof() {
    Metadata md = m_Current->md;
    next(); // 'sizeof'

    if (!match(TokenKind::Left))
        fatal("expected '<' after 'sizeof' keyword", &m_Current->md);
    next(); // '<'

    Type *T = parse_type();

    if (!match(TokenKind::Right))
        fatal("expected '>' after 'sizeof' type", &m_Current->md);
    next(); // '>'

    return new SizeofExpr(md, m_Context->getU64Type(), T);
}

Expr *Parser::parse_use_spec(UseDecl *use) {
    Metadata md = m_Current->md;
    String name;
    Expr *E = nullptr;

    if (!match(TokenKind::Identifier))
        fatal("expected identifier", &md);

    name = m_Current->value;

    // The following code must handle these cases:
    //
    // 1. use::ident
    // 2. use::ident(...)
    // 3. use::ident { ... }, where use::ident is a type
    //
    // Because case 3 is a type and could have extra sugar, i.e. array brackets
    // [] or indirection *, we need to eat all possible type tokens to get to
    // the defining tokens '(', '{', etc.

    unsigned identPos = save_pos();
    parse_type_name();

    switch (m_Current->kind) {
    case TokenKind::SetBrace:
        // use::ident { ... }
        restore_pos(identPos);
        return parse_init();
    default:
        break;
    }

    restore_pos(identPos);
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
    RefExpr *R = dynamic_cast<RefExpr *>(E);
    if (!R)
        fatal("expected reference expression after '::' operator", &m_Current->md);

    return new UnitSpecExpr(md, use, R);
}

Expr *Parser::parse_spec() {
    Metadata md = m_Current->md;
    String name;
    Expr *E = nullptr;

    if (!match(TokenKind::Identifier))
        fatal("expected identifier", &md);

    name = parse_type_name();

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

InitExpr *Parser::parse_init() {
    Metadata md = m_Current->md;
    Type *T = parse_type();
    std::vector<FieldInitExpr *> Fields;

    if (!match(TokenKind::SetBrace))
        fatal("expected '{' after type", &m_Current->md);
    next(); // '{'

    while (!match(TokenKind::EndBrace)) {
        Metadata field_md = m_Current->md;

        if (!match(TokenKind::Identifier))
            fatal("expected field name", &m_Current->md);

        String name = m_Current->value;
        next(); // identifier

        if (!match(TokenKind::Colon))
            fatal("expected ':' after field name", &m_Current->md);
        next(); // ':'

        Expr *E = parse_expr();
        if (!E)
            fatal("expected field initializer expression", &m_Current->md);

        Fields.push_back(
            new FieldInitExpr(field_md, nullptr, name, E)
        );

        if (match(TokenKind::EndBrace))
            break;

        if (!match(TokenKind::Comma))
            fatal("expected ',' or '}' in initializer list", &m_Current->md);
        next(); // ','
    }

    next(); // '}'

    if (Fields.empty())
        fatal("initializer enclosed by {, } cannot be empty", &m_Current->md);

    return new InitExpr(md, T, Fields);
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
        Metadata md = m_Current->md;

        UnaryExpr::Kind op = get_un_operator();
        if (UnaryExpr::isPostfixOp(op)) {
            // Operator is a recognized postfix unary operator.
            next();

            E = new UnaryExpr(md, nullptr, op, E, true);
        } else if (match(TokenKind::SetBrack)) {
            // Token is not an operator, but a subscript beginning '['
            next(); // '['

            Expr *Idx = parse_expr();
            if (!Idx)
                fatal("expected subscript index", &m_Current->md);

            if (!match(TokenKind::EndBrack))
                fatal("expected ']' after subscript index", &m_Current->md);
            next(); // ']'

            E = new SubscriptExpr(md, nullptr, E, Idx);
        } else if (match(TokenKind::Dot)) {
            // Token is not an operator, but an access with '.'
            next(); // '.'

            if (!match(TokenKind::Identifier)) {
                fatal("expected struct member name after '.' operator", 
                    &m_Current->md);
            }

            String member = m_Current->value;
            next(); // identifier

            std::vector<Type *> TArgs;
            if (match(TokenKind::Left)) {
                next(); // '<'

                while (!match(TokenKind::Right)) {
                    Type *targ = parse_type();
                    if (!targ)
                        fatal("expected type argument", &m_Current->md);

                    TArgs.push_back(targ);

                    if (match(TokenKind::Right))
                        break;

                    if (!match(TokenKind::Comma))
                        fatal("expected ',' or '>' in type argument list", &m_Current->md);
                    next(); // ','
                }

                if (TArgs.empty())
                    fatal("type argument list cannot be empty", &m_Current->md);

                next(); // '>'
            }

            if (match(TokenKind::SetParen)) {
                // The member is followed by a '(', so this is a method call.
                next(); // '('
                std::vector<Expr *> Args;
                
                while (!match(TokenKind::EndParen)) {
                    Expr *arg = parse_expr();
                    if (!arg)
                        fatal("expected method call argument" , &m_Current->md);

                    Args.push_back(arg);

                    if (match(TokenKind::EndParen))
                        break;

                    if (!match(TokenKind::Comma))
                        fatal("expected ',' or ')' in method call", &m_Current->md);
                    next(); // ','
                }

                next(); // ')'

                E = new MethodCallExpr(md, nullptr, member, E, nullptr, Args, TArgs);
            } else {
                E = new AccessExpr(md, nullptr, member, E);
            }
        } else
            break;
    }

    return E;
}

Expr *Parser::parse_rune_expr() {
    next(); // '$'

    if (!match(TokenKind::Identifier))
        fatal("expected identifier after rune symbol '$'", &m_Current->md);

    String ident = m_Current->value;

    if (ident == "syscall")
        return parse_rune_syscall();
    else
        fatal("unknown rune: " + ident, &m_Current->md);
}

RuneSyscallExpr *Parser::parse_rune_syscall() {
    Metadata md = m_Current->md;
    unsigned num;
    std::vector<Expr *> Args;
    next(); // 'syscall'

    if (!match(TokenKind::Left))
        fatal("expected '<' after 'syscall' rune", &m_Current->md);
    next(); // '<'

    if (!match(LiteralKind::Integer))
        fatal("expected syscall number", &m_Current->md);

    num = std::stoul(m_Current->value);
    next(); // syscall number

    if (!match(TokenKind::Right))
        fatal("expected '>' after syscall number", &m_Current->md);
    next(); // '>'

    if (!match(TokenKind::SetParen))
        fatal("expected '(' after syscall number", &m_Current->md);
    next(); // '('

    while (!match(TokenKind::EndParen)) {
        Expr *Arg = parse_expr();
        if (!Arg)
            fatal("expected syscall argument", &m_Current->md);

        Args.push_back(Arg);

        if (match(TokenKind::EndParen))
            break;

        if (!match(TokenKind::Comma))
            fatal("expected ',' or ')' in syscall arguments", &m_Current->md);
        next(); // ','
    }

    next(); // ')'
    return new RuneSyscallExpr(md, m_Context->getI64Type(), num, Args);
}
