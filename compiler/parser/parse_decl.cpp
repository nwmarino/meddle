#include "parser.h"
#include "../core/logger.h"

using namespace meddle;

Decl *Parser::parse_decl() {
    parse_attributes();

    if (!match(TokenKind::Identifier))
        fatal("expected declaration identifier", &m_Current->md);

    Token name = *m_Current;
    next();

    if (match(TokenKind::Path))
        next(); // '::'

    if (match(TokenKind::SetParen))
        return parse_function(name);
    else if (match_keyword("fix") || match_keyword("mut"))
        return parse_global_var(name);

    return nullptr;
}

FunctionDecl *Parser::parse_function(const Token &name) {
    next(); // '('
    next(); // ')'

    Type *ret = nullptr;
    Stmt *body = nullptr;
    Scope *scope = enter_scope();
    std::vector<ParamDecl *> params = {};

    if (match(TokenKind::Identifier))
        ret = parse_type(true);
    else
        ret = m_Context->getVoidType();

    if (match(TokenKind::SetBrace)) {
        body = parse_stmt();
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

VarDecl *Parser::parse_global_var(const Token &name) {
    Type *T = nullptr;
    Expr *init = nullptr;
    bool mut = m_Current->value == "mut";
    next(); // 'fix' or 'mut'

    T = parse_type(true);

    if (!match(TokenKind::Equals))
        fatal("global variable must have an initializer", &m_Current->md);
    next(); // '='

    init = parse_expr();
    if (!init)
        fatal("expected an initializer", &m_Current->md);

    if (!init->isConstant())
        fatal("global variable must be initialized with a constant", 
            &m_Current->md);

    if (!match(TokenKind::Semi))
        fatal("expected ';' after variable", &m_Current->md);
    next(); // ';'

    VarDecl *var = new VarDecl(
        m_Attributes,
        name.md,
        name.value,
        T,
        init,
        mut,
        true
    );
    m_Scope->addDecl(var);
    return var;
}

VarDecl *Parser::parse_var(bool mut) {
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

    T = parse_type();
    if (!T)
        fatal("expected type after ':'", &m_Current->md);

    if (match(TokenKind::Equals)) {
        next(); // '='
        init = parse_expr();
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
