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

mir::Value *CGN::inject_cmp(mir::Value *V) {
	if (V->get_type()->is_integer_ty(1))
        return V;

    if (V->get_type()->is_pointer_ty()) {
		return m_Builder.build_pcmp_ne(
			V,
			mir::ConstantNil::get(m_Segment, V->get_type()),
			"ptr.cmp"
		);
	}

    if (V->get_type()->is_integer_ty()) {
        return m_Builder.build_icmp_ne(
            V,
            mir::ConstantInt::get(m_Segment, V->get_type(), 0),
            "int.cmp"
        );
    }

    if (V->get_type()->is_float_ty()) {
        return m_Builder.build_fcmp_one(
			V,
            mir::ConstantFP::get(m_Segment, V->get_type(), 0.0),
            "fp.cmp"
        );
    }

    assert(false && "Unsupported conditional value.");
}

mir::Type *CGN::cgn_type(Type *T) {
    assert(T && "Cannot generate from a null type.");

	if (auto *PT = dynamic_cast<PrimitiveType *>(T)) {
		switch (PT->getKind()) {
		case PrimitiveType::Kind::Void:
			return m_Builder.get_void_ty();
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
	cond = inject_cmp(cond);

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

void CGN::visit(CaseStmt *stmt) {}

void CGN::visit(MatchStmt *stmt) {
	// Lower the match expression as an rvalue.
    m_VC = ValueContext::RValue;
    stmt->getPattern()->accept(this);
    mir::Value *matchV = m_Value;
    assert(matchV && "'match' expression does not produce a value.");

    // Create a merge block, without inserting it since it should come last.
    mir::BasicBlock *mergeBB = new mir::BasicBlock("match.merge");
    mir::BasicBlock *defBB = nullptr;
    if (stmt->getDefault())
        defBB = new mir::BasicBlock("match.def");

    // Create a "chain" block for every case in the statement.
    //
    // This should ideally be optimized instead with a jump table, but that
    // can come later...
	const std::vector<CaseStmt *> cases = stmt->getCases();
    std::vector<mir::BasicBlock *> chains;
    for (auto &C : cases)
        chains.push_back(new mir::BasicBlock("match.chain"));

    // Begin at the first case.
    assert(chains.size() > 0 && "'match' statement has no cases.");
    m_Builder.build_jmp(chains[0]);

    // For each case in the statement, lower its pattern value in the chain
    // block, and try to compare it to the match value.
    //
    // If the comparison succeeds, control flow is passed to the body of the 
    // case. Otherwise, it goes to either the next chain block, the default
    // block (if it exists), or the merge block if there are no more chains.
    for (unsigned i = 0, n = chains.size(); i != n; ++i) {
        CaseStmt *C = cases.at(i);
        mir::BasicBlock *chain = chains[i];

		m_Function->append(chain);
        m_Builder.set_insert(chain);

        // Lower the case pattern value.
        C->getPattern()->accept(this);
        assert(m_Value && "Case pattern does not produce a value.");
        mir::Value *patternV = m_Value;

        // Compare the pattern value with main match expression.
        mir::Value *cmpV = nullptr;
		if (matchV->get_type()->is_integer_ty()) {
			cmpV = m_Builder.build_icmp_eq(matchV, patternV, "match.cmp");
        } else if (matchV->get_type()->is_float_ty()) {
            cmpV = m_Builder.build_fcmp_oeq(matchV, patternV, "match.cmp");
		} else if (matchV->get_type()->is_pointer_ty()) {
			cmpV = m_Builder.build_pcmp_eq(matchV, patternV, "match.cmp");
		}

        mir::BasicBlock *body = new mir::BasicBlock("match.case", m_Function);

        // Branch to the case block if the comparison succeeded, otherwise
        // the next chain if there is one, the default if there is one, or the
        // merge block if no options exist.
        if (i + 1 != n)
            m_Builder.build_brif(cmpV, body, chains[i + 1]);
        else if (stmt->getDefault())
            m_Builder.build_brif(cmpV, body, defBB);
        else
            m_Builder.build_brif(cmpV, body, mergeBB);

        m_Builder.set_insert(body);
        C->getBody()->accept(this);

        if (!m_Builder.get_insert()->has_terminator()) {
            // If the body of the case does not terminate on its own, then the
            // case must branch to the merge block.
            m_Builder.build_jmp(mergeBB);
        }
    }
    
    // Pass over the default body, if it exists.
    if (stmt->getDefault()) {
		m_Function->append(defBB);
        m_Builder.set_insert(defBB);
        stmt->getDefault()->accept(this);
        if (!m_Builder.get_insert()->has_terminator()) {
            // Similar to the case bodies, control flow goes to merge if the
            // block does not terminate on its own.
            m_Builder.build_jmp(mergeBB);
        }
    }

    if (mergeBB->has_preds()) {
        // If the merge block is actually used, then emit it.
        m_Function->append(mergeBB);
		m_Builder.set_insert(mergeBB);
    } else
		delete mergeBB;
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
    cond = inject_cmp(cond);
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
	m_Value = mir::ConstantInt::get(m_Segment, cgn_type(expr->getType()), expr->getValue());
}

void CGN::visit(FloatLiteral *expr) {
	m_Value = mir::ConstantFP::get(m_Segment, cgn_type(expr->getType()), expr->getValue());
}

void CGN::visit(CharLiteral *expr) {
	m_Value = mir::ConstantInt::get(m_Segment, m_Builder.get_i8_ty(), expr->getValue());
}

void CGN::visit(StringLiteral *expr) {
	mir::ConstantString *STR = new mir::ConstantString(
		cgn_type(expr->getType()), expr->getValue());

	m_Value = new mir::Data("str", mir::PointerType::get(m_Segment, 
		STR->get_type()), m_Segment, STR, 
		m_Segment->get_data_layout().get_type_align(STR->get_type()), true);
}

void CGN::visit(NilLiteral *expr) {
	m_Value = mir::ConstantNil::get(m_Segment, cgn_type(expr->getType()));
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
	} else if (srcT->is_pointer_ty() && castT->is_pointer_ty()) {
		m_Value = m_Builder.build_reint(srcV, castT, "cast.ptr");
    } else if (srcT->is_pointer_ty() && castT->is_integer_ty()) {
        m_Value = m_Builder.build_ptr2int(srcV, castT, "cast.ptr");
	} else if (srcT->is_integer_ty() && castT->is_pointer_ty()) {
        m_Value = m_Builder.build_int2ptr(srcV, castT, "cast.ptr");
    } else {
		assert(false && "Unsupported cast.");
	}
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
