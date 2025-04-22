#include "parser.h"
#include "../core/logger.h"

using namespace meddle;

Decl *Parser::parseDecl() {
    parseAttributes();

    if (!match(TokenKind::Identifier))
        fatal("expected declaration identifier", &m_Current->md);

    Token name = *m_Current;
    next();

    if (match(TokenKind::Path))
        next(); // '::'

    if (match(TokenKind::SetParen))
        return parseFunction(name);

    return nullptr;
}

FunctionDecl *Parser::parseFunction(const Token &name) {
    next(); // '('
    next(); // ')'

    Type *ret = nullptr;
    Stmt *body = nullptr;
    Scope *scope = enterScope();
    std::vector<ParamDecl *> params = {};

    if (match(TokenKind::Identifier))
        ret = parseType(true);
    else
        ret = m_Context->getVoidType();

    if (match(TokenKind::SetBrace)) {
        body = parseStmt();
        if (!body)
            fatal("expected function body", &m_Current->md);
    } else if (!match(TokenKind::Semi)) {
        fatal("expected ';' after empty function", &m_Current->md);
    } else
        next(); // ';'
    
    std::vector<Type *> paramTys = {};
    paramTys.reserve(params.size());
    for (auto &P : params)
        paramTys.push_back(P->getType());

    FunctionType *ty = new FunctionType(paramTys, ret);
    m_Context->addType(ty);

    FunctionDecl *fn = new FunctionDecl(
        m_Attributes, 
        name.md, 
        name.value, 
        ty, 
        scope, 
        params, 
        body
    );
    m_Scope->addDecl(fn);
    return fn;
}

VarDecl *Parser::parseVariable(bool mut) {
    Metadata md = m_Current->md;
    String name;
    Type *T = nullptr;
    Expr *init = nullptr;
    next(); // 'fix' or 'mut'

    if (!match(TokenKind::Identifier))
        fatal("expected variable name", &m_Current->md);
    name = m_Current->value;
    next();

    if (!match(TokenKind::Colon))
        fatal("expected ':' after variable name", &m_Current->md);
    next(); // ':'

    T = parseType();
    if (!T)
        fatal("expected type after ':'", &m_Current->md);

    if (match(TokenKind::Equals)) {
        next(); // '='
        init = parseExpr();
        if (!init)
            fatal("expected expression after '='", &m_Current->md);
    } else if (!mut)
        fatal("immutable variable must be initialized: " + name, &m_Current->md);

    if (match(TokenKind::Semi))
        next(); // ';'
    else
        fatal("expected ';' after variable declaration", &m_Current->md);

    VarDecl *var = new VarDecl(
        Attributes(),
        md,
        name,
        T,
        init,
        mut,
        false
    );
    m_Scope->addDecl(var);
    return var;
}
