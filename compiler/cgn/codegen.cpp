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
  : m_Opts(opts), m_Unit(U), m_Segment(S), m_Builder(mir::Builder(S)) {
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
		mir::Slot *slot = m_Builder.build_slot(ty, param->getName());
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
		mir::Slot *slot = m_Builder.build_slot(ty, decl->getName());
		
		if (!decl->hasInit())
			return;

		m_VC = ValueContext::RValue;
		decl->getInit()->accept(this);
		assert(m_Value && "Variable initializer does not produce a value.");

		mir::DataLayout DL = m_Segment->get_data_layout();
		bool isAggregate = !DL.is_scalar_ty(ty);
		unsigned size = DL.get_type_size(ty);

		if (size > DL.get_pointer_size() || isAggregate) {
			m_Builder.build_cpy(
				slot, 
				DL.get_type_align(ty), 
				m_Value, 
				DL.get_type_align(ty), 
				size
			);
		} else {
			m_Builder.build_store(m_Value, slot);
		}
	}
}

void CGN::visit(ParamDecl *decl) {}

void CGN::visit(BreakStmt *stmt) {
	m_Builder.build_jmp(m_Merge);
}

void CGN::visit(ContinueStmt *stmt) {
	m_Builder.build_jmp(m_Cond);
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
	m_VC = ValueContext::RValue;
	stmt->getCond()->accept(this);
	mir::Value *cond = m_Value;
	assert(cond && "'if' condition does not produce a value.");

	mir::BasicBlock *thenBB = new mir::BasicBlock("if.then", m_Function);
	mir::BasicBlock *mergeBB = new mir::BasicBlock("if.merge");
	mir::BasicBlock *elseBB = nullptr;

	if (stmt->hasElse()) {
		elseBB = new mir::BasicBlock("if.else");
		m_Builder.build_brif(cond, thenBB, elseBB);
	} else {
		m_Builder.build_brif(cond, thenBB, mergeBB);
	}

	m_Builder.set_insert(thenBB);
	stmt->getThen()->accept(this);
	if (!m_Builder.get_insert()->has_terminator())
		m_Builder.build_jmp(mergeBB);

	if (elseBB) {
		m_Function->append(elseBB);
		m_Builder.set_insert(elseBB);
		stmt->getElse()->accept(this);
		if (!m_Builder.get_insert()->has_terminator())
			m_Builder.build_jmp(mergeBB);
	}

	if (mergeBB->has_preds()) {
		m_Function->append(mergeBB);
		m_Builder.set_insert(mergeBB);
	} else {
		delete mergeBB;
	}
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
	mir::BasicBlock *condBB = new mir::BasicBlock("until.cond", m_Function);
	mir::BasicBlock *bodyBB = new mir::BasicBlock("until.body");
	mir::BasicBlock *mergeBB = new mir::BasicBlock("until.merge");
	mir::BasicBlock *oldCond = m_Cond;
	mir::BasicBlock *oldMerge = m_Merge;

	m_Cond = condBB;
	m_Merge = mergeBB;

    // Lower the while loop stop condition, and inject a comparison if needed.
    m_Builder.build_jmp(condBB);
    m_Builder.set_insert(condBB);
    m_VC = ValueContext::RValue;
    stmt->getCond()->accept(this);
    mir::Value *cond = m_Value;
    assert(cond && "'until' condition does not produce a value.");
    //cond = injectCMP(cond);
    m_Builder.build_brif(cond, mergeBB, bodyBB);

	m_Function->append(bodyBB);
    m_Builder.set_insert(bodyBB);
    stmt->getBody()->accept(this);

    if (!m_Builder.get_insert()->has_terminator()) {
        // If the loop body does not automatically terminate, then it should go
        // back to the condition for another potential iteration.
        m_Builder.build_jmp(condBB);
    }

    if (mergeBB->has_preds()) {
        // If the merge block is actually used, then insert it.
        m_Function->append(mergeBB);
        m_Builder.set_insert(mergeBB);
    }

	m_Cond = oldCond;
	m_Merge = oldMerge;
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
	mir::ConstantString *STR = new mir::ConstantString(
		cgn_type(expr->getType()), expr->getValue());

	m_Value = new mir::Data("str", mir::PointerType::get(m_Segment, 
		STR->get_type()), m_Segment, STR, 
		m_Segment->get_data_layout().get_type_align(STR->get_type()), true);
}

