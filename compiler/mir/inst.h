#ifndef MEDDLE_INST_H
#define MEDDLE_INST_H

#include "value.h"

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

class SlotNode final : public Inst {
    friend class Builder;

    Type *m_Alloc;
    unsigned m_Align;
    bool m_Implicit;

    SlotNode(
        String N,
        Type *T,
        BasicBlock *P,
        Type *A,
        unsigned AL,
        bool I = false
    ) : Inst(N, T, P), m_Alloc(A), m_Align(AL), m_Implicit(I) {}

public:
    bool produces_value() const override { return true; }

    Type *get_alloc_type() const { return m_Type; }

    unsigned get_align() const { return m_Align; }

    bool is_implicit() const { return m_Implicit; }

    void print(std::ofstream &OS) const override;
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

        delete m_Dest;

        if (m_Offset)
            delete m_Offset;
    }

    Value *get_value() const { return m_Value; }

    Value *get_dest() const { return m_Dest; }

    bool has_offset() const { return m_Offset != nullptr; }

    ConstantInt *get_offset() const { return m_Offset; }

    void print(std::ofstream &OS) const override;
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
        delete m_Source;

        if (m_Offset)
            delete m_Offset;
    }

    bool produces_value() const override { return true; }

    Value *get_source() const { return m_Source; }

    bool has_offset() const { return m_Offset != nullptr; }

    ConstantInt *get_offset() const { return m_Offset; }

    void print(std::ofstream &OS) const override;
};

class RetInst final : public Inst {
    friend class Builder;

    Value *m_Value;

    RetInst(BasicBlock *P, Value *V = nullptr) : Inst(P), m_Value(V) {} 

public:
    ~RetInst() override {
        if (m_Value)
            delete m_Value;
    }

    bool is_terminator() const override { return true; }

    bool is_ret() const override { return true; }

    bool is_void() const { return m_Value == nullptr; }

    Value *get_value() const { return m_Value; }

    void print(std::ofstream &OS) const override;
};

} // namespace mir

#endif // MEDDLE_INST_H
