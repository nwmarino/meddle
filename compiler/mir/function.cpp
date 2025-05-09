#include "basicblock.h"
#include "function.h"
#include "inst.h"
#include "segment.h"
#include "value.h"

using namespace mir;

Function::Function(String N, FunctionType *FT, Linkage L, Segment *P, 
                   std::vector<Argument *> Args) 
    : Value(N, FT), m_Linkage(L), m_Parent(P), m_Args(Args) 
{
    if (m_Parent)
        m_Parent->add_function(this);
};

Function::~Function() {
    for (auto &[ String, slot ] : m_Slots)
        delete slot;

    for (BasicBlock *curr = m_Tail; curr != nullptr; ) {
        BasicBlock *prev = curr->get_prev();
        delete curr;
        curr = prev;
    }
}

void Function::add_slot(Slot *S) {
    assert(get_slot(S->get_name()) == nullptr && 
           "Slot with name already exists.");

    m_Slots[S->get_name()] = S;
}

Slot *Function::get_slot(String N) const {
    auto it = m_Slots.find(N);
    if (it != m_Slots.end())
        return it->second;

    return nullptr;
}

std::vector<Slot *> Function::get_slots() const {
    std::vector<Slot *> slots;
    slots.reserve(m_Slots.size());

    for (auto &[String, slot] : m_Slots)
        slots.push_back(slot);

    return slots;
}

void Function::append(BasicBlock *BB) {
    assert(BB && "BasicBlock cannot be null.");

    if (!m_Head && !m_Tail) {
        m_Head = BB;
        m_Tail = BB;
    } else {
        m_Tail->set_next(BB);
        BB->set_prev(m_Tail);
        BB->set_next(nullptr);
        m_Tail = BB;
    }

    BB->set_parent(this);

    if (BB->get_name().empty())
        BB->give_ssa(m_Parent);
}

void Function::prepend(BasicBlock *BB) {
    assert(BB && "BasicBlock cannot be null.");
    
    if (!m_Head && !m_Tail) {
        m_Head = BB;
        m_Tail = BB;
    } else {
        m_Head->set_prev(BB);
        BB->set_next(m_Head);
        BB->set_prev(nullptr);
        m_Head = BB;
    }

    BB->set_parent(this);

    if (BB->get_name().empty())
        BB->give_ssa(m_Parent);
}

void Function::detach() {
    assert(m_Parent && "Function has no parent.");
    m_Parent->remove_function(this);
}
