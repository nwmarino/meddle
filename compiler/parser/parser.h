#ifndef MEDDLE_PARSER_H
#define MEDDLE_PARSER_H

#include "../lexer/tokenstream.h"
#include "../tree/stmt.h"
#include "../tree/unit.h"
#include "../core/logger.h"

namespace meddle {

class Parser final {
    TokenStream m_Stream;
    TranslationUnit *m_Unit;
    Context *m_Context;
    Scope *m_Scope;
    const Token *m_Current;
    unsigned m_Saved;
    Attributes m_Attributes;

    void next() { m_Current = m_Stream.get(); }

    void backtrack(unsigned n = 1);

    void skip(unsigned n = 1);

    unsigned save_pos() { return m_Saved = m_Stream.getPos(); }

    void restore_pos(unsigned pos) {
        m_Stream.setPos(pos);
        m_Current = m_Stream.get(pos);
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

    Scope *enter_scope() { m_Scope = new Scope(m_Scope); return m_Scope; }

    void exit_scope() { m_Scope = m_Scope->getParent(); }

    /// Returns the precedence for the current token if it is a operator or -1.
    int get_bin_precedence() const;

    /// Returns the equivelant binary operator kind for the current token.
    BinaryExpr::Kind get_bin_operator() const;

    /// Returns the equivelant unary operator kind for the current token.
    UnaryExpr::Kind get_un_operator() const;

    Type *parse_type(bool produce = true);
    void parse_attributes();

    Decl *parse_decl();
    FunctionDecl *parse_function(const Token &name);
    VarDecl *parse_global_var(const Token &name);
    VarDecl *parse_var(bool mut);

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
    SizeofExpr *parse_sizeof();
    Expr *parse_unary_prefix();
    Expr *parse_unary_postfix();

public:
    Parser(const File &F, const TokenStream &S);

    TranslationUnit *get() const { return m_Unit; }
};

} // namespace meddle

#endif // MEDDLE_PARSER_H
