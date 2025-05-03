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

public:
    Builder(Segment *S) : m_Segment(S), m_Insert(nullptr) {}

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

    PHINode *build_phi(Type *T, String N = "");

    Value *build_ap(Type *T, Value *S, Value *Idx, String N = "");

    StoreInst *build_store(Value *V, Value *D);

    StoreInst *build_store_offset(Value *V, Value *D, ConstantInt *O);

    Value *build_load(Type *T, Value *S, String N = "");
    
    Value *build_load_offset(Type *T, Value *S, ConstantInt *O, String N = "");

    CpyInst *build_cpy(Value *D, unsigned DAL, Value *S, unsigned SAL, Value *Sz);

    BrifInst *build_brif(Value *C, BasicBlock *T, BasicBlock *F);

    JMPInst *build_jmp(BasicBlock *D);

    RetInst *build_ret_void();

    RetInst *build_ret(Value *V);

    CallInst *build_call(Function *C, std::vector<Value *> &Args, String N = "");

    Value *build_add(Value *LV, Value *RV, String N = "");

    Value *build_sub(Value *LV, Value *RV, String N = "");

    Value *build_smul(Value *LV, Value *RV, String N = "");

    Value *build_umul(Value *LV, Value *RV, String N = "");

    Value *build_sdiv(Value *LV, Value *RV, String N = "");

    Value *build_udiv(Value *LV, Value *RV, String N = "");

    Value *build_srem(Value *LV, Value *RV, String N = "");

    Value *build_urem(Value *LV, Value *RV, String N = "");

    Value *build_fadd(Value *LV, Value *RV, String N = "");

    Value *build_fsub(Value *LV, Value *RV, String N = "");

    Value *build_fmul(Value *LV, Value *RV, String N = "");

    Value *build_fdiv(Value *LV, Value *RV, String N = "");

    Value *build_and(Value *LV, Value *RV, String N = "");

    Value *build_or(Value *LV, Value *RV, String N = "");

    Value *build_xor(Value *LV, Value *RV, String N = "");

    Value *build_shl(Value *LV, Value *RV, String N = "");

    Value *build_lshr(Value *LV, Value *RV, String N = "");

    Value *build_ashr(Value *LV, Value *RV, String N = "");

    Value *build_not(Value *V, String N = "");

    Value *build_neg(Value *V, String N = "");

    Value *build_fneg(Value *V, String N = "");

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

    Value *build_ptr2int(Value *V, Type *D, String N = "");

    Value *build_int2ptr(Value *V, Type *D, String N = "");
    
    Value *build_icmp_eq(Value *LV, Value *RV, String N = "");

    Value *build_icmp_ne(Value *LV, Value *RV, String N = "");

    Value *build_icmp_slt(Value *LV, Value *RV, String N = "");

    Value *build_icmp_sle(Value *LV, Value *RV, String N = "");

    Value *build_icmp_sgt(Value *LV, Value *RV, String N = "");

    Value *build_icmp_sge(Value *LV, Value *RV, String N = "");

    Value *build_icmp_ult(Value *LV, Value *RV, String N = "");

    Value *build_icmp_ule(Value *LV, Value *RV, String N = "");

    Value *build_icmp_ugt(Value *LV, Value *RV, String N = "");

    Value *build_icmp_uge(Value *LV, Value *RV, String N = "");

    Value *build_fcmp_oeq(Value *LV, Value *RV, String N = "");

    Value *build_fcmp_one(Value *LV, Value *RV, String N = "");

    Value *build_fcmp_olt(Value *LV, Value *RV, String N = "");

    Value *build_fcmp_ole(Value *LV, Value *RV, String N = "");

    Value *build_fcmp_ogt(Value *LV, Value *RV, String N = "");

    Value *build_fcmp_oge(Value *LV, Value *RV, String N = "");

    Value *build_pcmp_eq(Value *LV, Value *RV, String N = "");

    Value *build_pcmp_ne(Value *LV, Value *RV, String N = "");

    Value *build_pcmp_lt(Value *LV, Value *RV, String N = "");

    Value *build_pcmp_le(Value *LV, Value *RV, String N = "");

    Value *build_pcmp_gt(Value *LV, Value *RV, String N = "");

    Value *build_pcmp_ge(Value *LV, Value *RV, String N = "");
};

} // namespace mir

#endif // MEDDLE_BUILDER_H
