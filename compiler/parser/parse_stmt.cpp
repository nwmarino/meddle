#include "parser.h"
#include "../core/logger.h"

using namespace meddle;

Stmt *Parser::parse_stmt() {
    if (match(TokenKind::SetBrace))
        return parse_compound();
    else if (match_keyword("break"))
        return parse_break();
    else if (match_keyword("continue"))
        return parse_continue();
    else if (match_keyword("fix") || match_keyword("mut"))
        return parse_decl_stmt();
    else if (match_keyword("if"))
        return parse_if();
    else if (match_keyword("match"))
        return parse_match();
    else if (match_keyword("ret"))
        return parse_ret();
    else if (match_keyword("until"))
        return parse_until();

    return parse_expr_stmt();
}

BreakStmt *Parser::parse_break() {
    Metadata md = m_Current->md;
    next(); // 'break'

    if (!match(TokenKind::Semi))
        fatal("expected ';' after break statement", &md);

    next(); // ';'
    return new BreakStmt(md);
}

ContinueStmt *Parser::parse_continue() {
    Metadata md = m_Current->md;
    next(); // 'continue'

    if (!match(TokenKind::Semi))
        fatal("expected ';' after continue statement", &md);

    next(); // ';'
    return new ContinueStmt(md);
}

CompoundStmt *Parser::parse_compound() {
    CompoundStmt *C = new CompoundStmt(m_Current->md, enter_scope());
    next(); // '{'

    while (!match(TokenKind::EndBrace)) {
        Stmt *S = parse_stmt();
        if (!S)
            fatal("expected statement", &m_Current->md);

        while (match(TokenKind::Semi))
            next(); // ';'

        C->addStmt(S);
    }

    next(); // '}'
    exit_scope();
    return C;
}

DeclStmt *Parser::parse_decl_stmt() {
    Decl *D = nullptr;

    if (match_keyword("fix"))
        D = parse_var(false);
    else if (match_keyword("mut"))
        D = parse_var(true);

    if (!D)
        fatal("expected variable declaration", &m_Current->md);

    if (match(TokenKind::Semi))
        next();

    return new DeclStmt(D->getMetadata(), D);
}

ExprStmt *Parser::parse_expr_stmt() {
    Expr *E = parse_expr();
    if (!E)
        return nullptr;

    return new ExprStmt(E->getMetadata(), E);
}

IfStmt *Parser::parse_if() {
    Metadata md = m_Current->md;
    Expr *C = nullptr;
    Stmt *T = nullptr;
    Stmt *E = nullptr;
    next(); // 'if'

    C = parse_expr();
    if (!C)
        fatal("expected expression after 'if'", &md);

    T = parse_stmt();
    if (!T)
        fatal("expected statement after 'if' condition", &m_Current->md);

    if (match_keyword("else")) {
        next(); // 'else'
        E = parse_stmt();
        if (!E)
            fatal("expected statement after 'else'", &m_Current->md);
    }

    return new IfStmt(md, C, T, E);
}

MatchStmt *Parser::parse_match() {
    Metadata md = m_Current->md;
    Expr *P = nullptr;
    std::vector<CaseStmt *> cases = {};
    Stmt *D = nullptr;
    next(); // 'match'

    P = parse_expr();
    if (!P)
        fatal("expected expression after 'match'", &md);

    if (!match(TokenKind::SetBrace))
        fatal("expected '{' after match expression", &md);
    next(); // '{'

    while (!match(TokenKind::EndBrace)) {
        if (match_keyword("_")) {
            if (D)
                fatal("duplicate default case", &m_Current->md);
            next(); // '_'

            if (!match(TokenKind::Arrow))
                fatal("expected '->' after default case", &m_Current->md);
            next(); // '->'

            D = parse_stmt();
            if (!D)
                fatal("expected default statement", &m_Current->md);
        } else {
            Expr *CP = parse_expr();
            if (!CP)
                fatal("expected case expression", &m_Current->md);

            if (!match(TokenKind::Arrow))
                fatal("expected '->' after case expression", &m_Current->md);
            next(); // '->'

            Stmt *B = parse_stmt();
            if (!B)
                fatal("expected statement after case expression", &m_Current->md);

            cases.push_back(new CaseStmt(md, CP, B));
        }

        if (match(TokenKind::EndBrace))
            break;

        if (match(TokenKind::Comma))
            next(); // ','
    }

    next(); // '}'

    if (cases.empty())
        fatal("match statement must have at least one case", &m_Current->md);

    return new MatchStmt(md, P, cases, D);
}

RetStmt *Parser::parse_ret() {
    Metadata md = m_Current->md;
    Expr *E = nullptr;
    next(); // 'ret'

    if (!match(TokenKind::Semi)) {
        E = parse_expr();
        if (!E)
            fatal("expected return expression", &md);
    }

    if (!match(TokenKind::Semi))
        fatal("expected ';' after return statement", &md);

    next(); // ';'
    return new RetStmt(md, E);    
}

UntilStmt *Parser::parse_until() {
    Metadata md = m_Current->md;
    Expr *C = nullptr;
    Stmt *B = nullptr;
    next(); // 'until'

    C = parse_expr();
    if (!C)
        fatal("expected expression after 'until'", &md);

    B = parse_stmt();
    if (!B)
        fatal("expected statement after 'until' condition", &m_Current->md);

    return new UntilStmt(md, C, B);
}
