#ifndef MEDDLE_PARSER_H
#define MEDDLE_PARSER_H

#include "../lexer/tokenstream.h"
#include "../tree/stmt.h"
#include "../tree/unit.h"
#include "../core/logger.h"
#include <vector>

namespace meddle {

class Parser final {
    TokenStream m_Stream;
    TranslationUnit *m_Unit;
    Context *m_Context;
    Scope *m_Scope;
    const Token *m_Current;
    unsigned m_Saved;
    Runes m_Runes;
    bool m_AllowUnresolved = false;

    void next() { m_Current = m_Stream.get(); }

    void backtrack(unsigned n = 1);

    void skip(unsigned n = 1);

    unsigned save_pos() { return m_Saved = m_Stream.getPos(); }

    void restore_pos(unsigned pos) {
        m_Stream.setPos(pos - 1);
        m_Current = m_Stream.get();
    }

    bool match(TokenKind K) const { return m_Current->kind == K; }

    bool match(LiteralKind K) const {
        return m_Current->kind == TokenKind::Literal 
            && m_Current->literal == K;
    }

    bool match_keyword(const char *KW) const {
        return m_Current->kind == TokenKind::Identifier 
            && m_Current->value == KW;
    }

    void expect(TokenKind K, const String &msg) const {
        if (m_Current->kind != K)
            fatal(msg, &m_Current->md);
    }

    void expect_and_eat(TokenKind K, const String &msg) {
        if (m_Current->kind != K)
            fatal(msg, &m_Current->md);
        next();
    }

    Scope *enter_scope() { return m_Scope = new Scope(m_Scope); }

    void exit_scope() { m_Scope = m_Scope->getParent(); }

    /// Returns the precedence for the current token if it is a operator or -1.
    int get_bin_precedence() const;

    /// Returns the equivelant binary operator kind for the current token.
    BinaryExpr::Kind get_bin_operator() const;

    /// Returns the equivelant unary operator kind for the current token.
    UnaryExpr::Kind get_un_operator() const;

    String parse_type_name();
    Type *parse_type();
    void parse_runes();

    Decl *parse_decl();
    FunctionDecl *parse_function(const Token &name, std::vector<TemplateParamDecl *> tps);
    VarDecl *parse_global_var(const Token &name);
    VarDecl *parse_var(bool mut);
    EnumDecl *parse_enum(const Token &name);
    StructDecl *parse_struct(const Token &name, std::vector<TemplateParamDecl *> tps);
    UseDecl *parse_use();

    Stmt *parse_stmt();
    BreakStmt *parse_break();
    ContinueStmt *parse_continue();
    CompoundStmt *parse_compound();
    DeclStmt *parse_decl_stmt();
    ExprStmt *parse_expr_stmt();
    IfStmt *parse_if();
    MatchStmt *parse_match();
    RetStmt *parse_ret();
    UntilStmt *parse_until();

    Expr *parse_expr();
    Expr *parse_primary();
    Expr *parse_ident();

    BoolLiteral *parse_bool();
    IntegerLiteral *parse_int();
    FloatLiteral *parse_fp();
    CharLiteral *parse_char();
    StringLiteral *parse_str();
    NilLiteral *parse_nil();

    Expr *parse_binary(Expr *B, int precedence);
    ArrayExpr *parse_array();
    CastExpr *parse_cast();
    ParenExpr *parse_paren();
    RefExpr *parse_ref();
    CallExpr *parse_call();
    SizeofExpr *parse_sizeof();
    Expr *parse_spec();
    Expr *parse_use_spec(UseDecl *use);
    InitExpr *parse_init();
    Expr *parse_unary_prefix();
    Expr *parse_unary_postfix();

    Expr *parse_rune_expr();
    RuneSyscallExpr *parse_rune_syscall();

public:
    Parser(const File &F, const TokenStream &S);

    TranslationUnit *get() const { return m_Unit; }
};

} // namespace meddle

#endif // MEDDLE_PARSER_H
