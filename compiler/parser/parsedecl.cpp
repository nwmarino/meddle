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

    return new FunctionDecl(
        m_Attributes, 
        name.md, 
        name.value, 
        ty, 
        scope, 
        params, 
        body
    );
}
