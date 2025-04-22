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

    unsigned savePos() { return m_Saved = m_Stream.getPos(); }

    void restorePos(unsigned pos) {
        m_Stream.setPos(pos);
        m_Current = m_Stream.get(pos);
    }

    bool match(TokenKind K) const { return m_Current->kind == K; }

    bool match(LiteralKind K) const {
        return m_Current->kind == TokenKind::Literal 
            && m_Current->literal == K;
    }

    bool matchKeyword(const char *KW) const {
        return m_Current->kind == TokenKind::Identifier 
            && m_Current->value == KW;
    }

    Scope *enterScope() { m_Scope = new Scope(m_Scope); return m_Scope; }

    void exitScope() { m_Scope = m_Scope->getParent(); }

    Type *parseType(bool produce = true);
    void parseAttributes();

    Decl *parseDecl();
    FunctionDecl *parseFunction(const Token &name);
    VarDecl *parseVariable(bool mut);

    Stmt *parseStmt();
    CompoundStmt *parseCompoundStmt();
    DeclStmt *parseDeclStmt();
    RetStmt *parseRetStmt();

    Expr *parseExpr();
    Expr *parsePrimaryExpr();
    Expr *parseIdentExpr();
    IntegerLiteral *parseInteger();
    RefExpr *parseRef();

public:
    Parser(const File &F, const TokenStream &S);

    TranslationUnit *get() const { return m_Unit; }
};

} // namespace meddle

#endif // MEDDLE_PARSER_H
