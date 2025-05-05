#include "codegen.h"
#include "../core/logger.h"
#include "../core/options.h"
#include "../tree/decl.h"
#include "../tree/stmt.h"
#include "../tree/unit.h"
#include "../mir/basicblock.h"
#include "../mir/builder.h"
#include "../mir/function.h"
#include <cassert>

using namespace meddle;

CGN::CGN(const Options &opts, TranslationUnit *U, mir::Segment *S) 
  : m_Opts(opts), m_Unit(U), m_Segment(S), m_Builder(mir::Builder(S)) {
    U->accept(this);
}

String CGN::mangle_name(NamedDecl *D) {
	auto it = m_Mangled.find(D);
	if (it != m_Mangled.end())
		return it->second;

	String mangled;

	if (auto *F = dynamic_cast<FunctionDecl *>(D)) {
		if (F->hasParent()) {
			mangled = F->getParent()->getName() + "." + F->getName();
		} else {
			mangled = F->getName();
		}
	} else {
		mangled = D->getName();
	}

	m_Mangled[D] = mangled;
	return mangled;
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
			return m_Builder.get_i1_ty();
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
		mir::Type *retTy = nullptr;
		std::vector<mir::Type *> params = {};
		params.reserve(FT->getNumParams());

		if (FT->getReturnType()->isArray())
			retTy = m_Builder.get_void_ty();
		else
			retTy = cgn_type(FT->getReturnType());
		
		for (auto &P : FT->getParams()) {
			if (P->isArray())
				params.push_back(mir::PointerType::get(m_Segment, cgn_type(P)));
			else
				params.push_back(cgn_type(P));
		}

		return mir::FunctionType::get(m_Segment, params, retTy);
	} else if (auto *PT = dynamic_cast<PointerType *>(T)) {
		return mir::PointerType::get(m_Segment, cgn_type(PT->getPointee()));
	} else if (auto *ET = dynamic_cast<EnumType *>(T)) {
		return cgn_type(ET->getUnderlying());
	} else if (auto *ST = dynamic_cast<StructType *>(T)) {
		mir::StructType *mirTy = mir::StructType::get(m_Segment, ST->getName());
		assert(mirTy && "Struct type not lowered.");
		return mirTy;
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
	// Lowering of function types have special behavior: aggregate parameters
	// are wrapped in pointers and return types are converted to void.
	mir::FunctionType *FT = static_cast<mir::FunctionType *>
		(cgn_type(FD->getType()));

	mir::Function::Linkage L = mir::Function::Linkage::Internal;
	mir::Function *FN = new mir::Function(mangle_name(FD), FT, L, m_Segment, {});

	std::vector<mir::Argument *> args;
	args.reserve(FD->getNumParams());

	Type *retTy = FD->getReturnType();
	bool isAggregateRet = retTy->isArray(); // || retTy->isStruct();
	if (isAggregateRet) {
		assert(FN->get_return_ty()->is_void_ty() 
			&& "Function returns aggregate, but MIR function not void.");

		// Aggregate return types are passed via pointer with the `ARet`
		// attribute on the first parameter, which we implicitly inject here.
		mir::Argument *aret = new mir::Argument(
			m_Opts.NamedMIR ? "aret.ptr" : m_Segment->get_ssa(),
			mir::PointerType::get(m_Segment, cgn_type(FD->getReturnType())),
			FN,
			0,
			nullptr // No slot, rets should copy themselves to this argument.
		);
		aret->add_attribute(mir::Attribute::ARet);
		args.push_back(aret);
	}

	// For every parameter in the function, create a new argument with the
	// lowered type. Also, build a slot node in the function for the parameter.
	for (unsigned i = 0; i != FD->getNumParams(); ++i) {
		ParamDecl *P = FD->getParam(i);
		Type *paramTy = P->getType();

		mir::Type *ty = FT->get_param_type(i);
		mir::Slot *slot = nullptr;
		mir::Argument *arg = new mir::Argument(P->getName(), ty, FN, 
			args.size(), slot);

		bool isAggregateParam = paramTy->isArray(); // || paramTy->isStruct();
		if (isAggregateParam) {
			// Aggregate parameters are passed via pointer with the `AParam`
			// attribute on the given parameter.
			//
			// Later, during actual code generation for this function, a copy
			// is made from the passed in argument to the created slot.
			arg->add_attribute(mir::Attribute::AArg);
			mir::PointerType *PT = static_cast<mir::PointerType *>(ty);
			slot = m_Builder.build_slot(PT->get_pointee(), P->getName(), FN);
		} else {
			slot = m_Builder.build_slot(ty, P->getName(), FN);
		}

		arg->set_slot(slot);
		args.push_back(arg);
	}

	FN->set_args(args);
}

