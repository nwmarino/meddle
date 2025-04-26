#include "parser.h"
#include "../core/logger.h"

using namespace meddle;

Stmt *Parser::parseStmt() {
    if (match(TokenKind::SetBrace))
        return parseCompound();
    else if (matchKeyword("fix") || matchKeyword("mut"))
        return parseDeclStmt();
    else if (matchKeyword("if"))
        return parseIf();
    else if (matchKeyword("match"))
        return parseMatchStmt();
    else if (matchKeyword("ret"))
        return parseRet();
    else if (matchKeyword("until"))
        return parseUntil();

    return nullptr;
}

CompoundStmt *Parser::parseCompound() {
    CompoundStmt *C = new CompoundStmt(m_Current->md, enterScope());
    next(); // '{'

    while (!match(TokenKind::EndBrace)) {
        Stmt *S = parseStmt();
        if (!S)
            fatal("expected statement", &m_Current->md);

        while (match(TokenKind::Semi))
            next(); // ';'

        C->addStmt(S);
    }

    next(); // '}'
    exitScope();
    return C;
}

DeclStmt *Parser::parseDeclStmt() {
    Decl *D = nullptr;

    if (matchKeyword("fix"))
        D = parseVariable(false);
    else if (matchKeyword("mut"))
        D = parseVariable(true);

    if (!D)
        fatal("expected variable declaration", &m_Current->md);

    if (match(TokenKind::Semi))
        next();

    return new DeclStmt(D->getMetadata(), D);
}

IfStmt *Parser::parseIf() {
    Metadata md = m_Current->md;
    Expr *C = nullptr;
    Stmt *T = nullptr;
    Stmt *E = nullptr;
    next(); // 'if'

    C = parseExpr();
    if (!C)
        fatal("expected expression after 'if'", &md);

    T = parseStmt();
    if (!T)
        fatal("expected statement after 'if' condition", &m_Current->md);

    if (matchKeyword("else")) {
        next(); // 'else'
        E = parseStmt();
        if (!E)
            fatal("expected statement after 'else'", &m_Current->md);
    }

    return new IfStmt(md, C, T, E);
}

MatchStmt *Parser::parseMatchStmt() {
    Metadata md = m_Current->md;
    Expr *P = nullptr;
    std::vector<CaseStmt *> cases = {};
    Stmt *D = nullptr;
    next(); // 'match'

    P = parseExpr();
    if (!P)
        fatal("expected expression after 'match'", &md);

    if (!match(TokenKind::SetBrace))
        fatal("expected '{' after match expression", &md);
    next(); // '{'

    while (!match(TokenKind::EndBrace)) {
        if (matchKeyword("_")) {
            if (D)
                fatal("duplicate default case", &m_Current->md);
            next(); // '_'

            if (!match(TokenKind::Arrow))
                fatal("expected '->' after default case", &m_Current->md);
            next(); // '->'

            D = parseStmt();
            if (!D)
                fatal("expected default statement", &m_Current->md);
        } else {
            Expr *CP = parseExpr();
            if (!CP)
                fatal("expected case expression", &m_Current->md);

            if (!match(TokenKind::Arrow))
                fatal("expected '->' after case expression", &m_Current->md);
            next(); // '->'

            Stmt *B = parseStmt();
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

RetStmt *Parser::parseRet() {
    Metadata md = m_Current->md;
    Expr *E = nullptr;
    next(); // 'ret'

    if (!match(TokenKind::Semi)) {
        E = parseExpr();
        if (!E)
            fatal("expected return expression", &md);
    }

    if (!match(TokenKind::Semi))
        fatal("expected ';' after return statement", &md);

    next(); // ';'
    return new RetStmt(md, E);    
}

UntilStmt *Parser::parseUntil() {
    Metadata md = m_Current->md;
    Expr *C = nullptr;
    Stmt *B = nullptr;
    next(); // 'until'

    C = parseExpr();
    if (!C)
        fatal("expected expression after 'until'", &md);

    B = parseStmt();
    if (!B)
        fatal("expected statement after 'until' condition", &m_Current->md);

    return new UntilStmt(md, C, B);
}
