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

    std::vector<TemplateParamDecl *> params;
    if (match(TokenKind::Left)) {
        next(); // '<'

        while (!match(TokenKind::Right)) {
            expect(TokenKind::Identifier, "expected parameter name");

            Metadata paramMd = m_Current->md;
            String paramName = m_Current->value;
            next(); // identifier

            params.push_back(new TemplateParamDecl(
                Runes(),
                paramMd,
                paramName,
                params.size()
            ));

            if (match(TokenKind::Right))
                break;

            expect_and_eat(TokenKind::Comma, 
                "expected ',' in template parameter list");
        }

        if (params.empty())
            fatal("template must have at least one parameter", &name.md);

        next(); // '>'
    }

    if (match(TokenKind::Path))
        next(); // '::'

    NamedDecl *D = nullptr;
    if (match(TokenKind::SetParen)) {
        D = parse_function(name, params);
    } else if (match(TokenKind::SetBrace)) {
        D = parse_struct(name, params);
    } else if (match_keyword("fix") || match_keyword("mut")) {
        if (!params.empty())
            fatal("global variable cannot be made a template", &name.md);

        D = parse_global_var(name);
    } else {
        if (!params.empty())
            fatal("enum cannot be made a template", &name.md);

        D = parse_enum(name);
    }

    if (D->hasPublicRune())
        m_Unit->addExport(D);

    return D;
}

FunctionDecl *Parser::parse_function(const Token &name, std::vector<TemplateParamDecl *> tps) {
    Stmt *body = nullptr;
    Scope *scope = enter_scope();
    std::vector<ParamDecl *> params;
    next(); // '('

    for (auto &tp : tps)
        m_Scope->addDecl(tp);

    while (!match(TokenKind::EndParen)) {
        Metadata paramMd = m_Current->md;
        String paramName;
        Type *paramTy = nullptr;

        expect(TokenKind::Identifier, "expected function parameter name");
        paramName = m_Current->value;
        next(); // identifier

        expect_and_eat(TokenKind::Colon, "expected ':' after parameter name");
        paramTy = parse_type();
        if (paramTy->isVoid())
            fatal("parameter type cannot be 'void'", &m_Current->md);

        ParamDecl *param = new ParamDecl(
            Runes(),
            paramMd,
            paramName,
            paramTy
        );
        scope->addDecl(param);
        params.push_back(param);

        if (match(TokenKind::EndParen))
            break;

        expect_and_eat(TokenKind::Comma, "expected ',' in function parameter list");
    }

    next(); // ')'

    Type *retTy = nullptr;
    if (match(TokenKind::Identifier)) {
        retTy = parse_type();
    } else if (match(TokenKind::Arrow)) {
        next(); // '->'
        retTy = parse_type();
    } else {
        retTy = m_Context->getVoidType();
    }

    std::vector<Type *> paramTys;
    paramTys.reserve(params.size());
    for (auto &param : params)
        paramTys.push_back(param->getType());

    if (match(TokenKind::SetBrace)) {
        body = parse_stmt();
        if (!body)
            fatal("expected function body", &m_Current->md);
    } else {
        expect_and_eat(TokenKind::Semi, "expected ';' after function declaration");   
    }
    
    exit_scope();

    FunctionDecl *fn = new FunctionDecl(
        m_Runes, 
        name.md, 
        name.value, 
        FunctionType::create(m_Context, paramTys, retTy), 
        scope, 
        params,
        body,
        tps
    );
    m_Scope->addDecl(fn);
    return fn;
}

VarDecl *Parser::parse_global_var(const Token &name) {
    Type *T = nullptr;
    Expr *init = nullptr;
    bool mut = m_Current->value == "mut";
    next(); // 'fix' or 'mut'

    T = parse_type();

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
        T = parse_type();
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
    EnumType *ty = EnumType::create(m_Context, name.value, parse_type());

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

        Runes VariantRunes;
        if (m_Runes.has(Rune::Public))
            VariantRunes.set(Rune::Public);

        EnumVariantDecl *Variant = new EnumVariantDecl(
            VariantRunes,
            var_md,
            var_name,
            ty,
            var_val
        );

        m_Scope->addDecl(Variant);
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

StructDecl *Parser::parse_struct(const Token &name, std::vector<TemplateParamDecl *> tps) {
    Runes runes = m_Runes;
    StructType *ty = nullptr;
    Scope *scope = enter_scope();
    std::vector<FieldDecl *> Fields;
    std::vector<FunctionDecl *> Functions;

    if (!match(TokenKind::SetBrace))
        fatal("expected '{' after struct binding", &m_Current->md);
    next(); // '{'

    for (auto &tp : tps)
        m_Scope->addDecl(tp);

    while (!match(TokenKind::EndBrace)) {
        parse_runes();

        if (!match(TokenKind::Identifier))
            fatal("expected named declaration", &m_Current->md);

        Token member_name = *m_Current;
        std::vector<TemplateParamDecl *> templateParams;
        next();

        if (match(TokenKind::Left)) {
            next(); // '<'
        
            while (!match(TokenKind::Right)) {
                expect(TokenKind::Identifier, "expected parameter name");

                Metadata paramMd = m_Current->md;
                String paramName = m_Current->value;
                next(); // identifier
    
                templateParams.push_back(new TemplateParamDecl(
                    Runes(),
                    paramMd,
                    paramName,
                    templateParams.size()
                ));
    
                if (match(TokenKind::Right))
                    break;
    
                expect_and_eat(TokenKind::Comma, 
                    "expected ',' in template parameter list");
            }

            if (templateParams.empty())
                fatal("template must have at least one parameter", &name.md);

            next(); // '>'
        }

        if (match(TokenKind::Colon)) {
            next(); // ':'

            Type *field_ty = parse_type();
            Expr *field_init = nullptr;

            if (match(TokenKind::Equals)) {
                next(); // '='
                field_init = parse_expr();
                if (!field_init)
                    fatal("expected expression after '='", &m_Current->md);

                if (!field_init->isConstant())
                    fatal("struct field must be initialized with a constant", 
                          &m_Current->md);
            }

            FieldDecl *F = new FieldDecl(m_Runes, member_name.md, 
                member_name.value, field_ty, Fields.size(), field_init);
            m_Scope->addDecl(F);
            Fields.push_back(F);

            if (match(TokenKind::EndBrace))
                break;

            expect_and_eat(TokenKind::Comma, "expected ',' in struct member list");
        } else if (match(TokenKind::Path)) {
            next(); // '::'
            
            FunctionDecl *F = parse_function(member_name, templateParams);
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

    StructDecl *_struct = new StructDecl(
        runes,
        name.md,
        name.value,
        ty,
        scope,
        Fields,
        Functions,
        tps
    );
    ty->setDecl(_struct);
    m_Scope->addDecl(_struct);
    return _struct;
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
