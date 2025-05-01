#include "basicblock.h"
#include "builder.h"
#include "inst.h"
#include "type.h"
#include "value.h"

using namespace mir;

Slot *Builder::build_slot(Type *T, String N, Function *P) {
    if (!P)
        assert(m_Insert && "No insertion point set.");

    assert(T && "Slot type cannot be null.");

    Slot *slot = new Slot(N.empty() ? get_ssa() : N, 
        PointerType::get(m_Segment, T), P ? P : m_Insert->get_parent(), T, 
        m_Segment->get_data_layout().get_type_align(T));

    return slot;
}

StoreInst *Builder::build_store(Value *V, Value *D) {
    return build_store_offset(V, D, nullptr);
}

StoreInst *Builder::build_store_offset(Value *V, Value *D, ConstantInt *O) {
    assert(m_Insert && "No insertion point set.");
    assert(D && "Store destination cannot be null.");
    assert(D->get_type()->is_pointer_ty() && "Store destination must be a place.");

    StoreInst *store = new StoreInst(m_Insert, V, D, O);
    V->add_use(store);
    D->add_use(store);
    return store;
}

Value *Builder::build_load(Type *T, Value *S, String N) {
    return build_load_offset(T, S, nullptr, N);
}

Value *Builder::build_load_offset(Type *T, Value *S, ConstantInt *O, String N) {
    assert(m_Insert && "No insertion point set.");
    assert(S && "Load source cannot be null.");

    LoadInst *load = new LoadInst(N.empty() ? get_ssa() : N, T, m_Insert, S, O);
    S->add_use(load);
    return load;
}

CpyInst *Builder::build_cpy(Value *D, unsigned DAL, Value *S, unsigned SAL, 
                            unsigned Sz) {
    assert(m_Insert && "No insertion point set.");
    assert(D && "Copy destination cannot be null.");
    assert(D->get_type()->is_pointer_ty() && "Copy destination must be a place.");
    assert(S && "Copy source cannot be null.");
    assert(S->get_type()->is_pointer_ty() && "Copy source must be a place.");
    assert(Sz > 0 && "Copy size must be greater than zero.");

    CpyInst *cpy = new CpyInst(m_Insert, S, SAL, D, DAL, Sz);
    D->add_use(cpy);
    S->add_use(cpy);
    return cpy;
}

BrifInst *Builder::build_brif(Value *C, BasicBlock *T, BasicBlock *F) {
    assert(m_Insert && "No insertion point set.");
    assert(C && "'brif' condition cannot be null.");
    assert(T && "'brif' true block cannot be null.");
    assert(F && "'brif' false block cannot be null.");
    
    BrifInst *BR = new BrifInst(m_Insert, C, T, F);
    T->add_pred(m_Insert);
    F->add_pred(m_Insert);
    T->add_use(BR);
    F->add_use(BR);
    m_Insert->add_succ(T);
    m_Insert->add_succ(F);
    return BR;
}

JMPInst *Builder::build_jmp(BasicBlock *D) {
    assert(m_Insert && "No insertion point set.");
    assert(D && "'jmp' destination cannot be null.");

    JMPInst *J = new JMPInst(m_Insert, D);
    D->add_pred(m_Insert);
    D->add_use(J);
    m_Insert->add_succ(D);
    return J;
}

RetInst *Builder::build_ret_void() {
    assert(m_Insert && "No insertion point set.");

    return new RetInst(m_Insert);
}

RetInst *Builder::build_ret(Value *V) {
    assert(m_Insert && "No insertion point set.");
    
    if (!V)
        return build_ret_void();

    RetInst *ret = new RetInst(m_Insert, V);
    V->add_use(ret);
    return ret;
}

Value *Builder::build_sext(Value *V, Type *D, String N) {
    assert(m_Insert && "No insertion point set.");
    assert(V && "Sign extend source cannot be null.");
    assert(D && "Sign extend destination cannot be null.");
    assert(V->get_type()->is_integer_ty() && "Sign extend source must be an integer.");
    assert(D->is_integer_ty() && "Sign extend destination must be an integer.");

    DataLayout DL = m_Segment->get_data_layout();
    assert(DL.get_type_size(V->get_type()) < DL.get_type_size(D) && 
           "Sign extend destination must be larger than source.");

    UnopInst *ext = new UnopInst(N.empty() ? get_ssa() : N, D, m_Insert,
        UnopInst::Kind::SExt, V);
    V->add_use(ext);
    return ext;
}

