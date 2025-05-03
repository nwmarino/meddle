#include "codegen.h"
#include "../core/logger.h"
#include "../tree/expr.h"
#include "../mir/basicblock.h"
#include "../mir/builder.h"
#include "../mir/function.h"

#include <cassert>

using namespace meddle;

void CGN::cgn_assign(BinaryExpr *BIN) {
    assert(BIN->getKind() == BinaryExpr::Kind::Assign);

    m_VC = ValueContext::LValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    mir::Value *dest = m_Value;

    mir::Type *ty = cgn_type(BIN->getType());
    mir::DataLayout DL = m_Segment->get_data_layout();
    bool isAggregate = !DL.is_scalar_ty(ty);
    unsigned size = DL.get_type_size(ty);

    m_VC = ValueContext::RValue;
    BIN->getRHS()->accept(this);
    assert(m_Value && "Binary RHS does not produce a value.");

    if (size > DL.get_pointer_size() || isAggregate) {
        m_Builder.build_cpy(
            dest, 
            DL.get_type_align(ty), 
            m_Value, 
            DL.get_type_align(ty), 
            size
        );
    } else {
        m_Builder.build_store(m_Value, dest);
    }
}

void CGN::cgn_add_assign(BinaryExpr *BIN) {
    assert(BIN->getKind() == BinaryExpr::Kind::Add_Assign);

    m_VC = ValueContext::RValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    mir::Value *LHS = m_Value;
    BIN->getRHS()->accept(this);
    assert(m_Value && "Binary RHS does not produce a value.");
    mir::Value *RHS = m_Value;

    mir::Value *ADD = nullptr;
    String name = m_Opts.NamedMIR ? "add.asn" : "";
    if (LHS->get_type()->is_pointer_ty() && RHS->get_type()->is_integer_ty()) {
        ADD = m_Builder.build_ap(LHS->get_type(), LHS, RHS, name);
    } else if (LHS->get_type()->is_integer_ty()) {
        ADD = m_Builder.build_add(LHS, RHS, name);
    } else if (LHS->get_type()->is_float_ty()) {
        ADD = m_Builder.build_fadd(LHS, RHS, name);
    } else {
        fatal("unsupported '+=' operator between types", &BIN->getMetadata());
    }

    m_VC = ValueContext::LValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    m_Builder.build_store(ADD, m_Value);
    m_Value = ADD;
}

void CGN::cgn_sub_assign(BinaryExpr *BIN) {
    assert(BIN->getKind() == BinaryExpr::Kind::Sub_Assign);

    m_VC = ValueContext::RValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    mir::Value *LHS = m_Value;
    BIN->getRHS()->accept(this);
    assert(m_Value && "Binary RHS does not produce a value.");
    mir::Value *RHS = m_Value;

    mir::Value *SUB = nullptr;
    String name = m_Opts.NamedMIR ? "sub.asn" : "";
    if (LHS->get_type()->is_pointer_ty() && RHS->get_type()->is_integer_ty()) {
        mir::Value *neg = m_Builder.build_neg(RHS, m_Opts.NamedMIR ? "parith.neg" : "");
        SUB = m_Builder.build_ap(LHS->get_type(), LHS, neg, name);
    } else if (LHS->get_type()->is_integer_ty()) {
        SUB = m_Builder.build_sub(LHS, RHS, name);
    } else if (LHS->get_type()->is_float_ty()) {
        SUB = m_Builder.build_fsub(LHS, RHS, name);
    } else {
        fatal("unsupported '-=' operator between types", &BIN->getMetadata());
    }

    m_VC = ValueContext::LValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    m_Builder.build_store(SUB, m_Value);
    m_Value = SUB;
}

void CGN::cgn_mul_assign(BinaryExpr *BIN) {
    assert(BIN->getKind() == BinaryExpr::Kind::Mul_Assign);

    m_VC = ValueContext::RValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    mir::Value *LHS = m_Value;
    BIN->getRHS()->accept(this);
    assert(m_Value && "Binary RHS does not produce a value.");
    mir::Value *RHS = m_Value;

    mir::Value *MUL = nullptr;
    String name = m_Opts.NamedMIR ? "mul.asn" : "";
    if (BIN->getLHS()->getType()->isSInt())
        MUL = m_Builder.build_smul(LHS, RHS, name);
    else if (BIN->getLHS()->getType()->isUInt())
        MUL = m_Builder.build_umul(LHS, RHS, name);
    else if (BIN->getLHS()->getType()->isFloat())
        MUL = m_Builder.build_fmul(LHS, RHS, name);
    else
        fatal("unsupported '*=' operator between types", &BIN->getMetadata());

    m_VC = ValueContext::LValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    m_Builder.build_store(MUL, m_Value);
    m_Value = MUL;
}

