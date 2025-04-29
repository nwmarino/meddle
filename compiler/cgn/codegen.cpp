#include "codegen.h"
#include "../core/logger.h"
#include "../core/options.h"
#include "../tree/decl.h"
#include "../tree/stmt.h"
#include "../tree/unit.h"
#include "../mir/basicblock.h"
#include "../mir/builder.h"
#include "../mir/function.h"

using namespace meddle;

CGN::CGN(const Options &opts, TranslationUnit *U, mir::Segment *S) 
  : m_Opts(opts), m_Unit(U), m_Segment(S), m_Builder(S) {
    U->accept(this);
}

String CGN::mangle_name(NamedDecl *D) {
	auto it = m_Mangled.find(D);
	if (it != m_Mangled.end())
		return it->second;

	m_Mangled[D] = D->getName();
	return D->getName();
}

mir::Type *CGN::cgn_type(Type *T) {
    assert(T && "Cannot generate from a null type.");

	if (auto *PT = dynamic_cast<PrimitiveType *>(T)) {
		switch (PT->getKind()) {
		case PrimitiveType::Kind::Void:
			return nullptr;
		case PrimitiveType::Kind::Bool:
		case PrimitiveType::Kind::Char:
		case PrimitiveType::Kind::Int8:
		case PrimitiveType::Kind::UInt8:
			return m_Builder.get_i8_ty();
		case PrimitiveType::Kind::Int16:
		case PrimitiveType::Kind::UInt16:
			return m_Builder.get_i16_ty();
		case PrimitiveType::Kind::Int32:
		case PrimitiveType::Kind::UInt32:
			return m_Builder.get_i32_ty();
		case PrimitiveType::Kind::Int64:
		case PrimitiveType::Kind::UInt64:
			return m_Builder.get_i64_ty();
		case PrimitiveType::Kind::Float32:
			return m_Builder.get_f32_ty();
		case PrimitiveType::Kind::Float64:
			return m_Builder.get_f64_ty();
		}
	} else if (auto *AT = dynamic_cast<ArrayType *>(T)) {
		return mir::ArrayType::get(m_Segment, cgn_type(AT->getElement()), AT->getSize());
	} else if (auto *FT = dynamic_cast<FunctionType *>(T)) {
		std::vector<mir::Type *> params = {};
		params.reserve(FT->getNumParams());
		for (auto &P : FT->getParams())
			params.push_back(cgn_type(P));

		return mir::FunctionType::get(m_Segment, params, cgn_type(FT->getReturnType()));
	} else if (auto *PT = dynamic_cast<PointerType *>(T)) {
		return mir::PointerType::get(m_Segment, cgn_type(PT->getPointee()));
	}

	assert(false && "Unable to generate a type.");
}

void CGN::declare_function(FunctionDecl *FD) {
	mir::FunctionType *FT = static_cast<mir::FunctionType *>
		(cgn_type(FD->getType()));

	mir::Function::Linkage L = mir::Function::Linkage::Internal;
	mir::Function *FN = new mir::Function(mangle_name(FD), FT, L, m_Segment, {});

	// For every parameter in the function, create a new argument with the
	// lowered type. Also, build a slot node in the function for the parameter.
	std::vector<mir::Argument *> args = {};
	args.reserve(FD->getNumParams());
	for (unsigned i = 0, n = FD->getNumParams(); i != n; ++i) {
		ParamDecl *param = FD->getParam(i);

		mir::Type *ty = cgn_type(param->getType());
		mir::SlotNode *slot = m_Builder.build_slot(ty, param->getName());
		args.push_back(new mir::Argument(param->getName(), ty, FN, i, nullptr));
	}

	FN->set_args(args);
	m_Segment->add_function(FN);
}

void CGN::define_function(FunctionDecl *FD) {
	mir::Function *FN = m_Segment->get_function(mangle_name(FD));
	assert(FN && "Unable to find function in segment.");

	// Skip codegen for empty functions.
	if (FD->empty())
		return;

	// Create a new entry block for the function.
	mir::BasicBlock *entry = new mir::BasicBlock("entry", FN);
	m_Builder.set_insert(entry);

	// For each argument in the function, if it was given a slot node, store
	// the value of the argument to it in the beginning of the function.
	for (unsigned i = 0, n = FD->getNumParams(); i != n; ++i) {
		mir::Argument *arg = FN->get_arg(i);
		if (arg->get_slot())
			m_Builder.build_store(arg, arg->get_slot());
	}

	m_Function = FN;
	FD->getBody()->accept(this);

	// If the function's tail block does not terminate on its own, then insert
	// a return if the function is void. Otherwise, emit an error.
	if (!m_Builder.get_insert()->has_terminator()) {
		if (FD->getReturnType()->isVoid())
			m_Builder.build_ret_void();
		else {
			fatal("function does not return a value: " + FD->getName(), 
				  &FD->getMetadata());	
		}
	}

	m_Function = nullptr;
}

void CGN::visit(TranslationUnit *unit) {
	m_Phase = Phase::Declare;
	for (auto &D : unit->getDecls())
		D->accept(this);

	m_Phase = Phase::Define;
	for (auto &D : unit->getDecls())
		D->accept(this);
}

void CGN::visit(FunctionDecl *decl) {
	if (m_Phase == Phase::Define)
		define_function(decl);
	else if (m_Phase == Phase::Declare)
		declare_function(decl);
}

void CGN::visit(VarDecl *decl) {
	mir::Type *ty = cgn_type(decl->getType());

	if (decl->isGlobal() && m_Phase == Phase::Declare) {

	} else if (!decl->isGlobal()) {
		mir::SlotNode *slot = m_Builder.build_slot(ty, decl->getName());
		
		if (!decl->hasInit())
			return;

		m_VC = ValueContext::RValue;
		decl->getInit()->accept(this);
		assert(m_Value && "Variable init does not produce a value.");
		m_Builder.build_store(m_Value, slot);
	}
}

void CGN::visit(ParamDecl *decl) {}

void CGN::visit(BreakStmt *stmt) {

}

void CGN::visit(ContinueStmt *stmt) {

}

void CGN::visit(CompoundStmt *stmt) {
	for (auto &S : stmt->getStmts())
		S->accept(this);
}

void CGN::visit(DeclStmt *stmt) {
	stmt->getDecl()->accept(this);
}

void CGN::visit(ExprStmt *stmt) {
	stmt->getExpr()->accept(this);
}

void CGN::visit(IfStmt *stmt) {

}

void CGN::visit(CaseStmt *stmt) {

}

void CGN::visit(MatchStmt *stmt) {

}

void CGN::visit(RetStmt *stmt) {
	if (!stmt->getExpr()) {
		m_Builder.build_ret_void();
		return;
	}
	
	m_VC = ValueContext::RValue;
	stmt->getExpr()->accept(this);
	assert(m_Value && "Return expression does not produce a value.");
	m_Builder.build_ret(m_Value);
}

void CGN::visit(UntilStmt *stmt) {

}

void CGN::visit(IntegerLiteral *expr) {
	m_Value = new mir::ConstantInt(cgn_type(expr->getType()), expr->getValue());
}

void CGN::visit(FloatLiteral *expr) {
	m_Value = new mir::ConstantFP(cgn_type(expr->getType()), expr->getValue());
}

void CGN::visit(CharLiteral *expr) {
	m_Value = new mir::ConstantInt(m_Builder.get_i8_ty(), expr->getValue());
}

void CGN::visit(StringLiteral *expr) {

}

void CGN::visit(CastExpr *expr) {
	expr->getExpr()->accept(this);
}

void CGN::visit(RefExpr *expr) {

}
