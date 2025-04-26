#include "parser.h"
#include "../core/logger.h"

using namespace meddle;

Stmt *Parser::parseStmt() {
    if (match(TokenKind::SetBrace))
        return parseCompoundStmt();
    else if (matchKeyword("fix") || matchKeyword("mut"))
        return parseDeclStmt();
    else if (matchKeyword("if"))
        return parseIfStmt();
    else if (matchKeyword("ret"))
        return parseRetStmt();

    return nullptr;
}

CompoundStmt *Parser::parseCompoundStmt() {
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

IfStmt *Parser::parseIfStmt() {
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

RetStmt *Parser::parseRetStmt() {
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