Value *Builder::build_zext(Value *V, Type *D, String N) {
    assert(m_Insert && "No insertion point set.");
    assert(V && "Zero extend source cannot be null.");
    assert(D && "Zero extend destination cannot be null.");
    assert(V->get_type()->is_integer_ty() && "Zero extend source must be an integer.");
    assert(D->is_integer_ty() && "Zero extend destination must be an integer.");

    DataLayout DL = m_Segment->get_data_layout();
    assert(DL.get_type_size(V->get_type()) < DL.get_type_size(D) && 
           "Zero extend destination must be larger than source.");

    UnopInst *ext = new UnopInst(N.empty() ? get_ssa() : N, D, m_Insert,
        UnopInst::Kind::ZExt, V);
    V->add_use(ext);
    return ext;
}

Value *Builder::build_trunc(Value *V, Type *D, String N) {
    assert(m_Insert && "No insertion point set.");
    assert(V && "Truncate source cannot be null.");
    assert(D && "Truncate destination cannot be null.");
    assert(V->get_type()->is_integer_ty() && "Truncate source must be an integer.");
    assert(D->is_integer_ty() && "Truncate destination must be an integer.");

    DataLayout DL = m_Segment->get_data_layout();
    assert(DL.get_type_size(V->get_type()) > DL.get_type_size(D) && 
           "Truncate destination must be smaller than source.");

    UnopInst *trunc = new UnopInst(N.empty() ? get_ssa() : N, D, m_Insert,
        UnopInst::Kind::Trunc, V);
    V->add_use(trunc);
    return trunc;
}

Value *Builder::build_fext(Value *V, Type *D, String N) {
    assert(m_Insert && "No insertion point set.");
    assert(V && "Floating point extend source cannot be null.");
    assert(D && "Floating point extend destination cannot be null.");
    assert(V->get_type()->is_float_ty() && 
           "Floating point extend source must be a floating point type.");
    assert(D->is_float_ty() && 
           "Floating point extend destination must be a floating point type.");

    UnopInst *ext = new UnopInst(N.empty() ? get_ssa() : N, D, m_Insert,
        UnopInst::Kind::FExt, V);
    V->add_use(ext);
    return ext;
}

Value *Builder::build_ftrunc(Value *V, Type *D, String N) {
    assert(m_Insert && "No insertion point set.");
    assert(V && "Floating point truncate source cannot be null.");
    assert(D && "Floating point truncate destination cannot be null.");
    assert(V->get_type()->is_float_ty() && 
           "Floating point truncate source must be a floating point type.");
    assert(D->is_float_ty() && 
           "Floating point truncate destination must be a floating point type.");

    UnopInst *trunc = new UnopInst(N.empty() ? get_ssa() : N, D, m_Insert,
        UnopInst::Kind::FTrunc, V);
    V->add_use(trunc);
    return trunc;
}

Value *Builder::build_si2fp(Value *V, Type *D, String N) {
    assert(m_Insert && "No insertion point set.");
    assert(V && "Signed integer to floating point source cannot be null.");
    assert(D && "Signed integer to floating point destination cannot be null.");
    assert(V->get_type()->is_integer_ty() && 
           "Signed integer to floating point source must be an integer.");
    assert(D->is_float_ty() && 
           "Signed integer to floating point destination must be a floating point type.");

    UnopInst *ext = new UnopInst(N.empty() ? get_ssa() : N, D, m_Insert,
        UnopInst::Kind::SI2FP, V);
    V->add_use(ext);
    return ext;
}

Value *Builder::build_ui2fp(Value *V, Type *D, String N) {
    assert(m_Insert && "No insertion point set.");
    assert(V && "Unsigned integer to floating point source cannot be null.");
    assert(D && "Unsigned integer to floating point destination cannot be null.");
    assert(V->get_type()->is_integer_ty() && 
           "Unsigned integer to floating point source must be an integer.");
    assert(D->is_float_ty() && 
           "Unsigned integer to floating point destination must be a floating point type.");

    UnopInst *ext = new UnopInst(N.empty() ? get_ssa() : N, D, m_Insert,
        UnopInst::Kind::UI2FP, V);
    V->add_use(ext);
    return ext;
}

Value *Builder::build_fp2si(Value *V, Type *D, String N) {
    assert(m_Insert && "No insertion point set.");
    assert(V && "Floating point to signed integer source cannot be null.");
    assert(D && "Floating point to signed integer destination cannot be null.");
    assert(V->get_type()->is_float_ty() && 
           "Floating point to signed integer source must be a floating point type.");
    assert(D->is_integer_ty() && 
           "Floating point to signed integer destination must be an integer.");

    UnopInst *ext = new UnopInst(N.empty() ? get_ssa() : N, D, m_Insert,
        UnopInst::Kind::FP2SI, V);
    V->add_use(ext);
    return ext;
}

