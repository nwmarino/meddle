#include "codegen.h"
#include "../core/logger.h"
#include "../tree/expr.h"
#include "../mir/builder.h"

#include <cassert>

using namespace meddle;

void CGN::cgn_not(UnaryExpr *UN) {
    assert(UN->getKind() == UnaryExpr::Kind::Bitwise_Not);

    m_VC = ValueContext::RValue;
    UN->getExpr()->accept(this);
    assert(m_Value && "Unary base does not produce a value.");
    m_Value = m_Builder.build_not(m_Value, m_Opts.NamedMIR ? "bnot" : "");
}

void CGN::cgn_logic_not(UnaryExpr *UN) {
    assert(UN->getKind() == UnaryExpr::Kind::Logic_Not);

    m_VC = ValueContext::RValue;
    UN->getExpr()->accept(this);
    assert(m_Value && "Unary base does not produce a value.");

    m_Value = m_Builder.build_xor(inject_cmp(m_Value), 
        mir::ConstantInt::get(m_Segment, m_Builder.get_i1_ty(), 1), 
        m_Opts.NamedMIR ? "lnot" : "");
}

void CGN::cgn_neg(UnaryExpr *UN) {
    assert(UN->getKind() == UnaryExpr::Kind::Negate);

    m_VC = ValueContext::RValue;
    UN->getExpr()->accept(this);
    assert(m_Value && "Unary base does not produce a value.");

    switch (type_class(UN->getExpr()->getType())) {
    case SInt:
    case UInt:
    case Pointer:
        m_Value = m_Builder.build_neg(m_Value, m_Opts.NamedMIR ? "neg" : "");
        break;

    case Float:
        m_Value = m_Builder.build_fneg(m_Value, m_Opts.NamedMIR ? "fneg" : "");
        break;

    default:
        fatal("unsupported '-' operator on type: '" + 
            UN->getExpr()->getType()->getName() + "'", &UN->getMetadata());
    }
}

void CGN::cgn_addrof(UnaryExpr *UN) {
    assert(UN->getKind() == UnaryExpr::Kind::Address_Of);

    m_VC = ValueContext::LValue;
    UN->getExpr()->accept(this);
    assert(m_Value && "Unary base does not produce a value.");
}

void CGN::cgn_deref(UnaryExpr *UN) {
    assert(UN->getKind() == UnaryExpr::Kind::Dereference);

    ValueContext oldVC = m_VC;
    m_VC = ValueContext::RValue;
    UN->getExpr()->accept(this);
    assert(m_Value && "Unary base does not produce a value.");

    if (oldVC == ValueContext::RValue)
        m_Value = m_Builder.build_load(cgn_type(UN->getType()), m_Value, 
            m_Opts.NamedMIR ? "deref" : "");
}

void CGN::cgn_inc(UnaryExpr *UN) {
    assert(UN->getKind() == UnaryExpr::Kind::Increment);

    ValueContext oldVC = m_VC;
    m_VC = ValueContext::LValue;
    UN->getExpr()->accept(this);
    assert(m_Value && "Unary base does not produce a value.");
    mir::Value *addr = m_Value;
    mir::Value *og_val = m_Builder.build_load(cgn_type(UN->getExpr()->getType()), 
        addr, m_Opts.NamedMIR ? "inc.og" : "");
    mir::Value *inc = nullptr;

    switch (type_class(UN->getExpr()->getType())) {
    case SInt:
    case UInt:
        inc = m_Builder.build_add(og_val, mir::ConstantInt::get(m_Segment, 
            og_val->get_type(), 1));

        break;

    case Float:
        inc = m_Builder.build_fadd(og_val, mir::ConstantFP::get(m_Segment, 
            og_val->get_type(), 1.0));

        break;

    case Pointer:
        assert(false && "Pointer arithmetic not implemented.");
        break;

    default:
        break;
    }

    m_Builder.build_store(inc, addr);

    if (UN->isPostfix())
        m_Value = og_val;
    else if (UN->isPrefix())
        m_Value = inc;
}

void CGN::cgn_dec(UnaryExpr *UN) {
    assert(UN->getKind() == UnaryExpr::Kind::Decrement);

    ValueContext oldVC = m_VC;
    m_VC = ValueContext::LValue;
    UN->getExpr()->accept(this);
    assert(m_Value && "Unary base does not produce a value.");
    mir::Value *addr = m_Value;
    mir::Value *og_val = m_Builder.build_load(cgn_type(UN->getExpr()->getType()), 
        addr, m_Opts.NamedMIR ? "dec.og" : "");
    mir::Value *dec = nullptr;

    switch (type_class(UN->getExpr()->getType())) {
    case SInt:
    case UInt:
        dec = m_Builder.build_sub(og_val, mir::ConstantInt::get(m_Segment, 
            og_val->get_type(), 1));

        break;

    case Float:
        dec = m_Builder.build_fsub(og_val, mir::ConstantFP::get(m_Segment, 
            og_val->get_type(), 1.0));

        break;

    case Pointer:
        assert(false && "Pointer arithmetic not implemented.");
        break;

    default:
        break;
    }

    m_Builder.build_store(dec, addr);

    if (UN->isPostfix())
        m_Value = og_val;
    else if (UN->isPrefix())
        m_Value = dec;
}
