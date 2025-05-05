#ifndef MEDDLE_EXPR_H
#define MEDDLE_EXPR_H

#include "type.h"
#include "visitor.h"

#include <cassert>

namespace meddle {

class NamedDecl;
class TypeDecl;
class RefExpr;

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

	virtual bool isConstant() const { return false; }

	virtual bool isAggregateInit() const { return false; }
};

class BoolLiteral final : public Expr {
	friend class CGN;
	friend class NameResolution;
	friend class Sema;

	bool m_Value;

public:
	BoolLiteral(const Metadata &M, Type *T, bool V) : Expr(M, T), m_Value(V) {}

	void accept(Visitor *V) override { V->visit(this); }

	bool getValue() const { return m_Value; }

	bool isConstant() const override { return true; }
};

class IntegerLiteral final : public Expr {
    friend class CGN;
    friend class NameResolution;
    friend class Sema;
    
    long m_Value;

public:
    IntegerLiteral(const Metadata &M, Type *T, long V) 
      : Expr(M, T), m_Value(V) {}
	
    void accept(Visitor *V) override { V->visit(this); }
	
    long getValue() const { return m_Value; }

	bool isConstant() const override { return true; }
};

class FloatLiteral final : public Expr {
    friend class CGN;
	friend class NameResolution;
	friend class Sema;

	double m_Value;

public:
	FloatLiteral(const Metadata &M, Type *T, double V) 
	  : Expr(M, T), m_Value(V) {}

	void accept(Visitor *V) override { V->visit(this); }

	double getValue() const { return m_Value; }

	bool isConstant() const override { return true; }
};

class CharLiteral final : public Expr {
	friend class CGN;
	friend class NameResolution;
	friend class Sema;

	char m_Value;

public:
	CharLiteral(const Metadata &M, Type *T, char V)
	  : Expr(M, T), m_Value(V) {}

	void accept(Visitor *V) override { V->visit(this); }

	char getValue() const { return m_Value; }

	bool isConstant() const override { return true; }
};

class StringLiteral final : public Expr {
	friend class CGN;
	friend class NameResolution;
	friend class Sema;

	String m_Value;

public:
	StringLiteral(const Metadata &M, Type *T, String V)
	  : Expr(M, T), m_Value(V) {}

	void accept(Visitor *V) override { V->visit(this); }

	String getValue() const { return m_Value; }

	bool isConstant() const override { return true; }
};

class NilLiteral final : public Expr {
	friend class CGN;
	friend class NameResolution;
	friend class Sema;

public:
	NilLiteral(const Metadata &M, Type *T) : Expr(M, T) {}

	void accept(Visitor *V) override { V->visit(this); }

	bool isConstant() const override { return true; }
};

class ArrayExpr final : public Expr {
	friend class CGN;
	friend class NameResolution;
	friend class Sema;

	std::vector<Expr *> m_Elements;

public:
	ArrayExpr(const Metadata &M, Type *T, std::vector<Expr *> E)
	  : Expr(M, T), m_Elements(E) {}

	~ArrayExpr() override {
		for (auto &E : m_Elements)
			delete E;
	}

	void accept(Visitor *V) override { V->visit(this); }

	std::vector<Expr *> getElements() const { return m_Elements; }

	bool isConstant() const override {
		for (auto &E : m_Elements)
			if (!E->isConstant())
				return false;
			
		return true;
	}

	bool isAggregateInit() const override { return true; }
};

class BinaryExpr final : public Expr {
	friend class CGN;
	friend class NameResolution;
	friend class Sema;

public:
	enum class Kind {
		Unknown,
		
		Assign,
		Add_Assign,
		Sub_Assign,
		Mul_Assign,
		Div_Assign,
		Mod_Assign,
		And_Assign,
		Or_Assign,
		Xor_Assign,
		LeftShift_Assign,
		RightShift_Assign,

		Add,
		Sub,
		Mul,
		Div,
		Mod,
		Bitwise_And,
		Bitwise_Or,
		Bitwise_Xor,
		LeftShift,
		RightShift,
		Logic_And,
		Logic_Or,

		Equals,
		NEquals,
		LessThan,
		LessThanEquals,
		GreaterThan,
		GreaterThanEquals,
	};

	static bool supportsPtrArith(Kind K) {
		return K == Kind::Add || K == Kind::Sub ||
		       K == Kind::Add_Assign || K == Kind::Sub_Assign;
	}

private:
	Kind m_Kind;
	Expr *m_LHS;
	Expr *m_RHS;

public:
	BinaryExpr(const Metadata &M, Type *T, Kind K, Expr *LHS, Expr *RHS)
	  : Expr(M, T), m_Kind(K), m_LHS(LHS), m_RHS(RHS) {}

