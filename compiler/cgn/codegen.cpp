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
			m_Opts.NamedMIR ? "ptr.cmp" : ""
		);
	}

    if (V->get_type()->is_integer_ty()) {
        return m_Builder.build_icmp_ne(
            V,
            mir::ConstantInt::get(m_Segment, V->get_type(), 0),
            m_Opts.NamedMIR ? "int.cmp" : ""
        );
    }

    if (V->get_type()->is_float_ty()) {
        return m_Builder.build_fcmp_one(
			V,
            mir::ConstantFP::get(m_Segment, V->get_type(), 0.0),
            m_Opts.NamedMIR ? "fp.cmp" : ""
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
		return mir::ArrayType::get(m_Segment, cgn_type(AT->getElement()), 
			AT->getSize());
	} else if (auto *FT = dynamic_cast<FunctionType *>(T)) {
		std::vector<mir::Type *> params = {};
		params.reserve(FT->getNumParams());
		for (auto &P : FT->getParams())
			params.push_back(cgn_type(P));

		return mir::FunctionType::get(m_Segment, params, 
				cgn_type(FT->getReturnType()));
	} else if (auto *PT = dynamic_cast<PointerType *>(T)) {
		return mir::PointerType::get(m_Segment, cgn_type(PT->getPointee()));
	}

	assert(false && "Unable to generate a type.");
}

CGN::TypeClass CGN::type_class(Type *T) const {
	if (T->isSInt())
		return TypeClass::SInt;
	else if (T->isUInt())
		return TypeClass::UInt;
	else if (T->isFloat())
		return TypeClass::Float;
	else if (T->isPointer())
		return TypeClass::Pointer;
	else if (T->isArray())
		return TypeClass::Aggregate;
	else
		return TypeClass::Unknown;
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
}

