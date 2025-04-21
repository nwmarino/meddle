#ifndef MEDDLE_STMT_H
#define MEDDLE_STMT_H

#include "expr.h"
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

    const Metadata &getMetadata() const { return m_Metadata; }
};

class CompoundStmt final : public Stmt {
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

    void addStmt(Stmt *S) { m_Stmts.push_back(S); }

    template<typename vT>
    void accept(vT &visitor) {
        visitor.visit(this);
    }

    const std::vector<Stmt *> &getStmts() const { return m_Stmts; }
};

class RetStmt final : public Stmt {
    Expr *m_Expr;

public:
    RetStmt(const Metadata &M, Expr *E) : Stmt(M), m_Expr(E) {}

    ~RetStmt() override {
        delete m_Expr;
    }

    template<typename vT>
    void accept(vT &visitor) {
        visitor.visit(this);
    }

    Expr *getExpr() const { return m_Expr; }
};

} // namespace meddle

#endif // MEDDLE_STMT_H
