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

    Type *get_void_ty() const { return m_Segment->m_Types.at("void"); }

    Slot *build_slot(Type *T, String N = "", Function *P = nullptr);

    StoreInst *build_store(Value *V, Value *D);

    StoreInst *build_store_offset(Value *V, Value *D, ConstantInt *O);

    Value *build_load(Type *T, Value *S, String N = "");
    
    Value *build_load_offset(Type *T, Value *S, ConstantInt *O, String N = "");

    CpyInst *build_cpy(Value *D, unsigned DAL, Value *S, unsigned SAL, unsigned Sz);

    BrifInst *build_brif(Value *C, BasicBlock *T, BasicBlock *F);

    JMPInst *build_jmp(BasicBlock *D);

    RetInst *build_ret_void();

    RetInst *build_ret(Value *V);

    Value *build_sext(Value *V, Type *D, String N = "");

    Value *build_zext(Value *V, Type *D, String N = "");

    Value *build_trunc(Value *V, Type *D, String N = "");

    Value *build_fext(Value *V, Type *D, String N = "");

    Value *build_ftrunc(Value *V, Type *D, String N = "");

    Value *build_si2fp(Value *V, Type *D, String N = "");

    Value *build_ui2fp(Value *V, Type *D, String N = "");

    Value *build_fp2si(Value *V, Type *D, String N = "");

    Value *build_fp2ui(Value *V, Type *D, String N = "");

    Value *build_reint(Value *V, Type *D, String N = "");
};

} // namespace mir

#endif // MEDDLE_BUILDER_H
