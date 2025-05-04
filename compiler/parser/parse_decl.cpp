#include "parser.h"
#include "../core/logger.h"

using namespace meddle;

Decl *Parser::parse_decl() {
    parse_runes();

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
    else
        return parse_enum(name);
}

FunctionDecl *Parser::parse_function(const Token &name) {
    Type *ret = nullptr;
    Stmt *body = nullptr;
    Scope *scope = enter_scope();
    std::vector<ParamDecl *> params;

    next(); // '('

    while (!match(TokenKind::EndParen)) {
        Metadata param_md = m_Current->md;
        String param_name;
        Type *param_ty = nullptr;

        if (!match(TokenKind::Identifier))
            fatal("expected parameter name", &param_md);
        
        param_name = m_Current->value;
        next();

        if (!match(TokenKind::Colon))
            fatal("expected ':' after parameter name", &m_Current->md);
        next(); // ':'

        param_ty = parse_type(true);
        if (param_ty->isVoid())
            fatal("parameter type cannot be 'void'", &m_Current->md);

        ParamDecl *param = new ParamDecl(
            Runes(),
            param_md,
            param_name,
            param_ty,
            params.size()
        );
        scope->addDecl(param);
        params.push_back(param);

        if (match(TokenKind::EndParen))
            break;

        if (!match(TokenKind::Comma))
            fatal("expected ',' or ')' in function parameter list", 
                  &m_Current->md);
        next(); // ','
    }

    next(); // ')'

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
        m_Runes, 
        name.md, 
        name.value, 
        ty, 
        scope, 
        params, 
        body
    );
    exit_scope();
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
        m_Runes,
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

    if (match(TokenKind::Colon)) {
        next(); // ':'
        T = parse_type(true);
    }

    if (match(TokenKind::Equals)) {
        next(); // '='
        init = parse_expr();
        if (!init)
            fatal("expected expression after '='", &m_Current->md);
    } else if (!mut) {
        fatal("immutable variable must be initialized: " + name, 
              &m_Current->md);
    } else if (!T) {
        fatal("type cannot be inferred without an initializer: " + name, 
              &m_Current->md);
    }

    if (match(TokenKind::Semi))
        next(); // ';'
    else
        fatal("expected ';' after variable declaration", &m_Current->md);

    VarDecl *var = new VarDecl(
        Runes(),
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

EnumDecl *Parser::parse_enum(const Token &name) {
    std::vector<EnumVariantDecl *> Variants;
    EnumType *ty = EnumType::create(m_Context, name.value, parse_type(true));

    if (!match(TokenKind::SetBrace))
        fatal("expected '{' after enum type", &m_Current->md);
    next(); // '{'

    long curr_val = 0;
    while (!match(TokenKind::EndBrace)) {
        Metadata var_md = m_Current->md;
        String var_name;
        long var_val = curr_val;

        if (!match(TokenKind::Identifier))
            fatal("expected enum variant name", &var_md);

        var_name = m_Current->value;
        next();

        if (match(TokenKind::Equals)) {
            next(); // '='

            if (!match(LiteralKind::Integer))
                fatal("expected integer literal after '='", &m_Current->md);
            
            var_val = std::stol(m_Current->value);
            curr_val = var_val + 1;
            next(); // integer literal
        }

        EnumVariantDecl *Variant = new EnumVariantDecl(
            Runes(),
            var_md,
            var_name,
            ty,
            var_val
        );

        Variants.push_back(Variant);

        if (match(TokenKind::EndBrace))
            break;

        if (!match(TokenKind::Comma))
            fatal("expected ',' or '}' in enum variant list", &m_Current->md);
        next(); // ','
    }

    next(); // '}'

    if (Variants.empty())
        fatal("enum must have at least one variant", &m_Current->md);

    EnumDecl *Enum = new EnumDecl(
        m_Runes,
        name.md,
        name.value,
        ty,
        Variants
    );
    ty->setDecl(Enum);
    m_Scope->addDecl(Enum);
    return Enum;
}
