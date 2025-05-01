#include "basicblock.h"
#include "function.h"
#include "inst.h"

#include <map>

using namespace mir;

static std::map<String, unsigned> g_Dict = {};

void mir::clear_bb_dict() { g_Dict.clear(); }

BasicBlock::BasicBlock(String N, Function *P) : Value(N, nullptr), m_Parent(P) {
    if (P)
        P->append(this);

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

static void print_store(std::ostream &OS, StoreInst *I) {
    OS << "str ";
    I->get_value()->print(OS);
    OS << " -> ";
    I->get_dest()->print(OS);
    if (I->get_offset()) {
        OS << " + ";
        I->get_offset()->print(OS);
    }
}

static void print_load(std::ostream &OS, LoadInst *I) {
    OS << "%" << I->get_name() << " := load ";
    I->get_source()->print(OS);
    if (I->get_offset()) {
        OS << " + ";
        I->get_offset()->print(OS);
    }
}

static void print_cpy(std::ostream &OS, CpyInst *I) {
    OS << "cpy i64 " << I->get_size() << ", ";
    I->get_source()->print(OS);
    OS << ", align " << I->get_source_align() << " -> ";
    I->get_dest()->print(OS);
    OS << ", align " << I->get_dest_align();
}

static void print_brif(std::ostream &OS, BrifInst *I) {
    OS << "brif ";
    I->get_cond()->print(OS);
    OS << ", " << I->get_true_dest()->get_name() << ", " 
       << I->get_false_dest()->get_name();
}

static void print_jmp(std::ostream &OS, JMPInst *I) {
    OS << "jmp " << I->get_dest()->get_name();
}

static void print_ret(std::ostream &OS, RetInst *I) {
    OS << "ret";
    if (I->get_value()) {
        OS << " ";
        I->get_value()->print(OS);
    }
}

static void print_unop(std::ostream &OS, UnopInst *I) {
    OS << "%" << I->get_name() << " := ";

    switch (I->get_kind()) {
    case UnopInst::Kind::SExt:
        OS << "sext ";
        break;
    case UnopInst::Kind::ZExt:
        OS << "zext ";
        break;
    case UnopInst::Kind::Trunc:
        OS << "trunc ";
        break;
    case UnopInst::Kind::FExt:
        OS << "fext ";
        break;
    case UnopInst::Kind::FTrunc:
        OS << "ftrunc ";
        break;
    case UnopInst::Kind::SI2FP:
        OS << "si2fp ";
        break;
    case UnopInst::Kind::UI2FP:
        OS << "ui2fp ";
        break;
    case UnopInst::Kind::FP2SI:
        OS << "fp2si ";
        break;
    case UnopInst::Kind::FP2UI:
        OS << "fp2ui ";
        break;
    }

    I->get_value()->print(OS);

    if (I->get_type() != I->get_value()->get_type())
        OS << " -> " << I->get_type()->get_name();
}

void BasicBlock::print(std::ostream &OS) const {
    OS << get_name() << ":\n";
    for (Inst *curr = m_Head; curr != nullptr; curr = curr->get_next()) {
        OS << "    ";
        if (auto *I = dynamic_cast<StoreInst *>(curr))
            print_store(OS, I);
        else if (auto *I = dynamic_cast<LoadInst *>(curr))
            print_load(OS, I);
        else if (auto *I = dynamic_cast<CpyInst *>(curr))
            print_cpy(OS, I);
        else if (auto *I = dynamic_cast<BrifInst *>(curr))
            print_brif(OS, I);
        else if (auto *I = dynamic_cast<JMPInst *>(curr))
            print_jmp(OS, I);
        else if (auto *I = dynamic_cast<RetInst *>(curr))
            print_ret(OS, I);
        else if (auto *I = dynamic_cast<UnopInst *>(curr))
            print_unop(OS, I);

        OS << "\n";
    }
}