	~BinaryExpr() override {
		delete m_LHS;
		delete m_RHS;
	}

	void accept(Visitor *V) override { V->visit(this); }

	Kind getKind() const { return m_Kind; }

	Expr *getLHS() const { return m_LHS; }

	Expr *getRHS() const { return m_RHS; }

	bool isConstant() const override
	{ return m_LHS->isConstant() && m_RHS->isConstant(); }

	bool isComparison() const {
		switch (m_Kind) {
			case Kind::Equals:
			case Kind::NEquals:
			case Kind::LessThan:
			case Kind::LessThanEquals:
			case Kind::GreaterThan:
			case Kind::GreaterThanEquals:
				return true;
			default:
				return false;
		}
	}

	bool isLogicComparison() const 
	{ return m_Kind == Kind::Logic_And || m_Kind == Kind::Logic_Or; }

	bool isInequality() const {
		switch (m_Kind) {
			case Kind::LessThan:
			case Kind::LessThanEquals:
			case Kind::GreaterThan:
			case Kind::GreaterThanEquals:
				return true;
			default:
				return false;
		}
	}

	bool isAssignment() const {
		switch (m_Kind) {
			case Kind::Assign:
			case Kind::Add_Assign:
			case Kind::Sub_Assign:
			case Kind::Mul_Assign:
			case Kind::Div_Assign:
			case Kind::Mod_Assign:
			case Kind::And_Assign:
			case Kind::Or_Assign:
			case Kind::Xor_Assign:
			case Kind::LeftShift_Assign:
			case Kind::RightShift_Assign:
				return true;
			default:
				return false;
		}
	}

	bool isDirectAssignment() const { return m_Kind == Kind::Assign; }
};

class CastExpr final : public Expr {
	friend class CGN;
	friend class NameResolution;
	friend class Sema;

	Expr *m_Expr;

public:
	CastExpr(const Metadata &M, Type *T, Expr *E) : Expr(M, T), m_Expr(E) {}

	~CastExpr() override {
		delete m_Expr;
	}

	void accept(Visitor *V) override { V->visit(this); }

	Expr *getExpr() const { return m_Expr; }

	Type *getCast() const { return m_Type; }

	bool isConstant() const override { return m_Expr->isConstant(); }
};

class ParenExpr final : public Expr {
	friend class CGN;
	friend class NameResolution;
	friend class Sema;

	Expr *m_Expr;

public:
	ParenExpr(const Metadata &M, Expr *E) : Expr(M, E->getType()), m_Expr(E) {}

	~ParenExpr() override {
		delete m_Expr;
	}

	void accept(Visitor *V) override { V->visit(this); }

	Expr *getExpr() const { return m_Expr; }

	bool isConstant() const override { return m_Expr->isConstant(); }
};

class RefExpr : public Expr {
	friend class CGN;
	friend class NameResolution;
	friend class Sema;

protected:
	String m_Name;
	NamedDecl *m_Ref;

public:
	RefExpr(const Metadata &M, Type *T, String N, NamedDecl *R = nullptr)
	  : Expr(M, T, true), m_Name(N), m_Ref(R) {}

	void accept(Visitor *V) override { V->visit(this); }

	String getName() const { return m_Name;}

	NamedDecl *getRef() const { return m_Ref; }
};

class AccessExpr final : public RefExpr {
	friend class CGN;
	friend class NameResolution;
	friend class Sema;

	Expr *m_Base;

public:
	AccessExpr(const Metadata &M, Type *T, String N, Expr *B, NamedDecl *R = nullptr)
	  : RefExpr(M, T, N, R), m_Base(B) {}

	~AccessExpr() override {
		delete m_Base;
	}

	void accept(Visitor *V) override { V->visit(this); }

	Expr *getBase() const { return m_Base; }
};

class CallExpr : public RefExpr {
	friend class CGN;
	friend class NameResolution;
	friend class Sema;

protected:
	std::vector<Expr *> m_Args;

public:
	CallExpr(const Metadata &M, Type *T, const String &N, 
			 NamedDecl *C = nullptr, std::vector<Expr *> Args = {})
	  : RefExpr(M, T, N, C), m_Args(Args) {}

	~CallExpr() override {
		for (auto &A : m_Args)
			delete A;
	}

	void accept(Visitor *V) override { V->visit(this); }

	std::vector<Expr *> getArgs() const { return m_Args; }

	Expr *getArg(unsigned i) const {
		assert(i < m_Args.size() && "Index out of range.");
		return m_Args.at(i);
	}

	unsigned getNumArgs() const { return m_Args.size(); }

	FunctionDecl *getCallee() const;
};

class MethodCallExpr final : public CallExpr {
	friend class CGN;
	friend class NameResolution;
	friend class Sema;