void CGN::define_function(FunctionDecl *FD) {
	mir::Function *FN = m_Segment->get_function(mangle_name(FD));
	assert(FN && "Unable to find function in segment.");

	// Skip codegen for empty functions.
	// TODO: Also stop here for imported functions.
	if (FD->empty())
		return;

	// Create a new entry block for the function.
	mir::BasicBlock *entry = new mir::BasicBlock(
		m_Opts.NamedMIR ? "entry" : "", FN);
	m_Builder.set_insert(entry);

	// For each argument in the function, if it was given a slot node, store
	// the value of the argument to it in the beginning of the function.
	for (unsigned i = 0, n = FN->get_args().size(); i != n; ++i) {
		mir::Argument *arg = FN->get_arg(i);
		if (arg->hasARetAttribute()) {
			// The aggregate return argument should not be moved.
			continue;
		} else if (arg->hasAArgAttribute()) {
			// Aggregate arguments passed via pointer need to be copied to their
			// destination slot.
			mir::Slot *slot = arg->get_slot();
			assert(slot && "Slot does not exist for argument with AArg attribute.");

			mir::DataLayout DL = m_Segment->get_data_layout();
			mir::PointerType *PT = static_cast<mir::PointerType *>(arg->get_type());
			mir::Type *ty = PT->get_pointee();

			m_Builder.build_cpy(
				slot, 
				DL.get_type_align(ty), 
				arg, 
				DL.get_type_align(ty), 
				mir::ConstantInt::get(
					m_Segment, m_Builder.get_i64_ty(), DL.get_type_size(ty)
				)
			);
		} else if (arg->get_slot() != nullptr) {
			// We assume that the lowered function is correct and any arguments
			// without the `AArg` attribute are scalar and can be moved with a
			// store.
			m_Builder.build_store(arg, arg->get_slot());
		}
	}

	if (FD->isMethod()) {
		assert(FN->get_args().size() > 0 && "Method has no arguments.");
		m_Self = FN->get_arg(0)->get_slot();
	}

	m_Function = FN;
	FD->getBody()->accept(this);

	// If the function's tail block does not terminate on its own, then insert
	// a return if the function is void. Otherwise, emit an error.
	if (!m_Builder.get_insert()->has_terminator()) {
		if (FN->get_return_ty()->is_void_ty())
			m_Builder.build_ret_void();
		else {
			fatal("function does not return a value: " + FD->getName(), 
				  &FD->getMetadata());	
		}
	}

	m_Function = nullptr;
	m_Self = nullptr;
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

		if (decl->getInit()->isAggregateInit()) {
			// The initializer is some aggregate initializer e.g. struct, array.
			// It will be responsible for copying itself to the slot/place.
			m_VC = ValueContext::RValue;
			m_Place = slot;
			decl->getInit()->accept(this);
			m_Place = nullptr;
		} else if (DL.is_scalar_ty(ty)) {
			// The initializer is a scalar value which may be passed via regs.
			m_VC = ValueContext::RValue;
			decl->getInit()->accept(this);
			assert(m_Value && "Variable initializer does not produce a value.");

			m_Builder.build_store(m_Value, slot);
		} else {
			// The initializer is some non-scalar (aggregate) that is our
			// responsibility to copy over to the newly created slot.
			m_VC = ValueContext::LValue;
			m_Place = slot;
			decl->getInit()->accept(this);
			assert(m_Value && "Variable initializer does not produce a value.");
			m_Place = nullptr;

			unsigned size = DL.get_type_size(ty);
			unsigned align = DL.get_type_align(ty);

			if (m_Value != slot) {
				m_Builder.build_cpy(
					slot, 
					align, 
					m_Value, 
					align, 
					mir::ConstantInt::get(
						m_Segment, m_Builder.get_i64_ty(), size
					)
				);
			}
			
			m_Value = nullptr;
		}
	}
}

