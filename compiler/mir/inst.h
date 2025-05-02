#ifndef MEDDLE_INST_H
#define MEDDLE_INST_H

#include "value.h"

#include <fstream>
#include <ostream>
#include <utility>

namespace mir {

class BasicBlock;

class Inst : public Value {
    friend class Builder;

protected:
    /// The parent basic block of this instruction.
    BasicBlock *m_Parent;

    /// Links to the previous and next instruction in the parent block.
    Inst *m_Prev = nullptr;
    Inst *m_Next = nullptr;

    Inst(BasicBlock *P);

    Inst(String N, Type *T, BasicBlock *P);

public:
    virtual ~Inst() = default;

    virtual bool is_terminator() const { return false; }

    virtual bool is_ret() const { return false; }

    virtual bool produces_value() const { return false; }

    BasicBlock *get_parent() const { return m_Parent; }

    void set_parent(BasicBlock *BB) { m_Parent = BB; }

    Inst *get_prev() const { return m_Prev; }

    Inst *get_next() const { return m_Next; }
    
    void set_prev(Inst *I) { m_Prev = I; }

    void set_next(Inst *I) { m_Next = I; }
};

class PHINode final : public Inst {
    friend class Builder;

    std::vector<std::pair<Value *, BasicBlock *>> m_Incoming;

    PHINode(
        String N,
        Type *T,
        BasicBlock *P
    ) : Inst(N, T, P), m_Incoming() {}

public:
    bool produces_value() const override { return true; }

    void add_incoming(Value *V, BasicBlock *BB) {
        assert(V->get_type() == this->m_Type && "PHI node type mismatch.");
        m_Incoming.emplace_back(V, BB);
    }

    const std::vector<std::pair<Value *, BasicBlock *>> &get_incoming() const 
    { return m_Incoming; }

    void print(std::ostream &OS) const override;
};

class StoreInst final : public Inst {
    friend class Builder;

    Value *m_Value;
    Value *m_Dest;
    ConstantInt *m_Offset;

    StoreInst(
        BasicBlock *P,
        Value *V,
        Value *D,
        ConstantInt *O = nullptr
    ) : Inst(P), m_Value(V), m_Dest(D), m_Offset(O) {}

public:
    Value *get_value() const { return m_Value; }

    Value *get_dest() const { return m_Dest; }

    bool has_offset() const { return m_Offset != nullptr; }

    ConstantInt *get_offset() const { return m_Offset; }
};

class LoadInst final : public Inst {
    friend class Builder;

    Value *m_Source;
    ConstantInt *m_Offset;

    LoadInst(
        String N,
        Type *T,
        BasicBlock *P,
        Value *S,
        ConstantInt *O = nullptr
    ) : Inst(N, T, P), m_Source(S), m_Offset(O) {}

public:
    bool produces_value() const override { return true; }

    Value *get_source() const { return m_Source; }

    bool has_offset() const { return m_Offset != nullptr; }

    ConstantInt *get_offset() const { return m_Offset; }

    void print(std::ostream &OS) const override;
};

class CpyInst final : public Inst {
    friend class Builder;

    Value *m_Source;
    unsigned m_SrcAlign;
    Value *m_Dest;
    unsigned m_DestAlign;
    unsigned m_Size;

    CpyInst(
        BasicBlock *P,
        Value *S,
        unsigned SAL,
        Value *D,
        unsigned DAL,
        unsigned Sz
    ) : Inst(P), m_Source(S), m_SrcAlign(SAL), m_Dest(D), m_DestAlign(DAL), 
        m_Size(Sz) {}

public:
    Value *get_source() const { return m_Source; }

    unsigned get_source_align() const { return m_SrcAlign; }

    Value *get_dest() const { return m_Dest; }

    unsigned get_dest_align() const { return m_DestAlign; }

    unsigned get_size() const { return m_Size; }
};

class BrifInst final : public Inst {
    friend class Builder;

    Value *m_Cond;
    BasicBlock *m_True;
    BasicBlock *m_False;