	Expr *m_Base;

public:
	MethodCallExpr(const Metadata &M, Type *T, const String &N, Expr *B, 
				   NamedDecl *C = nullptr, std::vector<Expr *> Args = {})
	  : CallExpr(M, T, N, C, Args), m_Base(B) {}

	~MethodCallExpr() {
		delete m_Base;

		for (auto &A : m_Args)
			delete A;
	}

	void accept(Visitor *V) override { V->visit(this); }

	Expr *getBase() const { return m_Base; }
};

class FieldInitExpr final : public RefExpr {
	friend class CGN;
	friend class NameResolution;
	friend class Sema;

	Expr *m_Expr;

public:
	FieldInitExpr(const Metadata &M, Type *T, String N, Expr *E)
	  : RefExpr(M, T, N), m_Expr(E) {}

	~FieldInitExpr() override {
		delete m_Expr;
	}

	void accept(Visitor *V) override { V->visit(this); }

	Expr *getExpr() const { return m_Expr; }

	bool isConstant() const override { return m_Expr->isConstant(); }
};

class InitExpr final : public Expr {
	friend class CGN;
	friend class NameResolution;
	friend class Sema;

	std::vector<FieldInitExpr *> m_Fields;

public:
	InitExpr(const Metadata &M, Type *T, std::vector<FieldInitExpr *> F)
	  : Expr(M, T), m_Fields(F) {}

	~InitExpr() override {
		for (auto &F : m_Fields)
			delete F;
	}

	void accept(Visitor *V) override { V->visit(this); }

	std::vector<FieldInitExpr *> getFields() const { return m_Fields; }

	bool isConstant() const override {
		for (auto &F : m_Fields)
			if (!F->isConstant())
				return false;

		return true;
	}

	bool isAggregateInit() const override { return true; }
};

class SizeofExpr final : public Expr {
	friend class CGN;
	friend class NameResolution;
	friend class Sema;

	Type *m_Target;

public:
	SizeofExpr(const Metadata &M, Type *T, Type *Target)
	  : Expr(M, T), m_Target(Target) {}

	void accept(Visitor *V) override { V->visit(this); }

	Type *getTarget() const { return m_Target; }
};

class SubscriptExpr final : public Expr {
	friend class CGN;
	friend class NameResolution;
	friend class Sema;

	Expr *m_Base;
	Expr *m_Index;

public:
	SubscriptExpr(const Metadata &M, Type *T, Expr *B, Expr *I)
	  : Expr(M, T, true), m_Base(B), m_Index(I) {}

	~SubscriptExpr() override {
		delete m_Base;
		delete m_Index;
	}

	void accept(Visitor *V) override { V->visit(this); }

	Expr *getBase() const { return m_Base; }

	Expr *getIndex() const { return m_Index; }

	bool isConstant() const override
	{ return m_Base->isConstant() && m_Index->isConstant(); }
};

class TypeSpecExpr final : public RefExpr {
	RefExpr *m_Expr;

public:
	TypeSpecExpr(const Metadata &M, String N, RefExpr *E)
	  : RefExpr(M, nullptr, N), m_Expr(E) {}

	~TypeSpecExpr() override {
		delete m_Expr;
	}

	void accept(Visitor *V) override { V->visit(this); }

	RefExpr *getExpr() const { return m_Expr; }

	TypeDecl *getTypeDecl() const;
};

class UnaryExpr final : public Expr {
	friend class CGN;
	friend class NameResolution;
	friend class Sema;

public:
	enum class Kind {
		Unknown,

		Logic_Not,
		Bitwise_Not,
		Negate,
		Address_Of,
		Dereference,
		Increment,
		Decrement,
	};
	
	static bool isPrefixOp(Kind K) { 
		return K != Kind::Unknown;
	}

	static bool isPostfixOp(Kind K) {
		return K == Kind::Increment || K == Kind::Decrement;
	}

private:
	Kind m_Kind;
	Expr *m_Expr;
	bool m_Postfix;

public:
	UnaryExpr(const Metadata &M, Type *T, Kind K, Expr *E, bool POST)
	  : Expr(M, T, K == Kind::Dereference), m_Kind(K), m_Expr(E), m_Postfix(POST) {}

	~UnaryExpr() override {
		delete m_Expr;
	}

	void accept(Visitor *V) override { V->visit(this); }

	Kind getKind() const { return m_Kind; }

	Expr *getExpr() const { return m_Expr; }

	bool isConstant() const override { return m_Expr->isConstant(); }

	bool isPostfix() const { return m_Postfix; }

	bool isPrefix() const { return !m_Postfix; }

	bool isAddressOf() const { return m_Kind == Kind::Address_Of; }

	bool isDereference() const { return m_Kind == Kind::Dereference; }
};

} // namespace meddle

#endif // MEDDLE_EXPR_H
