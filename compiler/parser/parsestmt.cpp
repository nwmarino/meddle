#include "parser.h"
#include "../core/logger.h"

using namespace meddle;

Stmt *Parser::parseStmt() {
    if (match(TokenKind::SetBrace))
        return parseCompoundStmt();
    else if (matchKeyword("ret"))
        return parseRetStmt();

    return nullptr;
}

CompoundStmt *Parser::parseCompoundStmt() {
    CompoundStmt *C = new CompoundStmt(m_Current->md, m_Scope);
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
