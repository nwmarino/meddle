#include "basicblock.h"
#include "inst.h"
#include "type.h"

#include <cassert>
#include <map>
#include <ostream>

using namespace mir;

static std::map<String, unsigned> g_Dict = {};

void mir::clear_inst_dict() { g_Dict.clear(); }

Inst::Inst(BasicBlock *P) : Value("", nullptr), m_Parent(P) {
    m_Parent->append(this);
}

Inst::Inst(String N, Type *T, BasicBlock *P) : Value(N, T), m_Parent(P) {
    m_Parent->append(this);

    if (g_Dict[N] == 0) {
        m_Name = N;
        g_Dict[N] = 1;
    } else {
        m_Name = N + std::to_string(g_Dict[N]);
        g_Dict[N]++;
    }
}

void StoreInst::print(std::ostream &OS) const { 
    assert(false && "'store' does not produce a value."); 
}

void LoadInst::print(std::ostream &OS) const {
    OS << get_type()->get_name() << " %" << get_name();
}

void CpyInst::print(std::ostream &OS) const {
    assert(false && "'cpy' does not produce a value.");
}

void BrifInst::print(std::ostream &OS) const {
    assert(false && "'brif' does not produce a value.");
}

void JMPInst::print(std::ostream &OS) const {
    assert(false && "'jmp' does not produce a value.");
}

void RetInst::print(std::ostream &OS) const {
    assert(false && "'ret' does not produce a value.");
}

void BinopInst::print(std::ostream &OS) const {
    OS << get_type()->get_name() << " %" << get_name();
}

void UnopInst::print(std::ostream &OS) const {
    OS << get_type()->get_name() << " %" << get_name();
}

void CMPInst::print(std::ostream &OS) const {
    OS << get_type()->get_name() << " %" << get_name();
}