void CGN::cgn_div_assign(BinaryExpr *BIN) {
    assert(BIN->getKind() == BinaryExpr::Kind::Div_Assign);

    m_VC = ValueContext::RValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    mir::Value *LHS = m_Value;
    BIN->getRHS()->accept(this);
    assert(m_Value && "Binary RHS does not produce a value.");
    mir::Value *RHS = m_Value;

    mir::Value *DIV = nullptr;
    String name = m_Opts.NamedMIR ? "div.asn" : "";
    if (BIN->getLHS()->getType()->isSInt())
        DIV = m_Builder.build_sdiv(LHS, RHS, name);
    else if (BIN->getLHS()->getType()->isUInt())
        DIV = m_Builder.build_udiv(LHS, RHS, name);
    else if (BIN->getLHS()->getType()->isFloat())
        DIV = m_Builder.build_fdiv(LHS, RHS, name);
    else
        fatal("unsupported '/=' operator between types", &BIN->getMetadata());

    m_VC = ValueContext::LValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    m_Builder.build_store(DIV, m_Value);
    m_Value = DIV;
}

void CGN::cgn_mod_assign(BinaryExpr *BIN) {
    assert(BIN->getKind() == BinaryExpr::Kind::Mod_Assign);

    m_VC = ValueContext::RValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    mir::Value *LHS = m_Value;
    BIN->getRHS()->accept(this);
    assert(m_Value && "Binary RHS does not produce a value.");
    mir::Value *RHS = m_Value;

    String name = m_Opts.NamedMIR ? "mod.asn" : "";
    mir::Value *MOD = nullptr;
    switch (type_class(BIN->getType())) {
    case TypeClass::SInt:
        MOD = m_Builder.build_srem(LHS, RHS, name);
        break;
    case TypeClass::UInt:
        MOD = m_Builder.build_urem(LHS, RHS, name);
        break;
    default:
        fatal("unsupported '%=' operator between types", &BIN->getMetadata());
    }

    m_VC = ValueContext::LValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    m_Builder.build_store(MOD, m_Value);
    m_Value = MOD;
}

void CGN::cgn_and_assign(BinaryExpr *BIN) {
    assert(BIN->getKind() == BinaryExpr::Kind::And_Assign);

    m_VC = ValueContext::RValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    mir::Value *LHS = m_Value;
    BIN->getRHS()->accept(this);
    assert(m_Value && "Binary RHS does not produce a value.");
    mir::Value *RHS = m_Value;

    mir::Value *AND = nullptr;
    if (LHS->get_type()->is_integer_ty())
        AND = m_Builder.build_and(LHS, RHS, m_Opts.NamedMIR ? "and.asn" : "");
    else
        fatal("unsupported '&=' operator between types", &BIN->getMetadata());

    m_VC = ValueContext::LValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    m_Builder.build_store(AND, m_Value);
    m_Value = AND;
}

void CGN::cgn_or_assign(BinaryExpr *BIN) {
    assert(BIN->getKind() == BinaryExpr::Kind::Or_Assign);

    m_VC = ValueContext::RValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    mir::Value *LHS = m_Value;
    BIN->getRHS()->accept(this);
    assert(m_Value && "Binary RHS does not produce a value.");
    mir::Value *RHS = m_Value;

    mir::Value *OR = nullptr;
    if (LHS->get_type()->is_integer_ty())
        OR = m_Builder.build_or(LHS, RHS, m_Opts.NamedMIR ? "or.asn" : "");
    else
        fatal("unsupported '|=' operator between types", &BIN->getMetadata());

    m_VC = ValueContext::LValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    m_Builder.build_store(OR, m_Value);
    m_Value = OR;
}

