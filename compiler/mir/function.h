#ifndef MEDDLE_FUNCTION_H
#define MEDDLE_FUNCTION_H

#include "type.h"
#include "value.h"

#include <fstream>
#include <unordered_map>

namespace mir {

class BasicBlock;
class Function;
class Segment;
class SlotNode;

struct Attributes final {

};

class Argument final : public Value {
    Attributes attrs;

    /// The parent function of this argument.
    Function *m_Parent;

    /// The index of this argument in the parent function.
    unsigned m_Index;

    /// The corresponding function slot allocated for this argument.
    ///
    /// This might not always exist.
    Slot *m_Slot;

public:
    Argument(String N, Type *T, Function *P, unsigned I, Slot *S = nullptr)
        : Value(N, T), m_Parent(P), m_Index(I), m_Slot(S) {}

    Function *get_parent() const { return m_Parent; }

    unsigned get_index() const { return m_Index; }

    Slot *get_slot() const { return m_Slot; }

    void print(std::ostream &OS) const override;
};

class Function final : public Value {
public:
    enum class Linkage {
        Internal,
        External,
    };

private:
    Attributes attrs;

    Linkage m_Linkage;

    /// The parent segment of this function.
    Segment *m_Parent;

    /// The arguments to this function.
    std::vector<Argument *> m_Args;

    /// The slot nodes of this function.
    std::unordered_map<String, Slot *> m_Slots = {};

    /// The head and tail blocks of this function.
    BasicBlock *m_Head = nullptr;
    BasicBlock *m_Tail = nullptr;

public:
    Function(String N, FunctionType *FT, Linkage L, Segment *P, 
             std::vector<Argument *> Args) 
        : Value(N, FT), m_Linkage(L), m_Parent(P), m_Args(Args) {};

    ~Function() override;

    Segment *get_parent() const { return m_Parent; }

    BasicBlock *head() const { return m_Head; }

    BasicBlock *tail() const { return m_Tail; }

    void set_head(BasicBlock *BB) { m_Head = BB; }

    void set_tail(BasicBlock *BB) { m_Tail = BB; }

    void add_slot(Slot *S);

    Slot *get_slot(String N) const;

    std::vector<Slot *> get_slots() const;

    const std::vector<Argument *> &get_args() const { return m_Args; }

    Argument *get_arg(unsigned i) const {
        assert(i < m_Args.size() && "Argument index out of bounds.");
        return m_Args[i];
    }

    void set_args(std::vector<Argument *> Args) { m_Args = Args; }

    void append(BasicBlock *BB);

    void prepend(BasicBlock *BB);
    
    /// Detach this function from its parent segment and delete it.
    void detach();

    void print(std::ostream &OS) const override;
};

} // namespace mir

#endif // MEDDLE_FUNCTION_H
