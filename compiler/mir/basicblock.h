#ifndef MEDDLE_BASICBLOCK_H
#define MEDDLE_BASICBLOCK_H

#include "value.h"

namespace mir {

class Function;

class BasicBlock final : public Value {
    /// The parent function of this block.
    Function *m_Parent;

    /// The head and tail instructions of this block.
    Inst *m_Head = nullptr;
    Inst *m_Tail = nullptr;
    
    /// Links to the previous and next blocks in the parent function.
    BasicBlock *m_Prev = nullptr;
    BasicBlock *m_Next = nullptr;

    /// Links to the predecessors and successors of this block.
    std::vector<BasicBlock *> m_Preds = {};
    std::vector<BasicBlock *> m_Succs = {};

public:
    BasicBlock(String N, Function *P = nullptr);

    ~BasicBlock() override;

    Function *get_parent() const { return m_Parent; }

    void set_parent(Function *F) { m_Parent = F; }

    BasicBlock *get_prev() const { return m_Prev; }

    BasicBlock *get_next() const { return m_Next; }

    void set_prev(BasicBlock *BB) { m_Prev = BB; }

    void set_next(BasicBlock *BB) { m_Next = BB; }

    bool is_empty() const { return m_Head == nullptr; }

    bool has_preds() const { return !m_Preds.empty(); }

    bool has_succs() const { return !m_Succs.empty(); }

    Inst *head() const { return m_Head; }

    Inst *tail() const { return m_Tail; }

    void set_head(Inst *I) { m_Head = I; }

    void set_tail(Inst *I) { m_Tail = I; }

    void append(Inst *I);

    void prepend(Inst *I);

    /// Add a new successor to this block.
    void add_succ(BasicBlock *BB);

    /// Add a new predecessor to this block.
    void add_pred(BasicBlock *BB);

    /// \returns `true` if this block has at least one terminator.
    bool has_terminator() const;

    /// \returns The number of terminators in this block.
    unsigned terminators() const;

    /// Detach this block from its parent function and delete it.
    void detach();

    void print(std::ofstream &OS) const override;
};

} // namespace mir

#endif // MEDDLE_BASICBLOCK_H