void CGN::cgn_xor_assign(BinaryExpr *BIN) {
    assert(BIN->getKind() == BinaryExpr::Kind::Xor_Assign);

    m_VC = ValueContext::RValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    mir::Value *LHS = m_Value;
    BIN->getRHS()->accept(this);
    assert(m_Value && "Binary RHS does not produce a value.");
    mir::Value *RHS = m_Value;

    mir::Value *XOR = nullptr;
    if (LHS->get_type()->is_integer_ty())
        XOR = m_Builder.build_xor(LHS, RHS, m_Opts.NamedMIR ? "xor.asn" : "");
    else
        fatal("unsupported '^=' operator between types", &BIN->getMetadata());

    m_VC = ValueContext::LValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    m_Builder.build_store(XOR, m_Value);
    m_Value = XOR;
}

void CGN::cgn_shl_assign(BinaryExpr *BIN) {
    assert(BIN->getKind() == BinaryExpr::Kind::LeftShift_Assign);

    m_VC = ValueContext::RValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    mir::Value *LHS = m_Value;
    BIN->getRHS()->accept(this);
    assert(m_Value && "Binary RHS does not produce a value.");
    mir::Value *RHS = m_Value;

    mir::Value *SHL = nullptr;
    if (LHS->get_type()->is_integer_ty())
        SHL = m_Builder.build_shl(LHS, RHS, m_Opts.NamedMIR ? "shl.asn" : "");
    else
        fatal("unsupported '<<=' operator between types", &BIN->getMetadata());

    m_VC = ValueContext::LValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    m_Builder.build_store(SHL, m_Value);
    m_Value = SHL;
}

void CGN::cgn_shr_assign(BinaryExpr *BIN) {
    assert(BIN->getKind() == BinaryExpr::Kind::RightShift_Assign);

    m_VC = ValueContext::RValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    mir::Value *LHS = m_Value;
    BIN->getRHS()->accept(this);
    assert(m_Value && "Binary RHS does not produce a value.");
    mir::Value *RHS = m_Value;

    mir::Value *SHR = nullptr;
    if (BIN->getType()->isSInt())
        SHR = m_Builder.build_ashr(LHS, RHS, m_Opts.NamedMIR ? "shr.asn" : "");
    else if (BIN->getType()->isUInt())
        SHR = m_Builder.build_lshr(LHS, RHS, m_Opts.NamedMIR ? "shr.asn" : "");
    else
        fatal("unsupported '>>=' operator between types", &BIN->getMetadata());

    m_VC = ValueContext::LValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    m_Builder.build_store(SHR, m_Value);
    m_Value = SHR;
}

void CGN::cgn_equals(BinaryExpr *BIN) {
    assert(BIN->getKind() == BinaryExpr::Kind::Equals);

    m_VC = ValueContext::RValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    mir::Value *LHS = m_Value;
    BIN->getRHS()->accept(this);
    assert(m_Value && "Binary RHS does not produce a value.");
    mir::Value *RHS = m_Value;

    if (LHS->get_type()->is_integer_ty()) {
        m_Value = m_Builder.build_icmp_eq(LHS, RHS, 
            m_Opts.NamedMIR ? "cmp.eq" : "");
    } else if (LHS->get_type()->is_float_ty()) {
        m_Value = m_Builder.build_fcmp_oeq(LHS, RHS, 
            m_Opts.NamedMIR ? "cmp.eq" : "");
    } else if (LHS->get_type()->is_pointer_ty()) {
        m_Value = m_Builder.build_pcmp_eq(LHS, RHS, 
            m_Opts.NamedMIR ? "cmp.eq" : "");
    } else {
        fatal("unsupported '==' operator between types", &BIN->getMetadata());
    }
}

void CGN::cgn_not_equals(BinaryExpr *BIN) {
    assert(BIN->getKind() == BinaryExpr::Kind::NEquals);

    m_VC = ValueContext::RValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    mir::Value *LHS = m_Value;
    BIN->getRHS()->accept(this);
    assert(m_Value && "Binary RHS does not produce a value.");
    mir::Value *RHS = m_Value;

    if (LHS->get_type()->is_integer_ty()) {
        m_Value = m_Builder.build_icmp_ne(LHS, RHS, 
            m_Opts.NamedMIR ? "cmp.ne" : "");
    } else if (LHS->get_type()->is_float_ty()) {
        m_Value = m_Builder.build_fcmp_one(LHS, RHS, 
            m_Opts.NamedMIR ? "cmp.ne" : "");
    } else if (LHS->get_type()->is_pointer_ty()) {
        m_Value = m_Builder.build_pcmp_ne(LHS, RHS, 
            m_Opts.NamedMIR ? "cmp.ne" : "");
    } else {
        fatal("unsupported '!=' operator between types", &BIN->getMetadata());
    }
}