void CGN::define_function(FunctionDecl *FD) {
	mir::Function *FN = m_Segment->get_function(mangle_name(FD));
	assert(FN && "Unable to find function in segment.");

	// Skip codegen for empty functions.
	if (FD->empty())
		return;

	// Create a new entry block for the function.
	mir::BasicBlock *entry = new mir::BasicBlock(
		m_Opts.NamedMIR ? "entry" : "", FN);
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
	mir::DataLayout DL = m_Segment->get_data_layout();

	if (decl->isGlobal() && m_Phase == Phase::Declare) {
		mir::Data::Linkage L = mir::Data::Linkage::Internal;

		m_VC = ValueContext::RValue;
		decl->getInit()->accept(this);
		assert(m_Value && "Variable initializer does not produce a value.");
		mir::Constant *C = static_cast<mir::Constant *>(m_Value);

		new mir::Data(
			mangle_name(decl), 
			ty, 
			L, 
			m_Segment, 
			C,
			DL.get_type_align(ty),
			!decl->isMutable()
		);
	} else if (!decl->isGlobal()) {
		mir::Slot *slot = m_Builder.build_slot(ty, decl->getName());
		
		if (!decl->hasInit())
			return;

		m_VC = ValueContext::RValue;
		decl->getInit()->accept(this);
		assert(m_Value && "Variable initializer does not produce a value.");

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

	mir::BasicBlock *thenBB = new mir::BasicBlock( 
		m_Opts.NamedMIR ? "if.then" : "", m_Function);
	mir::BasicBlock *mergeBB = new mir::BasicBlock(
		m_Opts.NamedMIR ? "if.merge" : "");
	mir::BasicBlock *elseBB = nullptr;

	if (stmt->hasElse()) {
		elseBB = new mir::BasicBlock(m_Opts.NamedMIR ? "if.else" : "");
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
    mir::BasicBlock *mergeBB = new mir::BasicBlock(
		m_Opts.NamedMIR ? "match.merge" : "");
    mir::BasicBlock *defBB = nullptr;
    if (stmt->getDefault())
        defBB = new mir::BasicBlock(m_Opts.NamedMIR ? "match.def" : "");

    // Create a "chain" block for every case in the statement.
    //
    // This should ideally be optimized instead with a jump table, but that
    // can come later...
	const std::vector<CaseStmt *> cases = stmt->getCases();
    std::vector<mir::BasicBlock *> chains;
    for (auto &C : cases)
        chains.push_back(new mir::BasicBlock(m_Opts.NamedMIR ? "match.chain" : ""));

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
		String cmpName = m_Opts.NamedMIR ? "match.cmp" : "";
		switch (type_class(stmt->getPattern()->getType())) {
		case TypeClass::SInt:
		case TypeClass::UInt:
			cmpV = m_Builder.build_icmp_eq(matchV, patternV, cmpName);
			break;
		case TypeClass::Float:
			cmpV = m_Builder.build_fcmp_oeq(matchV, patternV, cmpName);
			break;
		case TypeClass::Pointer:
			cmpV = m_Builder.build_pcmp_eq(matchV, patternV, cmpName);
			break;
		default:
			fatal("unsupported 'match' pattern type", 
				&stmt->getMetadata());
			break;
		}

        mir::BasicBlock *body = new mir::BasicBlock( 
			m_Opts.NamedMIR ? "match.case" : "", m_Function);

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
	assert(m_Value && "'ret' expression does not produce a value.");
	m_Builder.build_ret(m_Value);
}

void CGN::visit(UntilStmt *stmt) {
	mir::BasicBlock *condBB = new mir::BasicBlock( 
		m_Opts.NamedMIR ? "until.cond" : "", m_Function);
	mir::BasicBlock *bodyBB = new mir::BasicBlock(
		m_Opts.NamedMIR ? "until.body" : "");
	mir::BasicBlock *mergeBB = new mir::BasicBlock(
		m_Opts.NamedMIR ? "until.merge" : "");
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

void CGN::visit(BoolLiteral *expr) {
	m_Value = mir::ConstantInt::get(m_Segment, cgn_type(expr->getType()), 
		expr->getValue());
}

void CGN::visit(IntegerLiteral *expr) {
	m_Value = mir::ConstantInt::get(m_Segment, cgn_type(expr->getType()), 
		expr->getValue());
}

void CGN::visit(FloatLiteral *expr) {
	m_Value = mir::ConstantFP::get(m_Segment, cgn_type(expr->getType()), 
		expr->getValue());
}

void CGN::visit(CharLiteral *expr) {
	m_Value = mir::ConstantInt::get(m_Segment, m_Builder.get_i8_ty(), 
		expr->getValue());
}

void CGN::visit(StringLiteral *expr) {
	mir::ConstantString *STR = new mir::ConstantString(
		cgn_type(expr->getType()), expr->getValue());

	m_Value = new mir::Data(
		"__const.str", // name
		mir::PointerType::get(m_Segment, STR->get_type()), // data pointer type 
		mir::Data::Linkage::Internal, // linkage 
		m_Segment, // parent
		STR, // value
		m_Segment->get_data_layout().get_type_align(STR->get_type()), // align
		true // readonly?
	);
}

void CGN::visit(NilLiteral *expr) {
	m_Value = mir::ConstantNil::get(m_Segment, cgn_type(expr->getType()));
}

void CGN::visit(BinaryExpr *expr) {
	switch (expr->getKind()) {
	case BinaryExpr::Kind::Unknown: assert(false && "Unknown binary operator.");
	case BinaryExpr::Kind::Assign: return cgn_assign(expr);
	case BinaryExpr::Kind::Add_Assign: return cgn_add_assign(expr);
	case BinaryExpr::Kind::Sub_Assign: return cgn_sub_assign(expr);
	case BinaryExpr::Kind::Mul_Assign: return cgn_mul_assign(expr);
	case BinaryExpr::Kind::Div_Assign: return cgn_div_assign(expr);
	case BinaryExpr::Kind::Mod_Assign: return cgn_mod_assign(expr);
	case BinaryExpr::Kind::And_Assign: return cgn_and_assign(expr);
	case BinaryExpr::Kind::Or_Assign: return cgn_or_assign(expr);
	case BinaryExpr::Kind::Xor_Assign: return cgn_xor_assign(expr);
	case BinaryExpr::Kind::LeftShift_Assign: return cgn_shl_assign(expr);
	case BinaryExpr::Kind::RightShift_Assign: return cgn_shr_assign(expr);
	case BinaryExpr::Kind::Add: return cgn_add(expr);
	case BinaryExpr::Kind::Sub: return cgn_sub(expr);
	case BinaryExpr::Kind::Mul: return cgn_mul(expr);
	case BinaryExpr::Kind::Div: return cgn_div(expr);
	case BinaryExpr::Kind::Mod: return cgn_mod(expr);
	case BinaryExpr::Kind::Bitwise_And: return cgn_and(expr);
	case BinaryExpr::Kind::Bitwise_Or: return cgn_or(expr);
	case BinaryExpr::Kind::Bitwise_Xor: return cgn_xor(expr);
	case BinaryExpr::Kind::LeftShift: return cgn_shl(expr);
	case BinaryExpr::Kind::RightShift: return cgn_shr(expr);
	case BinaryExpr::Kind::Logic_And: return cgn_logic_and(expr);
	case BinaryExpr::Kind::Logic_Or: return cgn_logic_or(expr);
	case BinaryExpr::Kind::Equals: return cgn_equals(expr);
	case BinaryExpr::Kind::NEquals: return cgn_not_equals(expr);
	case BinaryExpr::Kind::LessThan: return cgn_less(expr);
	case BinaryExpr::Kind::LessThanEquals: return cgn_less_eq(expr);
	case BinaryExpr::Kind::GreaterThan: return cgn_greater(expr);
	case BinaryExpr::Kind::GreaterThanEquals: return cgn_greater_eq(expr);
    }
}

void CGN::visit(CastExpr *expr) {
	m_VC = ValueContext::RValue;
    expr->getExpr()->accept(this);
    assert(m_Value&& "Cast source does not produce a value.");

    Type *srcTy = expr->getExpr()->getType();
    Type *destTy = expr->getType();
	TypeClass srcCls = type_class(srcTy);
	TypeClass destCls = type_class(destTy);
	mir::Type *srcTyIR = cgn_type(srcTy);
	mir::Type *destTyIR = cgn_type(destTy);
	mir::Value *V = m_Value;
	mir::DataLayout DL = m_Segment->get_data_layout();
	String name;

    if (srcTyIR == destTyIR) {
        m_Value = V;
		return;
	}

	unsigned srcWidth = DL.get_type_size(srcTyIR);
	unsigned destWidth = DL.get_type_size(destTyIR);
    
	if ((srcCls == SInt | srcCls == UInt) && (destCls == SInt | destCls == UInt)) {
		if (srcWidth == destWidth) {
			m_Value = V;
			return;
		}

		if (srcWidth > destWidth) {
			// Downcasting.
			name = m_Opts.NamedMIR ? "cast.trunc" : "";
			m_Value = m_Builder.build_trunc(V, destTyIR, name);
		} else {
			// Upcasting.
			name = m_Opts.NamedMIR ? "cast.ext" : "";

			if (srcCls == SInt)
				m_Value = m_Builder.build_sext(V, destTyIR, name);
			else
				m_Value = m_Builder.build_zext(V, destTyIR, name);
		}
	} 
	
	else if (srcCls == Float && destCls == Float) {
		// Floating point cast.
		if (srcWidth == destWidth) {
			m_Value = V;
			return;
		}

		if (srcWidth > destWidth) {
			// Downcasting.
			name = m_Opts.NamedMIR ? "cast.ftrunc" : "";
			m_Value = m_Builder.build_ftrunc(V, destTyIR, name);
		} else {
			// Upcasting.
			name = m_Opts.NamedMIR ? "cast.fext" : "";
			m_Value = m_Builder.build_fext(V, destTyIR, name);
		}
	}

	else if (srcCls == SInt && destCls == Float) {
		// Signed integer to floating point.
		name = m_Opts.NamedMIR ? "cast.cvt" : "";
		m_Value = m_Builder.build_si2fp(V, destTyIR, name);
	}

	else if (srcCls == UInt && destCls == Float) {
		// Unsigned integer to floating point.
		name = m_Opts.NamedMIR ? "cast.cvt" : "";
		m_Value = m_Builder.build_ui2fp(V, destTyIR, name);
	}

	else if (srcCls == Float && destCls == SInt) {
		// Floating point to signed integer.
		name = m_Opts.NamedMIR ? "cast.cvt" : "";
		m_Value = m_Builder.build_fp2si(V, destTyIR, name);
	}

	else if (srcCls == Float && destCls == UInt) {
		// Floating point to unsigned integer.
		name = m_Opts.NamedMIR ? "cast.cvt" : "";
		m_Value = m_Builder.build_fp2ui(V, destTyIR, name);
	}

	else if (srcCls == Pointer && destCls == Pointer) {
		// Pointer reinterpretation.
		name = m_Opts.NamedMIR ? "cast.ptr" : "";
		m_Value = m_Builder.build_reint(V, destTyIR, name);
	}

	else if (srcCls == Pointer && (destCls == SInt | destCls == UInt)) {
		// Pointer to integer cast.
		name = m_Opts.NamedMIR ? "cast.ptr" : "";
		m_Value = m_Builder.build_ptr2int(V, destTyIR, name);
	}

	else if ((srcCls == SInt | srcCls == UInt) && destCls == Pointer) {
		// Integer to pointer cast.
		name = m_Opts.NamedMIR ? "cast.ptr" : "";
		m_Value = m_Builder.build_int2ptr(V, destTyIR, name);
	} 
	
	else {
		fatal("invalid cast from type '" + srcTy->getName() + "' to '" + 
			destTy->getName(), &expr->getMetadata());
	}
}

void CGN::visit(ParenExpr *expr) {
	expr->getExpr()->accept(this);
}

void CGN::visit(RefExpr *expr) {
	NamedDecl *ref = expr->getRef();
	mir::Slot *slot = m_Function->get_slot(expr->getName());
	assert(slot && "Slot does not exist in function.");
	
	if (m_VC == ValueContext::LValue)
		m_Value = slot;
	else {
		m_Value = m_Builder.build_load(cgn_type(expr->getType()), slot, 
			m_Opts.NamedMIR ? expr->getName() + ".val" : "");
	}
}

void CGN::visit(SizeofExpr *expr) {
	mir::DataLayout DL = m_Segment->get_data_layout();
	mir::Type *T = cgn_type(expr->getTarget());
	
	m_Value = mir::ConstantInt::get(m_Segment, cgn_type(expr->getType()), 
		DL.get_type_size(T));
}

void CGN::visit(UnaryExpr *expr) {
	switch (expr->getKind()) {
	case UnaryExpr::Kind::Logic_Not: 
	case UnaryExpr::Kind::Bitwise_Not:
	case UnaryExpr::Kind::Negate:
	case UnaryExpr::Kind::Address_Of:
	case UnaryExpr::Kind::Dereference:
	case UnaryExpr::Kind::Increment:
	case UnaryExpr::Kind::Decrement:
	default: assert(false && "Unknown unary operator.");
    }
}
