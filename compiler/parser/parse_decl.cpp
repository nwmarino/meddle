#include "parser.h"
#include "../core/logger.h"

using namespace meddle;

Decl *Parser::parse_decl() {
    parse_runes();

    if (!match(TokenKind::Identifier))
        fatal("expected declaration identifier", &m_Current->md);

    if (match_keyword("use"))
        return parse_use();

    Token name = *m_Current;
    next();

    if (match(TokenKind::Path))
        next(); // '::'

    NamedDecl *D = nullptr;
    if (match(TokenKind::SetParen))
        D = parse_function(name);
    else if (match(TokenKind::SetBrace))
        D = parse_struct(name);
    else if (match_keyword("fix") || match_keyword("mut"))
        D = parse_global_var(name);
    else
        D = parse_enum(name);

    if (D->hasPublicRune())
        m_Unit->addExport(D);

    return D;
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

StructDecl *Parser::parse_struct(const Token &name) {
    Runes runes = m_Runes;
    StructType *ty = nullptr;
    Scope *scope = enter_scope();
    std::vector<FieldDecl *> Fields;
    std::vector<FunctionDecl *> Functions;

    if (!match(TokenKind::SetBrace))
        fatal("expected '{' after struct binding", &m_Current->md);
    next(); // '{'

    while (!match(TokenKind::EndBrace)) {
        parse_runes();

        if (!match(TokenKind::Identifier))
            fatal("expected named declaration", &m_Current->md);

        Token member_name = *m_Current;
        next();

        if (match(TokenKind::Colon)) {
            next(); // ':'

            Type *field_ty = parse_type(true);
            Expr *field_init = nullptr;

            if (match(TokenKind::Equals)) {
                next(); // '='
                field_init = parse_expr();
                if (!field_init)
                    fatal("expected expression after '='", &m_Current->md);

                if (!field_init->isConstant()) {
                    fatal("struct field must be initialized with a constant", 
                          &m_Current->md);
                }
            }

            FieldDecl *F = new FieldDecl(m_Runes, member_name.md, 
                member_name.value, field_ty, Fields.size(), field_init);
            m_Scope->addDecl(F);
            Fields.push_back(F);

            if (match(TokenKind::Comma))
                next(); // ','
            else if (match(TokenKind::EndBrace))
                break;
            else {
                fatal("expected ',' or '}' in struct member list", 
                      &m_Current->md);
            }
        } else if (match(TokenKind::Path)) {
            next(); // '::'
            
            FunctionDecl *F = parse_function(member_name);
            if (!F)
                fatal("expected function declaration", &m_Current->md);

            Functions.push_back(F);
        } else {
            fatal("expected field or method declaration", &m_Current->md);
        }
    }

    next(); // '}'
    exit_scope();

    std::vector<Type *> fieldTys = {};
    for (auto &F : Fields)
        fieldTys.push_back(F->getType());

    ty = StructType::create(m_Context, name.value, fieldTys);

    StructDecl *Struct = new StructDecl(
        runes,
        name.md,
        name.value,
        ty,
        scope,
        Fields,
        Functions
    );
    ty->setDecl(Struct);
    m_Scope->addDecl(Struct);
    return Struct;
}

UseDecl *Parser::parse_use() {
    Metadata md = m_Current->md;
    String name = "";
    String path;
    std::vector<String> names = {};
    next(); // 'use'

    if (match(TokenKind::Identifier)) {
        name = m_Current->value;
        next(); // identifier

        if (!match(TokenKind::Equals))
            fatal("expected '=' after named 'use'", &m_Current->md);
        next();
    } else if (match(TokenKind::SetBrace)) {
        next(); // '{'

        while (!match(TokenKind::EndBrace)) {
            if (!match(TokenKind::Identifier))
                fatal("expected identifier in listed use declaration", &m_Current->md);

            names.push_back(m_Current->value);
            next(); // identifier

            if (match(TokenKind::EndBrace))
                break;

            if (!match(TokenKind::Comma))
                fatal("expected ',' or '}' in listed use declaration", &m_Current->md);
            next(); // ','
        }

        next(); // '}'

        if (!match(TokenKind::Equals))
            fatal("expected '=' after listed 'use'", &m_Current->md);
        next(); // '='
    }

    if (!match(LiteralKind::String))
        fatal("expected string path after 'use'", &m_Current->md);

    path = m_Current->value;
    next();

    if (!match(TokenKind::Semi))
        fatal("expected ';' after use declaration", &m_Current->md);
    next(); // ';'

    UseDecl *use = nullptr;
    if (!name.empty()) {
        use = new UseDecl(m_Runes, md, path, name);
    } else if (!names.empty()) {
        use = new UseDecl(m_Runes, md, path, names);
    } else {
        use = new UseDecl(m_Runes, md, path);
    }

    if (use->isNamed())
        m_Scope->addDecl(use);

    return use;
}
