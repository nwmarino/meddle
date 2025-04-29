#include "basicblock.h"
#include "function.h"
#include "inst.h"

using namespace mir;

BasicBlock::BasicBlock(String N, Function *P) : Value(N, nullptr), m_Parent(P) {
    if (P)
        P->append(this);
}

BasicBlock::~BasicBlock() {
    for (Inst *curr = m_Head; curr != nullptr; ) {
        Inst *next = curr->get_next();
        delete curr;
        curr = next;
    }
}

void BasicBlock::append(Inst *I) {
    assert(I && "Instruction cannot be null.");

    if (!m_Head && !m_Tail) {
        m_Head = I;
        m_Tail = I;
    } else {
        m_Tail->set_next(I);
        I->set_prev(m_Tail);
        I->set_next(nullptr);
        m_Tail = I;
    }
    
    I->set_parent(this);
}

void BasicBlock::prepend(Inst *I) {
    assert(I && "Instruction cannot be null.");

    if (!m_Head && !m_Tail) {
        m_Head = I;
        m_Tail = I;
    } else {
        m_Head->set_prev(I);
        I->set_next(m_Head);
        I->set_prev(nullptr);
        m_Head = I;
    }
}

void BasicBlock::add_succ(BasicBlock *BB) {
    assert(BB && "Basic block cannot be null.");
    assert(BB != this && "Cannot add itself as a successor.");

    if (std::find(m_Succs.begin(), m_Succs.end(), BB) == m_Succs.end()) {
        m_Succs.push_back(BB);
        BB->add_pred(this);
    }
}

void BasicBlock::add_pred(BasicBlock *BB) {
    assert(BB && "Basic block cannot be null.");
    assert(BB != this && "Cannot add itself as a predecessor.");

    if (std::find(m_Preds.begin(), m_Preds.end(), BB) == m_Preds.end()) {
        m_Preds.push_back(BB);
        BB->add_succ(this);
    }
}

bool BasicBlock::has_terminator() const {
    for (Inst *curr = m_Tail; curr != nullptr; curr = curr->get_prev())
        if (curr->is_terminator())
            return true;
    
    return false;
}

unsigned BasicBlock::terminators() const {
    unsigned terms = 0;
    for (Inst *curr = m_Head; curr != nullptr; curr = curr->get_next())
        if (curr->is_terminator())
            terms++;

    return terms;
}

void BasicBlock::detach() {
    assert(get_parent() && "Basic block has no parent.");

    if (m_Prev)
        m_Prev->set_next(m_Next);
    else {
        // This is the head of the function.
        m_Parent->set_head(m_Next);
    }

    if (m_Next)
        m_Next->set_prev(m_Prev);
    else {
        // This is the tail of the function.
        m_Parent->set_tail(m_Prev);
    }

    delete this;
}

void BasicBlock::print(std::ofstream &OS) const {
    OS << get_name() << ":\n";
    for (Inst *curr = m_Head; curr != nullptr; curr = curr->get_next()) {
        OS << "\t";
        curr->print(OS);
    }
}
