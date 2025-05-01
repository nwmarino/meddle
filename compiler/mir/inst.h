#ifndef MEDDLE_INST_H
#define MEDDLE_INST_H

#include "value.h"
#include <fstream>

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
    ~StoreInst() override {
        if (m_Value->is_constant())
            delete m_Value;

        if (m_Offset)
            delete m_Offset;
    }

    Value *get_value() const { return m_Value; }

    Value *get_dest() const { return m_Dest; }

    bool has_offset() const { return m_Offset != nullptr; }

    ConstantInt *get_offset() const { return m_Offset; }

    void print(std::ostream &OS) const override;
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
    ~LoadInst() override {
        if (m_Offset)
            delete m_Offset;
    }

    bool produces_value() const override { return true; }

    Value *get_source() const { return m_Source; }

    bool has_offset() const { return m_Offset != nullptr; }

    ConstantInt *get_offset() const { return m_Offset; }

    void print(std::ostream &OS) const override;
};

class BrifInst final : public Inst {
    friend class Builder;

    Value *m_Cond;
    BasicBlock *m_True;
    BasicBlock *m_False;

    BrifInst(BasicBlock *P, Value *C, BasicBlock *T, BasicBlock *F)
      : Inst(P), m_Cond(C), m_True(T), m_False(F) {}

public:
    ~BrifInst() override {
        if (m_Cond->is_constant())
            delete m_Cond;
    }

    bool is_terminator() const override { return true; }

    Value *get_cond() const { return m_Cond; }

    BasicBlock *get_true_dest() const { return m_True; }

    BasicBlock *get_false_dest() const { return m_False; }

    void print(std::ostream &OS) const override;
};

class JMPInst final : public Inst {
    friend class Builder;

    BasicBlock *m_Dest;

    JMPInst(BasicBlock *P, BasicBlock *D) : Inst(P), m_Dest(D) {}

public:
    bool is_terminator() const override { return true; }

    BasicBlock *get_dest() const { return m_Dest; }

    void print(std::ostream &OS) const override;
};

class RetInst final : public Inst {
    friend class Builder;

    Value *m_Value;

    RetInst(BasicBlock *P, Value *V = nullptr) : Inst(P), m_Value(V) {} 

public:
    ~RetInst() override {
        if (m_Value && m_Value->is_constant())
            delete m_Value;
    }

    bool is_terminator() const override { return true; }

    bool is_ret() const override { return true; }

    bool is_void() const { return m_Value == nullptr; }

    Value *get_value() const { return m_Value; }

    void print(std::ostream &OS) const override;
};

void clear_inst_dict();

} // namespace mir

#endif // MEDDLE_INST_H
