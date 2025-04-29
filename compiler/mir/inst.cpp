#include "basicblock.h"
#include "inst.h"
#include "type.h"
#include <fstream>

using namespace mir;

Inst::Inst(BasicBlock *P) : Value("", nullptr), m_Parent(P) {
    m_Parent->append(this);
}

Inst::Inst(String N, Type *T, BasicBlock *P) : Value(N, T), m_Parent(P) {
    m_Parent->append(this);
}

void SlotNode::print(std::ofstream &OS) const {
    OS << get_name() << " := slot " << m_Alloc->get_name() << ", " << m_Align 
       << "\n";
}

void StoreInst::print(std::ofstream &OS) const {
    OS << "store ";
    m_Value->print(OS);
    OS << ", ";
    m_Dest->print(OS);
    if (m_Offset) {
        OS << " + ";
        m_Offset->print(OS);
    }
    OS << "\n";
}

void LoadInst::print(std::ofstream &OS) const {
    OS << get_name() << " := load " << m_Type->get_name() << ", ";
    m_Source->print(OS);
    OS << "\n";
}

void RetInst::print(std::ofstream &OS) const {
    OS << "ret";
    if (m_Value) {
        OS << " ";
        m_Value->print(OS);
    }
    OS << "\n";
}