void CGN::visit(StructDecl *decl) {
	if (m_Phase == Phase::Declare) {
		std::vector<mir::Type *> memberTys;

		for (auto &F : decl->getFields())
			memberTys.push_back(cgn_type(F->getType()));

		mir::StructType *ST = mir::StructType::create(
			m_Segment, decl->getName(), memberTys);
	}
	
	for (auto &F : decl->getFunctions())
		F->accept(this);
}

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

	mir::Type *ty = cgn_type(stmt->getExpr()->getType());
	mir::DataLayout DL = m_Segment->get_data_layout();

	if (stmt->getExpr()->isAggregateInit()) {
		assert(m_Function->hasARetAttribute() && 
			"Return type is an aggregate, but function has no ARet.");

		m_VC = ValueContext::RValue;
		m_Place = m_Function->get_arg(0);
		stmt->getExpr()->accept(this);
		m_Place = nullptr;
	} else if (DL.is_scalar_ty(ty)) {
		m_VC = ValueContext::RValue;
		stmt->getExpr()->accept(this);
		assert(m_Value && "Return expression does not produce a value.");

		m_Builder.build_ret(m_Value);
	} else {
		assert(m_Function->hasARetAttribute() && 
			"Return type is an aggregate, but function has no ARet.");

		m_VC = ValueContext::LValue;
		stmt->getExpr()->accept(this);
		assert(m_Value && "Return expression does not produce a value.");

		unsigned size = DL.get_type_size(ty);
		unsigned align = DL.get_type_align(ty);

		m_Builder.build_cpy(m_Function->get_arg(0), align, m_Value, 
			align, mir::ConstantInt::get(m_Segment, m_Builder.get_i64_ty(), size));
		m_Value = nullptr;
	}
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

void CGN::cgn_aggregate_init(mir::Value *base, Expr *expr, Type *ty) {
    if (ty->isArray()) {
		ArrayExpr *array = static_cast<ArrayExpr *>(expr);
		Type *elemTy = static_cast<ArrayType *>(ty)->getElement();

		for (unsigned i = 0, n = array->getElements().size(); i != n; ++i) {
			mir::Value *elem = m_Builder.build_ap(
				mir::PointerType::get(m_Segment, cgn_type(elemTy)), 
				base, 
				mir::ConstantInt::get(m_Segment, m_Builder.get_i64_ty(), i),
				m_Opts.NamedMIR ? "agg.elem" : ""
			);
			cgn_aggregate_init(elem, array->getElements().at(i), elemTy);
		}
	} else {
		// Scalar type.
		expr->accept(this);
		m_Builder.build_store(m_Value, base);
	}

	/*
	} else if (ty->is_struct()) {
        const auto* structExpr = cast<StructExpr>(expr);
        for (size_t i = 0; i < structExpr->fields.size(); ++i) {
            auto fieldTy = ty->get_field_type(i);

            Value* fieldPtr = builder.build_ap(fieldTy->get_pointer_to(), basePtr, i);
            emit_aggregate_init(fieldPtr, structExpr->fields[i], fieldTy);
	}
	*/
}

