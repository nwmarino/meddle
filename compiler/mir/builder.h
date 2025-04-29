#ifndef MEDDLE_BUILDER_H
#define MEDDLE_BUILDER_H

#include "inst.h"
#include "segment.h"
#include "type.h"

#include <string>

using String = std::string;

namespace mir {

class BasicBlock;
class Segment;

class Builder final {
    Segment *m_Segment;
    BasicBlock *m_Insert;
    unsigned long m_SSA;

    String get_ssa() { return std::to_string(m_SSA++); }

public:
    Builder(Segment *S) : m_Segment(S), m_Insert(nullptr), m_SSA(1) {}

    BasicBlock *get_insert() const { return m_Insert; }

    void set_insert(BasicBlock *BB) { m_Insert = BB; }

    Type *get_i1_ty() const { return m_Segment->m_Types.at("i1"); }

    Type *get_i8_ty() const { return m_Segment->m_Types.at("i8"); }

    Type *get_i16_ty() const { return m_Segment->m_Types.at("i16"); }

    Type *get_i32_ty() const { return m_Segment->m_Types.at("i32"); }

    Type *get_i64_ty() const { return m_Segment->m_Types.at("i64"); }

    Type *get_f32_ty() const { return m_Segment->m_Types.at("f32"); }

    Type *get_f64_ty() const { return m_Segment->m_Types.at("f64"); }

    SlotNode *build_slot(Type *T, String N = "");

    Value *build_load(Type *T, Value *S, String N = "");
    
    Value *build_load_offset(Type *T, Value *S, ConstantInt *O, String N = "");

    RetInst *build_ret_void();

    RetInst *build_ret(Value *V);

    StoreInst *build_store(Value *V, Value *D);

    StoreInst *build_store_offset(Value *V, Value *D, ConstantInt *O);
};

} // namespace mir

#endif // MEDDLE_BUILDER_H