void CGN::cgn_less(BinaryExpr *BIN) {
    assert(BIN->getKind() == BinaryExpr::Kind::LessThan);

    m_VC = ValueContext::RValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    mir::Value *LHS = m_Value;
    BIN->getRHS()->accept(this);
    assert(m_Value && "Binary RHS does not produce a value.");
    mir::Value *RHS = m_Value;

    if (BIN->getLHS()->getType()->isSInt()) {
        m_Value = m_Builder.build_icmp_slt(LHS, RHS, 
            m_Opts.NamedMIR ? "cmp.lt" : "");
    } else if (BIN->getLHS()->getType()->isUInt()) {
        m_Value = m_Builder.build_icmp_ult(LHS, RHS, 
            m_Opts.NamedMIR ? "cmp.lt" : "");
    } else if (LHS->get_type()->is_pointer_ty()) {
        m_Value = m_Builder.build_pcmp_lt(LHS, RHS, 
            m_Opts.NamedMIR ? "cmp.lt" : "");
    } else if (LHS->get_type()->is_float_ty()) {
        m_Value = m_Builder.build_fcmp_olt(LHS, RHS, 
            m_Opts.NamedMIR ? "cmp.lt" : "");
    } else {
        fatal("unsupported '<' operator between types", &BIN->getMetadata());
    }
}

void CGN::cgn_less_eq(BinaryExpr *BIN) {
    assert(BIN->getKind() == BinaryExpr::Kind::LessThanEquals);

    m_VC = ValueContext::RValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    mir::Value *LHS = m_Value;
    BIN->getRHS()->accept(this);
    assert(m_Value && "Binary RHS does not produce a value.");
    mir::Value *RHS = m_Value;

    if (BIN->getLHS()->getType()->isSInt()) {
        m_Value = m_Builder.build_icmp_sle(LHS, RHS, 
            m_Opts.NamedMIR ? "cmp.le" : "");
    } else if (BIN->getLHS()->getType()->isUInt()) {
        m_Value = m_Builder.build_icmp_ule(LHS, RHS, 
            m_Opts.NamedMIR ? "cmp.le" : "");
    } else if (LHS->get_type()->is_pointer_ty()) {
        m_Value = m_Builder.build_pcmp_le(LHS, RHS, 
            m_Opts.NamedMIR ? "cmp.le" : "");
    } else if (LHS->get_type()->is_float_ty()) {
        m_Value = m_Builder.build_fcmp_ole(LHS, RHS, 
            m_Opts.NamedMIR ? "cmp.le" : "");
    } else {
        fatal("unsupported '<=' operator between types", &BIN->getMetadata());
    }
}

void CGN::cgn_greater(BinaryExpr *BIN) {
    assert(BIN->getKind() == BinaryExpr::Kind::GreaterThan);

    m_VC = ValueContext::RValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    mir::Value *LHS = m_Value;
    BIN->getRHS()->accept(this);
    assert(m_Value && "Binary RHS does not produce a value.");
    mir::Value *RHS = m_Value;

    if (BIN->getLHS()->getType()->isSInt()) {
        m_Value = m_Builder.build_icmp_sgt(LHS, RHS, 
            m_Opts.NamedMIR ? "cmp.gt" : "");
    } else if (BIN->getLHS()->getType()->isUInt()) {
        m_Value = m_Builder.build_icmp_ugt(LHS, RHS, 
            m_Opts.NamedMIR ? "cmp.gt" : "");
    } else if (LHS->get_type()->is_pointer_ty()) {
        m_Value = m_Builder.build_pcmp_gt(LHS, RHS, 
            m_Opts.NamedMIR ? "cmp.gt" : "");
    } else if (LHS->get_type()->is_float_ty()) {
        m_Value = m_Builder.build_fcmp_ogt(LHS, RHS, 
            m_Opts.NamedMIR ? "cmp.gt" : "");
    } else {
        fatal("unsupported '>' operator between types", &BIN->getMetadata());
    }
}