    BrifInst(BasicBlock *P, Value *C, BasicBlock *T, BasicBlock *F)
      : Inst(P), m_Cond(C), m_True(T), m_False(F) {}

public:
    bool is_terminator() const override { return true; }

    Value *get_cond() const { return m_Cond; }

    BasicBlock *get_true_dest() const { return m_True; }

    BasicBlock *get_false_dest() const { return m_False; }
};

class JMPInst final : public Inst {
    friend class Builder;

    BasicBlock *m_Dest;

    JMPInst(BasicBlock *P, BasicBlock *D) : Inst(P), m_Dest(D) {}

public:
    bool is_terminator() const override { return true; }

    BasicBlock *get_dest() const { return m_Dest; }
};

class RetInst final : public Inst {
    friend class Builder;

    Value *m_Value;

    RetInst(BasicBlock *P, Value *V = nullptr) : Inst(P), m_Value(V) {} 

public:
    bool is_terminator() const override { return true; }

    bool is_ret() const override { return true; }

    bool is_void() const { return m_Value == nullptr; }

    Value *get_value() const { return m_Value; }
};

class BinopInst final : public Inst {
    friend class Builder;

public:
    enum class Kind {
        Add,
        Sub,
        SMul,
        UMul,
        SDiv,
        UDiv,
        SRem,
        URem,

        FAdd,
        FSub,
        FMul,
        FDiv,

        And,
        Or,
        Xor,
        Shl,
        AShr,
        LShr,
    };

private:
    Kind m_Kind;
    Value *m_LVal;
    Value *m_RVal;

    BinopInst(String N, Type *T, BasicBlock *P, Kind K, Value *L, Value *R)
      : Inst(N, T, P), m_Kind(K), m_LVal(L), m_RVal(R) {}

public:
    Kind get_kind() const { return m_Kind; }

    Value *get_lval() const { return m_LVal; }

    Value *get_rval() const { return m_RVal; }

    void print(std::ostream &OS) const override;
};

class UnopInst final : public Inst {
    friend class Builder;

public:
    enum class Kind {
        SExt,
        ZExt,
        Trunc,
        FExt,
        FTrunc,
        SI2FP,
        UI2FP,
        FP2SI,
        FP2UI,
        Reint,
        Ptr2Int,
        Int2Ptr,
    };

private:
    Kind m_Kind;
    Value *m_Value;

    UnopInst(String N, Type *T, BasicBlock *P, Kind K, Value *V)
      : Inst(N, T, P), m_Kind(K), m_Value(V) {}

public:
    Kind get_kind() const { return m_Kind; }

    Value *get_value() const { return m_Value; }

    void print(std::ostream &OS) const override;
};

class CMPInst final : public Inst {
    friend class Builder;

public:
    enum class Kind {
        ICMP_EQ,
        ICMP_NE,
        ICMP_SLT,
        ICMP_ULT,
        ICMP_SLE,
        ICMP_ULE,
        ICMP_SGT,
        ICMP_UGT,
        ICMP_SGE,
        ICMP_UGE,
        
        FCMP_OEQ,
        FCMP_ONE,
        FCMP_OLT,
        FCMP_OLE,
        FCMP_OGT,
        FCMP_OGE,

        PCMP_EQ,
        PCMP_NE,
        PCMP_LT,
        PCMP_LE,
        PCMP_GT,
        PCMP_GE,
    };

private:
    Kind m_Kind;
    Value *m_LVal;
    Value *m_RVal;

    CMPInst(String N, Type *T, BasicBlock *P, Kind K, Value *LV, Value *RV)
      : Inst(N, T, P), m_Kind(K), m_LVal(LV), m_RVal(RV) {}

public:
    Kind get_kind() const { return m_Kind; }

    Value *get_lval() const { return m_LVal; }

    Value *get_rval() const { return m_RVal; }

    void print(std::ostream &OS) const override;
};

void clear_inst_dict();

} // namespace mir

#endif // MEDDLE_INST_H
