#ifndef MEDDLE_STMT_H
#define MEDDLE_STMT_H

#include "expr.h"
#include "../core/metadata.h"

namespace meddle {

class Stmt {
protected:
    Metadata m_Metadata;

public:
    Stmt(const Metadata &M) : m_Metadata(M) {}
    virtual ~Stmt() = default;

    const Metadata &getMetadata() const { return m_Metadata; }
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

    Expr const *getExpr() const { return m_Expr; }
};

} // namespace meddle

#endif // MEDDLE_STMT_H