Value *Builder::build_fp2ui(Value *V, Type *D, String N) {
    assert(m_Insert && "No insertion point set.");
    assert(V && "Floating point to unsigned integer source cannot be null.");
    assert(D && "Floating point to unsigned integer destination cannot be null.");
    assert(V->get_type()->is_float_ty() && 
           "Floating point to unsigned integer source must be a floating point type.");
    assert(D->is_integer_ty() && 
           "Floating point to unsigned integer destination must be an integer.");

    UnopInst *ext = new UnopInst(N.empty() ? get_ssa() : N, D, m_Insert,
        UnopInst::Kind::FP2UI, V);
    V->add_use(ext);
    return ext;
}

Value *Builder::build_reint(Value *V, Type *D, String N) {
    assert(m_Insert && "No insertion point set.");
    assert(V && "Reinterpret source cannot be null.");
    assert(D && "Reinterpret destination cannot be null.");
    assert(V->get_type()->is_pointer_ty() && 
           "Reinterpret source must be a pointer type.");
    assert(D->is_pointer_ty() && 
           "Reinterpret destination must be a pointer type.");

    UnopInst *ext = new UnopInst(N.empty() ? get_ssa() : N, D, m_Insert,
        UnopInst::Kind::Reint, V);
    V->add_use(ext);
    return ext;
}

Value *Builder::build_cmp_ieq(Value *LV, Value *RV, String N) {
    assert(m_Insert && "No insertion point set.");
    assert(LV && "Compare left value cannot be null.");
    assert(RV && "Compare right value cannot be null.");
    assert((LV->get_type()->is_integer_ty() || LV->get_type()->is_pointer_ty()) 
           && "Compare 'ieq' left value must be an integer.");
    assert((RV->get_type()->is_integer_ty() || RV->get_type()->is_pointer_ty()) 
           && "Compare 'ieq' right value must be an integer.");

    CMPInst *cmp = new CMPInst(N.empty() ? get_ssa() : N, get_i1_ty(), m_Insert,
        CMPInst::Kind::CMP_IEQ, LV, RV);
    LV->add_use(cmp);
    RV->add_use(cmp);
    return cmp;
}

Value *Builder::build_cmp_ine(Value *LV, Value *RV, String N) {
    assert(m_Insert && "No insertion point set.");
    assert(LV && "Compare left value cannot be null.");
    assert(RV && "Compare right value cannot be null.");
    assert((LV->get_type()->is_integer_ty() || LV->get_type()->is_pointer_ty()) 
           && "Compare 'ine' left value must be an integer.");
    assert((RV->get_type()->is_integer_ty() || RV->get_type()->is_pointer_ty()) 
           && "Compare 'ine' right value must be an integer.");

    CMPInst *cmp = new CMPInst(N.empty() ? get_ssa() : N, get_i1_ty(), m_Insert,
        CMPInst::Kind::CMP_INE, LV, RV);
    LV->add_use(cmp);
    RV->add_use(cmp);
    return cmp;
}

Value *Builder::build_cmp_foeq(Value *LV, Value *RV, String N) {
    assert(m_Insert && "No insertion point set.");
    assert(LV && "Compare left value cannot be null.");
    assert(RV && "Compare right value cannot be null.");
    assert(LV->get_type()->is_float_ty() && 
           "Compare 'foeq' left value must be a floating point type.");
    assert(RV->get_type()->is_float_ty() && 
           "Compare 'foeq' right value must be a floating point type.");

    CMPInst *cmp = new CMPInst(N.empty() ? get_ssa() : N, get_i1_ty(), m_Insert,
        CMPInst::Kind::CMP_FOEQ, LV, RV);
    LV->add_use(cmp);
    RV->add_use(cmp);
    return cmp;
}

Value *Builder::build_cmp_fone(Value *LV, Value *RV, String N) {
    assert(m_Insert && "No insertion point set.");
    assert(LV && "Compare left value cannot be null.");
    assert(RV && "Compare right value cannot be null.");
    assert(LV->get_type()->is_float_ty() && 
           "Compare 'fone' left value must be a floating point type.");
    assert(RV->get_type()->is_float_ty() && 
           "Compare 'fone' right value must be a floating point type.");

    CMPInst *cmp = new CMPInst(N.empty() ? get_ssa() : N, get_i1_ty(), m_Insert,
        CMPInst::Kind::CMP_FONE, LV, RV);
    LV->add_use(cmp);
    RV->add_use(cmp);
    return cmp;
}
