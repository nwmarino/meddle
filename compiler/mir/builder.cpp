#include "builder.h"
#include "type.h"
#include "value.h"

using namespace mir;

SlotNode *Builder::build_slot(Type *T, String N) {
    SlotNode *slot = new SlotNode(N.empty() ? get_ssa() : N, 
        PointerType::get(m_Segment, T), m_Insert, T, 8);

    return slot;
}

Value *Builder::build_load(Type *T, Value *S, String N) {
    return build_load_offset(T, S, nullptr, N);
}
    
Value *Builder::build_load_offset(Type *T, Value *S, ConstantInt *O, String N) {
    LoadInst *load = new LoadInst(N.empty() ? get_ssa() : N, T, m_Insert, S, O);
    S->add_use(load);
    return load;
}

RetInst *Builder::build_ret_void() {
    return new RetInst(m_Insert);
}

RetInst *Builder::build_ret(Value *V) {
    RetInst *ret = new RetInst(m_Insert, V);
    V->add_use(ret);
    return ret;
}

StoreInst *Builder::build_store(Value *V, Value *D) {
    return build_store_offset(V, D, nullptr);
}

StoreInst *Builder::build_store_offset(Value *V, Value *D, ConstantInt *O) {
    StoreInst *store = new StoreInst(m_Insert, V, D, O);
    V->add_use(store);
    D->add_use(store);
    return store;
}