void CGN::visit(CastExpr *expr) {
	m_VC = ValueContext::RValue;
    expr->getExpr()->accept(this);
    mir::Value *srcV = m_Value;
    assert(srcV && "Cast source does not produce a value.");

    mir::Type *srcT = srcV->get_type();
    mir::Type *castT = cgn_type(expr->getType());
	mir::DataLayout DL = m_Segment->get_data_layout();

    if (srcT == castT) {
        m_Value = srcV;
        return;
    } else if (expr->getType()->isSInt() && srcT->is_integer_ty() && castT->is_integer_ty()) {
        unsigned srcWD = DL.get_type_size(srcT);
        unsigned castWD = DL.get_type_size(castT);

        if (srcWD == castWD)
            m_Value = srcV;
        else if (srcWD < castWD)
            m_Value = m_Builder.build_sext(srcV, castT, "cast.sext");
        else
            m_Value = m_Builder.build_trunc(srcV, castT, "cast.trunc");
    } else if (expr->getType()->isUInt( ) && srcT->is_integer_ty() && castT->is_integer_ty()) {
		unsigned srcWD = DL.get_type_size(srcT);
		unsigned castWD = DL.get_type_size(castT);

		if (srcWD == castWD)
			m_Value = srcV;
		else if (srcWD < castWD)
			m_Value = m_Builder.build_zext(srcV, castT, "cast.zext");
		else
			m_Value = m_Builder.build_trunc(srcV, castT, "cast.trunc");
	} else if (srcT->is_float_ty() && castT->is_float_ty()) {
        if (srcT->is_float_ty(64) && castT->is_float_ty(32))
            m_Value = m_Builder.build_ftrunc(srcV, castT, "cast.ftrunc");
        else
            m_Value = m_Builder.build_fext(srcV, castT, "cast.fext");
    } else if (srcT->is_integer_ty() && castT->is_float_ty()) {
        if (expr->getExpr()->getType()->isUInt())
            m_Value = m_Builder.build_ui2fp(srcV, castT, "cast.cvt");
        else
            m_Value = m_Builder.build_si2fp(srcV, castT, "cast.cvt");
    } else if (srcT->is_float_ty() && castT->is_integer_ty()) {
        if (expr->getType()->isUInt())
            m_Value = m_Builder.build_fp2ui(srcV, castT, "cast.cvt");
        else
            m_Value = m_Builder.build_fp2si(srcV, castT, "cast.cvt");
    }

	/*
    else if (srcT->isPointerTy() && castT->isPointerTy())
        tmp = IB.CreateBitCast(srcV, castT, "cast.ptr");

    else if (srcT->isPointerTy() && castT->isIntegerTy())
        tmp = IB.CreatePtrToInt(srcV, castT, "cast.ptr2int");

    else if (srcT->isIntegerTy() && castT->isPointerTy())
        tmp = IB.CreateIntToPtr(srcV, castT, "cast.int2ptr");
	*/

    else
		assert(false && "Unsupported cast.");
}

void CGN::visit(RefExpr *expr) {
	NamedDecl *ref = expr->getRef();
	mir::Slot *slot = m_Function->get_slot(expr->getName());
	assert(slot && "Slot does not exist in function.");
	
	if (m_VC == ValueContext::LValue)
		m_Value = slot;
	else {
		m_Value = m_Builder.build_load(cgn_type(expr->getType()), slot, 
			expr->getName() + ".val");
	}
}
