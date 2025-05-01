#include "basicblock.h"
#include "builder.h"
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