void CGN::cgn_greater_eq(BinaryExpr *BIN) {
    assert(BIN->getKind() == BinaryExpr::Kind::GreaterThanEquals);

    m_VC = ValueContext::RValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    mir::Value *LHS = m_Value;
    BIN->getRHS()->accept(this);
    assert(m_Value && "Binary RHS does not produce a value.");
    mir::Value *RHS = m_Value;

    if (BIN->getLHS()->getType()->isSInt()) {
        m_Value = m_Builder.build_icmp_sge(LHS, RHS, 
            m_Opts.NamedMIR ? "cmp.ge" : "");
    } else if (BIN->getLHS()->getType()->isUInt()) {
        m_Value = m_Builder.build_icmp_uge(LHS, RHS, 
            m_Opts.NamedMIR ? "cmp.ge" : "");
    } else if (LHS->get_type()->is_pointer_ty()) {
        m_Value = m_Builder.build_pcmp_ge(LHS, RHS, 
            m_Opts.NamedMIR ? "cmp.ge" : "");
    } else if (LHS->get_type()->is_float_ty()) {
        m_Value = m_Builder.build_fcmp_oge(LHS, RHS, 
            m_Opts.NamedMIR ? "cmp.ge" : "");
    } else { 
        fatal("unsupported '>=' operator between types", &BIN->getMetadata());
    }
}

void CGN::cgn_add(BinaryExpr *BIN) {
    assert(BIN->getKind() == BinaryExpr::Kind::Add);

    m_VC = ValueContext::RValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    mir::Value *LHS = m_Value;
    BIN->getRHS()->accept(this);
    assert(m_Value && "Binary RHS does not produce a value.");
    mir::Value *RHS = m_Value;

    String name = m_Opts.NamedMIR ? "add" : "";
    if (LHS->get_type()->is_pointer_ty() && RHS->get_type()->is_integer_ty()) {
        m_Value = m_Builder.build_ap(LHS->get_type(), LHS, RHS);
    } else if (LHS->get_type()->is_integer_ty()) {
        m_Value = m_Builder.build_add(LHS, RHS, name);
    } else if (LHS->get_type()->is_float_ty()) {
        m_Value = m_Builder.build_fadd(LHS, RHS, name);
    } else {
        fatal("unsupported '+' operator between types", &BIN->getMetadata());
    }
}

void CGN::cgn_sub(BinaryExpr *BIN) {
    assert(BIN->getKind() == BinaryExpr::Kind::Sub);

    m_VC = ValueContext::RValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    mir::Value *LHS = m_Value;
    BIN->getRHS()->accept(this);
    assert(m_Value && "Binary RHS does not produce a value.");
    mir::Value *RHS = m_Value;

    String name = m_Opts.NamedMIR ? "sub" : "";
    if (LHS->get_type()->is_pointer_ty() && RHS->get_type()->is_integer_ty()) {
        mir::Value *neg = m_Builder.build_neg(RHS, m_Opts.NamedMIR ? "parith.neg" : "");
        m_Value = m_Builder.build_ap(LHS->get_type(), LHS, neg, name);
    } else if (LHS->get_type()->is_integer_ty()) {
        m_Value = m_Builder.build_sub(LHS, RHS, name);
    } else if (LHS->get_type()->is_float_ty()) {
        m_Value = m_Builder.build_fsub(LHS, RHS, name);
    } else {
        fatal("unsupported '-' operator between types", &BIN->getMetadata());
    }
}

void CGN::cgn_mul(BinaryExpr *BIN) {
    assert(BIN->getKind() == BinaryExpr::Kind::Mul);

    m_VC = ValueContext::RValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    mir::Value *LHS = m_Value;
    BIN->getRHS()->accept(this);
    assert(m_Value && "Binary RHS does not produce a value.");
    mir::Value *RHS = m_Value;

    if (BIN->getLHS()->getType()->isSInt())
        m_Value = m_Builder.build_smul(LHS, RHS, m_Opts.NamedMIR ? "mul" : "");
    else if (BIN->getLHS()->getType()->isUInt())
        m_Value = m_Builder.build_umul(LHS, RHS, m_Opts.NamedMIR ? "mul" : "");
    else if (BIN->getLHS()->getType()->isFloat())
        m_Value = m_Builder.build_fmul(LHS, RHS, m_Opts.NamedMIR ? "mul" : "");
    else
        fatal("unsupported '*' operator between types", &BIN->getMetadata());
}

