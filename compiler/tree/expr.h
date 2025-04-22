#ifndef MEDDLE_EXPR_H
#define MEDDLE_EXPR_H

#include "nameres.h"
#include "type.h"
#include "visitor.h"

namespace meddle {

class NamedDecl;

class Expr {
protected:
    Metadata m_Metadata;
    Type *m_Type;
    bool m_IsLValue;

public:
    Expr(const Metadata &M, Type *T, bool LVal = false) 
      : m_Metadata(M), m_Type(T), m_IsLValue(LVal) {}
    virtual ~Expr() = default;

    virtual void accept(Visitor *V) = 0;

    const Metadata &getMetadata() const { return m_Metadata; }

    Type *getType() const { return m_Type; }

    void setType(Type *T) { m_Type = T; }

    bool isLValue() const { return m_IsLValue; }
};

class IntegerLiteral final : public Expr {
    friend class CCGN;
    friend class NameResolution;
    friend class Sema;
    
    long m_Value;

public:
    IntegerLiteral(const Metadata &M, Type *T, long V) 
      : Expr(M, T), m_Value(V) {}
	
    void accept(Visitor *V) override { V->visit(this); }
	
    long getValue() const { return m_Value; }
};

class RefExpr final : public Expr {
	friend class CCGN;
	friend class NameResolution;
	friend class Sema;

	String m_Name;
	NamedDecl *m_Ref;

public:
	RefExpr(const Metadata &M, Type *T, String N, NamedDecl *R = nullptr)
	  : Expr(M, T, true), m_Name(N), m_Ref(R) {}

	void accept(Visitor *V) override { V->visit(this); }

	String getName() const { return m_Name;}

	NamedDecl *getRef() const { return m_Ref; }
};

} // namespace meddle

#endif // MEDDLE_EXPR_H