void CGN::visit(AccessExpr *expr) {
	ValueContext oldVC = m_VC;

	if (expr->getBase()->getType()->isPointer())
		m_VC = ValueContext::RValue;
	else if (expr->getBase()->getType()->isStruct())
		m_VC = ValueContext::LValue;
	else
		assert(false && "Access base is not a pointer or struct.");

	expr->getBase()->accept(this);
	assert(m_Value && "Access base does not produce a value.");
	FieldDecl *fld = static_cast<FieldDecl *>(expr->getRef());

	mir::Type *ty = cgn_type(expr->getType());

	m_Value = m_Builder.build_ap(mir::PointerType::get(m_Segment, ty), m_Value, 
		mir::ConstantInt::get(m_Segment, m_Builder.get_i64_ty(), fld->getIndex()), 
		m_Opts.NamedMIR ? "access.ptr" : "");

    if (oldVC == ValueContext::RValue)
        m_Value = m_Builder.build_load(ty, m_Value, 
			m_Opts.NamedMIR ? expr->getName() + ".val" : "");
}

void CGN::visit(ArrayExpr *expr) {
	assert(m_Place && "RValue array type needs a destination place.");

	cgn_aggregate_init(m_Place, expr, expr->getType());
	m_Value = m_Place;
}

void CGN::visit(BinaryExpr *expr) {
	switch (expr->getKind()) {
	case BinaryExpr::Kind::Unknown: 
		assert(false && "Unknown binary operator.");
	case BinaryExpr::Kind::Assign: 
		return cgn_assign(expr);
	case BinaryExpr::Kind::Add_Assign: 
		return cgn_add_assign(expr);
	case BinaryExpr::Kind::Sub_Assign: 
		return cgn_sub_assign(expr);
	case BinaryExpr::Kind::Mul_Assign: 
		return cgn_mul_assign(expr);
	case BinaryExpr::Kind::Div_Assign: 
		return cgn_div_assign(expr);
	case BinaryExpr::Kind::Mod_Assign: 
		return cgn_mod_assign(expr);
	case BinaryExpr::Kind::And_Assign: 
		return cgn_and_assign(expr);
	case BinaryExpr::Kind::Or_Assign: 
		return cgn_or_assign(expr);
	case BinaryExpr::Kind::Xor_Assign: 
		return cgn_xor_assign(expr);
	case BinaryExpr::Kind::LeftShift_Assign: 
		return cgn_shl_assign(expr);
	case BinaryExpr::Kind::RightShift_Assign: 
		return cgn_shr_assign(expr);
	case BinaryExpr::Kind::Add: 
		return cgn_add(expr);
	case BinaryExpr::Kind::Sub: 
		return cgn_sub(expr);
	case BinaryExpr::Kind::Mul: 
		return cgn_mul(expr);
	case BinaryExpr::Kind::Div: 
		return cgn_div(expr);
	case BinaryExpr::Kind::Mod: 
		return cgn_mod(expr);
	case BinaryExpr::Kind::Bitwise_And: 
		return cgn_and(expr);
	case BinaryExpr::Kind::Bitwise_Or: 
		return cgn_or(expr);
	case BinaryExpr::Kind::Bitwise_Xor: 
		return cgn_xor(expr);
	case BinaryExpr::Kind::LeftShift: 
		return cgn_shl(expr);
	case BinaryExpr::Kind::RightShift:
		return cgn_shr(expr);
	case BinaryExpr::Kind::Logic_And: 
		return cgn_logic_and(expr);
	case BinaryExpr::Kind::Logic_Or: 
		return cgn_logic_or(expr);
	case BinaryExpr::Kind::Equals: 
		return cgn_equals(expr);
	case BinaryExpr::Kind::NEquals: 
		return cgn_not_equals(expr);
	case BinaryExpr::Kind::LessThan: 
		return cgn_less(expr);
	case BinaryExpr::Kind::LessThanEquals: 
		return cgn_less_eq(expr);
	case BinaryExpr::Kind::GreaterThan:
		return cgn_greater(expr);
	case BinaryExpr::Kind::GreaterThanEquals:
		return cgn_greater_eq(expr);
    }
}