void CGN::cgn_div(BinaryExpr *BIN) {
    assert(BIN->getKind() == BinaryExpr::Kind::Div);

    m_VC = ValueContext::RValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    mir::Value *LHS = m_Value;
    BIN->getRHS()->accept(this);
    assert(m_Value && "Binary RHS does not produce a value.");
    mir::Value *RHS = m_Value;

    if (BIN->getLHS()->getType()->isSInt())
        m_Value = m_Builder.build_sdiv(LHS, RHS, m_Opts.NamedMIR ? "div" : "");
    else if (BIN->getLHS()->getType()->isUInt())
        m_Value = m_Builder.build_udiv(LHS, RHS, m_Opts.NamedMIR ? "div" : "");
    else if (BIN->getLHS()->getType()->isFloat())
        m_Value = m_Builder.build_fdiv(LHS, RHS, m_Opts.NamedMIR ? "div" : "");
    else
        fatal("unsupported '/' operator between types", &BIN->getMetadata());
}

void CGN::cgn_mod(BinaryExpr *BIN) {
    assert(BIN->getKind() == BinaryExpr::Kind::Mod);

    m_VC = ValueContext::RValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    mir::Value *LHS = m_Value;
    BIN->getRHS()->accept(this);
    assert(m_Value && "Binary RHS does not produce a value.");
    mir::Value *RHS = m_Value;

    if (BIN->getLHS()->getType()->isSInt())
        m_Value = m_Builder.build_srem(LHS, RHS, m_Opts.NamedMIR ? "mod" : "");
    else if (BIN->getLHS()->getType()->isUInt())
        m_Value = m_Builder.build_urem(LHS, RHS, m_Opts.NamedMIR ? "mod" : "");
    else
        fatal("unsupported '%' operator between types", &BIN->getMetadata());
}

void CGN::cgn_and(BinaryExpr *BIN) {
    assert(BIN->getKind() == BinaryExpr::Kind::Bitwise_And);

    m_VC = ValueContext::RValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    mir::Value *LHS = m_Value;
    BIN->getRHS()->accept(this);
    assert(m_Value && "Binary RHS does not produce a value.");
    mir::Value *RHS = m_Value;

    if (LHS->get_type()->is_integer_ty())
        m_Value = m_Builder.build_and(LHS, RHS, m_Opts.NamedMIR ? "and" : "");
    else
        fatal("unsupported '&' operator between types", &BIN->getMetadata());
}

void CGN::cgn_or(BinaryExpr *BIN) {
    assert(BIN->getKind() == BinaryExpr::Kind::Bitwise_Or);

    m_VC = ValueContext::RValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    mir::Value *LHS = m_Value;
    BIN->getRHS()->accept(this);
    assert(m_Value && "Binary RHS does not produce a value.");
    mir::Value *RHS = m_Value;

    if (LHS->get_type()->is_integer_ty())
        m_Value = m_Builder.build_or(LHS, RHS, m_Opts.NamedMIR ? "or" : "");
    else
        fatal("unsupported '|' operator between types", &BIN->getMetadata());
}

void CGN::cgn_xor(BinaryExpr *BIN) {
    assert(BIN->getKind() == BinaryExpr::Kind::Bitwise_Xor);

    m_VC = ValueContext::RValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    mir::Value *LHS = m_Value;
    BIN->getRHS()->accept(this);
    assert(m_Value && "Binary RHS does not produce a value.");
    mir::Value *RHS = m_Value;

    if (LHS->get_type()->is_integer_ty())
        m_Value = m_Builder.build_xor(LHS, RHS, m_Opts.NamedMIR ? "xor" : "");
    else
        fatal("unsupported '^' operator between types", &BIN->getMetadata());
}

void CGN::cgn_shl(BinaryExpr *BIN) {
    assert(BIN->getKind() == BinaryExpr::Kind::LeftShift);

    m_VC = ValueContext::RValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    mir::Value *LHS = m_Value;
    BIN->getRHS()->accept(this);
    assert(m_Value && "Binary RHS does not produce a value.");
    mir::Value *RHS = m_Value;

    if (LHS->get_type()->is_integer_ty())
        m_Value = m_Builder.build_shl(LHS, RHS, m_Opts.NamedMIR ? "shl" : "");
    else
        fatal("unsupported '<<' operator between types", &BIN->getMetadata());
}

