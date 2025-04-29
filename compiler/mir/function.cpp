#include "basicblock.h"
#include "function.h"
#include "inst.h"
#include "segment.h"
#include <fstream>

using namespace mir;

void Argument::print(std::ofstream &OS) const {
    OS << get_name() << " : " << get_type()->get_name();
    if (m_Slot)
        OS << " = " << m_Slot->get_name();
}

Function::~Function() {
    for (auto &[ String, slot ] : m_Slots)
        delete slot;

    for (BasicBlock *curr = m_Head; curr != nullptr; ) {
        BasicBlock *next = curr->get_next();
        delete curr;
        curr = next;
    }
}

void Function::add_slot(SlotNode *S) {
    assert(get_slot(S->get_name()) == nullptr && 
           "Slot with name already exists.");

    m_Slots[S->get_name()] = S;
}

SlotNode *Function::get_slot(String N) const {
    auto it = m_Slots.find(N);
    if (it != m_Slots.end())
        return it->second;

    return nullptr;
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

void Function::print(std::ofstream &OS) const {
    OS << m_Name << " :: (";
    for (auto &A : m_Args) {

    }

    OS << ")";
    if (!m_Head) {
        OS << ";\n";
        return;
    }

    OS << " {\n";
    for (BasicBlock *curr = m_Head; curr != nullptr; curr = curr->get_next()) {
        curr->print(OS);
        if (curr->get_next())
            OS << "\n";
    }

    OS << "}\n";
}