void CGN::visit(CallExpr *expr) {
	mir::Function *callee = m_Segment->get_function(mangle_name(expr->getCallee()));
	assert(callee && "Callee does not exist.");

	std::vector<mir::Value *> args;
	args.reserve(expr->getNumArgs());
	mir::Value *ARet = nullptr;
	mir::Type *ty = cgn_type(expr->getType());

	if (callee->hasARetAttribute()) {
		ARet = m_Place ? m_Place : m_Builder.build_slot(ty, m_Opts.NamedMIR ? "aret.tmp" : "");
		args.push_back(ARet);
	}

    for (unsigned i = 0; i != expr->getNumArgs(); ++i) {
        Expr *arg = expr->getArg(i);
		
		if (callee->hasArgAttribute(ARet ? i + 1 : i, mir::Attribute::AArg)) {
			// Aggregate arguments must be copied before being passed to the
			// callee, since the caller is always responsible for cloning the
			// argument, regardless if its to be spilled in the callee.
			mir::DataLayout DL = m_Segment->get_data_layout();
			mir::Type *aargTy = cgn_type(arg->getType());
			mir::Slot *aargSlot = m_Builder.build_slot(aargTy, 
				m_Opts.NamedMIR ? "aarg.tmp" : "");

			unsigned align = DL.get_type_align(aargTy);
			unsigned size = DL.get_type_size(aargTy);

			m_VC = ValueContext::LValue;
			m_Place = aargSlot;
			arg->accept(this);
			m_Place = nullptr;

			// Emit a copy from the original aggregate to the new temporary
			// slot for it.
			if (m_Value != aargSlot) {
				assert(m_Value && "Call argument does not produce a value.");
				m_Builder.build_cpy(aargSlot, align, m_Value, align, 
					mir::ConstantInt::get(m_Segment, m_Builder.get_i64_ty(), size));
			}

			args.push_back(aargSlot);
        } else {
			m_VC = ValueContext::RValue;
			arg->accept(this);
			assert(m_Value && "Call argument does not produce a value.");
            args.push_back(m_Value);
		}
    }

	mir::CallInst *call = m_Builder.build_call(callee, args, 
		m_Opts.NamedMIR ? "call.tmp" : "");

	if (ARet)
		m_Value = ARet;
	else
		m_Value = call;
}

