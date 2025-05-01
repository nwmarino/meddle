#include "basicblock.h"
#include "function.h"
#include "inst.h"
#include "segment.h"
#include "value.h"

using namespace mir;

void Argument::print(std::ostream &OS) const {
    OS << get_name() << " : " << get_type()->get_name();
    if (m_Slot)
        OS << " = " << m_Slot->get_name();
}

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
}

void Function::detach() {
    assert(m_Parent && "Function has no parent.");
    m_Parent->remove_function(this);
}

void Function::print(std::ostream &OS) const {
    
}