void CGN::cgn_shr(BinaryExpr *BIN) {
    assert(BIN->getKind() == BinaryExpr::Kind::RightShift);

    m_VC = ValueContext::RValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    mir::Value *LHS = m_Value;
    BIN->getRHS()->accept(this);
    assert(m_Value && "Binary RHS does not produce a value.");
    mir::Value *RHS = m_Value;

    if (BIN->getType()->isSInt())
        m_Value = m_Builder.build_ashr(LHS, RHS, m_Opts.NamedMIR ? "shr" : "");
    else if (BIN->getType()->isUInt())
        m_Value = m_Builder.build_lshr(LHS, RHS, m_Opts.NamedMIR ? "shr" : "");
    else
        fatal("unsupported '>>' operator between types", &BIN->getMetadata());
}

void CGN::cgn_logic_and(BinaryExpr *BIN) {
    assert(BIN->getKind() == BinaryExpr::Kind::Logic_And);

    mir::BasicBlock *falseBB = m_Builder.get_insert();
    
    mir::BasicBlock *rightBB = new mir::BasicBlock(
        m_Opts.NamedMIR ? "land.rhs" : "");
    mir::BasicBlock *mergeBB = new mir::BasicBlock(
        m_Opts.NamedMIR ? "land.merge" : "");
    
    m_VC = ValueContext::RValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    mir::Value *LHS = m_Value;
    LHS = inject_cmp(LHS);
    
    // Short-circuit: if LHS is false, skip RHS evaluation.
    m_Builder.build_brif(LHS, rightBB, mergeBB);
    
    m_Function->append(rightBB);
    m_Builder.set_insert(rightBB);
    m_VC = ValueContext::RValue;
    BIN->getRHS()->accept(this);
    assert(m_Value && "Binary RHS does not produce a value.");
    mir::Value *RHS = m_Value;
    RHS = inject_cmp(RHS);
    m_Builder.build_jmp(mergeBB);

    mir::BasicBlock *OW = m_Builder.get_insert();

    m_Function->append(mergeBB);
    m_Builder.set_insert(mergeBB);
    mir::PHINode *phi = m_Builder.build_phi(m_Builder.get_i1_ty(),
        m_Opts.NamedMIR ? "land.result" : "");
    phi->add_incoming(mir::ConstantInt::get(m_Segment, m_Builder.get_i1_ty(), 0), falseBB);
    phi->add_incoming(RHS, OW);
    
    m_Value = phi;
}

void CGN::cgn_logic_or(BinaryExpr *BIN) {
    assert(BIN->getKind() == BinaryExpr::Kind::Logic_Or);

    mir::BasicBlock *trueBB = m_Builder.get_insert();
    
    mir::BasicBlock *rightBB = new mir::BasicBlock(
        m_Opts.NamedMIR ? "lor.rhs" : "");
    mir::BasicBlock *mergeBB = new mir::BasicBlock(
        m_Opts.NamedMIR ? "lor.merge" : "");

    m_VC = ValueContext::RValue;
    BIN->getLHS()->accept(this);
    assert(m_Value && "Binary LHS does not produce a value.");
    mir::Value *LHS = m_Value;
    LHS = inject_cmp(LHS);

    // Short-circuit: if LHS is true, skip RHS evaluation.
    m_Builder.build_brif(LHS, mergeBB, rightBB);

    m_Function->append(rightBB);
    m_Builder.set_insert(rightBB);
    m_VC = ValueContext::RValue;
    BIN->getRHS()->accept(this);
    assert(m_Value && "Binary RHS does not produce a value.");
    mir::Value *RHS = m_Value;
    RHS = inject_cmp(RHS);
    m_Builder.build_jmp(mergeBB);

    mir::BasicBlock *OW = m_Builder.get_insert();

    m_Function->append(mergeBB);
    m_Builder.set_insert(mergeBB);
    
    mir::PHINode *phi = m_Builder.build_phi(m_Builder.get_i1_ty(), 
        m_Opts.NamedMIR ? "lor.result" : "");
    phi->add_incoming(mir::ConstantInt::get(m_Segment, m_Builder.get_i1_ty(), 1), trueBB);
    phi->add_incoming(RHS, OW);

    m_Value = phi;
}
