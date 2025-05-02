#include "basicblock.h"
#include "function.h"
#include "inst.h"
#include "segment.h"

#include <map>
#include <ostream>

using namespace mir;

static std::map<String, unsigned> g_Dict = {};

void mir::clear_bb_dict() { g_Dict.clear(); }

BasicBlock::BasicBlock(String N, Function *P) : Value(N, nullptr), m_Parent(P) {
    if (P)
        P->append(this);

    if (N.empty())
        return;

    if (g_Dict[N] == 0) {
        m_Name = N;
        g_Dict[N] = 1;
    } else {
        m_Name = N + std::to_string(g_Dict[N]);
        g_Dict[N]++;
    }
}

BasicBlock::~BasicBlock() {
    for (Inst *curr = m_Tail; curr != nullptr; ) {
        Inst *prev = curr->get_prev();
        delete curr;
        curr = prev;
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
