#ifndef MEDDLE_STMT_H
#define MEDDLE_STMT_H

#include "decl.h"
#include "expr.h"
#include "nameres.h"
#include "scope.h"
#include "../core/metadata.h"

namespace meddle {

class Scope;

class Stmt {
protected:
    Metadata m_Metadata;

public:
    Stmt(const Metadata &M) : m_Metadata(M) {}
    virtual ~Stmt() = default;

    virtual void accept(Visitor *V) = 0;

    const Metadata &getMetadata() const { return m_Metadata; }
};

class BreakStmt final : public Stmt {
    friend class CCGN;
    friend class NameResolution;
    friend class Sema;

public:
    BreakStmt(const Metadata &M) : Stmt(M) {}
    ~BreakStmt() override = default;

    void accept(Visitor *V) override { V->visit(this); }
};

class ContinueStmt final : public Stmt {
    friend class CCGN;
    friend class NameResolution;
    friend class Sema;

public:
    ContinueStmt(const Metadata &M) : Stmt(M) {}
    ~ContinueStmt() override = default;

    void accept(Visitor *V) override { V->visit(this); }
};

class CompoundStmt final : public Stmt {
    friend class CCGN;
    friend class NameResolution;
    friend class Sema;

    Scope *m_Scope;
    std::vector<Stmt *> m_Stmts;

public:
    CompoundStmt(const Metadata &M, Scope *S, std::vector<Stmt *> stmts = {}) 
      : Stmt(M), m_Scope(S), m_Stmts(stmts) {}

    ~CompoundStmt() override {
        for (auto *stmt : m_Stmts)
            delete stmt;

        delete m_Scope;
    }

    void accept(Visitor *V) override { V->visit(this); }

    void addStmt(Stmt *S) { m_Stmts.push_back(S); }

    const std::vector<Stmt *> &getStmts() const { return m_Stmts; }

    Scope *getScope() const { return m_Scope; }
};

class DeclStmt final : public Stmt {
    friend class CCGN;
    friend class NameResolution;
    friend class Sema;
    
    Decl *m_Decl;

public:
    DeclStmt(const Metadata &M, Decl *D) : Stmt(M), m_Decl(D) {}

    ~DeclStmt() override {
        delete m_Decl;
    }

    void accept(Visitor *V) override { V->visit(this); }

    Decl *getDecl() const { return m_Decl; }
};

class ExprStmt final : public Stmt {
    friend class CCGN;
    friend class NameResolution;
    friend class Sema;

    Expr *m_Expr;

public:
    ExprStmt(const Metadata &M, Expr *E) : Stmt(M), m_Expr(E) {}

    ~ExprStmt() override {
        delete m_Expr;
    }

    void accept(Visitor *V) override { V->visit(this); }

    Expr *getExpr() const { return m_Expr; }
};

class IfStmt final : public Stmt {
    friend class CCGN;
    friend class NameResolution;
    friend class Sema;

    Expr *m_Cond;
    Stmt *m_Then;
    Stmt *m_Else;

public:
    IfStmt(const Metadata &M, Expr *C, Stmt *T, Stmt *E)
      : Stmt(M), m_Cond(C), m_Then(T), m_Else(E) {}

    ~IfStmt() override {
        delete m_Cond;
        delete m_Then;
        delete m_Else;
    }

    void accept(Visitor *V) override { V->visit(this); }

    Expr *getCond() const { return m_Cond; }

    Stmt *getThen() const { return m_Then; }

    Stmt *getElse() const { return m_Else; }
};

class CaseStmt final : public Stmt {
    friend class CCGN;
    friend class NameResolution;
    friend class Sema;

    Expr *m_Pattern;
    Stmt *m_Body;

public:
    CaseStmt(const Metadata &M, Expr *P, Stmt *B) 
      : Stmt(M), m_Pattern(P), m_Body(B) {}

    ~CaseStmt() override {
        delete m_Pattern;
        delete m_Body;
    }
    
    void accept(Visitor *V) override { V->visit(this); }

    Expr *getPattern() const { return m_Pattern; }

    Stmt *getBody() const { return m_Body; }
};

class MatchStmt final : public Stmt {
    friend class CCGN;
    friend class NameResolution;
    friend class Sema;

    Expr *m_Pattern;
    std::vector<CaseStmt *> m_Cases;
    Stmt *m_Default;

public:
    MatchStmt(const Metadata &M, Expr *P, std::vector<CaseStmt *> C, Stmt *D)
      : Stmt(M), m_Pattern(P), m_Cases(C), m_Default(D) {}

    ~MatchStmt() override {
        delete m_Pattern;
        delete m_Default;
        for (auto &C : m_Cases)
            delete C;
    }

    void accept(Visitor *V) override { V->visit(this); }

    Expr *getPattern() const { return m_Pattern; }

    const std::vector<CaseStmt *> &getCases() const { return m_Cases; }

    Stmt *getDefault() const { return m_Default; }
};

class RetStmt final : public Stmt {
    friend class CCGN;
    friend class NameResolution;
    friend class Sema;
    
    Expr *m_Expr;

public:
    RetStmt(const Metadata &M, Expr *E) : Stmt(M), m_Expr(E) {}

    ~RetStmt() override {
        delete m_Expr;
    }

    void accept(Visitor *V) override { V->visit(this); }

    Expr *getExpr() const { return m_Expr; }
};

class UntilStmt final : public Stmt {
    friend class CCGN;
    friend class NameResolution;
    friend class Sema;

    Expr *m_Cond;
    Stmt *m_Body;

public:
    UntilStmt(const Metadata &M, Expr *C, Stmt *B) 
      : Stmt(M), m_Cond(C), m_Body(B) {}

    ~UntilStmt() override {
        delete m_Cond;
        delete m_Body;
    }

    void accept(Visitor *V) override { V->visit(this); }

    Expr *getCond() const { return m_Cond; }

    Stmt *getBody() const { return m_Body; }
};

} // namespace meddle

#endif // MEDDLE_STMT_H