void CGN::visit(MethodCallExpr *expr) {
	mir::Function *callee = m_Segment->get_function(mangle_name(expr->getCallee()));
	assert(callee && "Callee does not exist.");

	std::vector<mir::Value *> args;
	args.reserve(expr->getNumArgs());
	mir::Value *ARet = nullptr;
	mir::Type *ty = cgn_type(expr->getType());

	if (callee->hasARetAttribute()) {
		ARet = m_Place ? m_Place : m_Builder.build_slot(ty, m_Opts.NamedMIR ? "aret.tmp" : "");
		args.push_back(ARet);
	}

	if (expr->getBase()->getType()->isPointer())
		m_VC = ValueContext::RValue;
	else if (expr->getBase()->getType()->isStruct())
		m_VC = ValueContext::LValue;
	else
		assert(false && "Access base is not a pointer or struct.");

	expr->getBase()->accept(this);
	assert(m_Value && "Access base does not produce a value.");
	args.push_back(m_Value);

    for (unsigned i = 0; i != expr->getNumArgs(); ++i) {
        Expr *arg = expr->getArg(i);
		
		if (callee->hasArgAttribute(ARet ? i + 1 : i, mir::Attribute::AArg)) {
			// Aggregate arguments must be copied before being passed to the
			// callee, since the caller is always responsible for cloning the
			// argument, regardless if its to be spilled in the callee.
			mir::DataLayout DL = m_Segment->get_data_layout();
			mir::Type *aargTy = cgn_type(arg->getType());
			mir::Slot *aargSlot = m_Builder.build_slot(aargTy, 
				m_Opts.NamedMIR ? "aarg.tmp" : "");

			unsigned align = DL.get_type_align(aargTy);
			unsigned size = DL.get_type_size(aargTy);

			m_VC = ValueContext::LValue;
			m_Place = aargSlot;
			arg->accept(this);
			m_Place = nullptr;

			// Emit a copy from the original aggregate to the new temporary
			// slot for it.
			if (m_Value != aargSlot) {
				assert(m_Value && "Call argument does not produce a value.");
				m_Builder.build_cpy(aargSlot, align, m_Value, align, 
					mir::ConstantInt::get(m_Segment, m_Builder.get_i64_ty(), size));
			}

			args.push_back(aargSlot);
        } else {
			m_VC = ValueContext::RValue;
			arg->accept(this);
			assert(m_Value && "Call argument does not produce a value.");
            args.push_back(m_Value);
		}
    }

	mir::CallInst *call = m_Builder.build_call(callee, args, 
		m_Opts.NamedMIR ? "call.tmp" : "");

	if (ARet)
		m_Value = ARet;
	else
		m_Value = call;
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

	if (auto *field = dynamic_cast<FieldDecl *>(ref)) {
		assert(m_Self && "Reference to field decl, but no self set.");

		unsigned field_idx = field->getIndex();
		mir::Type *field_ty = cgn_type(expr->getType());
		mir::Value *self_ptr = m_Builder.build_load(
			mir::PointerType::get(
				m_Segment, cgn_type(field->getParent()->getDefinedType())
			), 
			m_Self
		);

		mir::Value *field_ptr = m_Builder.build_ap(
			mir::PointerType::get(m_Segment, field_ty), 
			self_ptr, 
			mir::ConstantInt::get(m_Segment, m_Builder.get_i64_ty(), field_idx)
		);

		if (m_VC == ValueContext::LValue)
			m_Value = field_ptr;
		else
			m_Value = m_Builder.build_load(field_ty, field_ptr);

		return;
	}

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

void CGN::visit(SubscriptExpr *expr) {
	ValueContext oldVC = m_VC;
	mir::Value *base = nullptr;
	mir::Value *idx = nullptr;
	mir::Type *ty = cgn_type(expr->getType());
	mir::Type *ptrTy = mir::PointerType::get(m_Segment, ty);

	m_VC = ValueContext::LValue;
	if (expr->getBase()->getType()->isPointer()) 
		m_VC = ValueContext::RValue;

	expr->getBase()->accept(this);
	assert(m_Value && "Subscript base does not produce a value.");
	base = m_Value;

    m_VC = ValueContext::RValue;
    expr->getIndex()->accept(this);
    assert(m_Value && "Subscript index does not produce a value.");
    idx = m_Value;

    m_Value = m_Builder.build_ap(ptrTy, base, idx, 
		m_Opts.NamedMIR ? "ss.ptr" : "");

    if (oldVC == ValueContext::RValue) {
        m_Value = m_Builder.build_load(ty, m_Value, 
			m_Opts.NamedMIR ? "ss.val" : "");
	}
}

void CGN::visit(TypeSpecExpr *expr) {
	RefExpr *ref = expr->getExpr();

	if (auto *EVD = dynamic_cast<EnumVariantDecl *>(ref->getRef())) {
		m_Value = mir::ConstantInt::get(m_Segment, cgn_type(EVD->getType()), 
			EVD->getValue());
	} else {
		expr->getExpr()->accept(this);
	}
}

void CGN::visit(UnaryExpr *expr) {
	switch (expr->getKind()) {
	case UnaryExpr::Kind::Logic_Not: 
		return cgn_logic_not(expr);
	case UnaryExpr::Kind::Bitwise_Not: 
		return cgn_not(expr);
	case UnaryExpr::Kind::Negate: 
		return cgn_neg(expr);
	case UnaryExpr::Kind::Address_Of: 
		return cgn_addrof(expr);
	case UnaryExpr::Kind::Dereference: 
		return cgn_deref(expr);
	case UnaryExpr::Kind::Increment: 
		return cgn_inc(expr);
	case UnaryExpr::Kind::Decrement: 
		return cgn_dec(expr);
	default: 
		assert(false && "Unknown unary operator.");
    }
}
