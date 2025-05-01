#include "function.h"
#include "type.h"
#include "value.h"

#include <map>

using namespace mir;

static std::map<String, unsigned> g_Dict = {};

Data::Data(String N, Type *T, Segment *P, Value *V, unsigned A, bool R)
    : Value(N, T), m_Parent(P), m_Value(V), m_Align(A), m_ReadOnly(R) {
    if (m_Parent)
        m_Parent->add_data(this);

    if (g_Dict[N] == 0) {
        m_Name = N;
        g_Dict[N] = 1;
    } else {
        m_Name = N + std::to_string(g_Dict[N]);
        g_Dict[N]++;
    }
}

void Data::print(std::ostream &OS) const {
    OS <<  get_type()->get_name() << " #" << get_name();
}

Slot::Slot(String N, Type *T, Function *P, Type *A, unsigned AL) 
  : Value(N, T), m_Parent(P), m_Alloc(A), m_Align(AL) {
    m_Parent->add_slot(this);
}

void Slot::print(std::ostream &OS) const {
    OS << get_type()->get_name() << " $" << get_name();
}

void ConstantInt::print(std::ostream &OS) const {
    OS << m_Type->get_name() << " " << m_Value;
}

void ConstantFP::print(std::ostream &OS) const {
    OS << m_Type->get_name() << " " << m_Value;
}

void ConstantNil::print(std::ostream &OS) const {
    OS << m_Type->get_name() << " nil";
}

void ConstantString::print(std::ostream &OS) const {
    OS << m_Type->get_name() << " \"";
    for (unsigned i = 0, n = m_Value.size(); i != n; ++i) {
        switch (m_Value[i]) {
        case '\n':
            OS << "\\n";
            break;
        case '\t':
            OS << "\\t";
            break;
        case '\r':
            OS << "\\r";
            break;
        case '\0':
            OS << "\\0";
            break;
        case '\\':
            OS << "\\\\";
            break;
        case '\"':
            OS << "\\\"";
            break;
        default:
            OS << m_Value[i];
            break;
        }
    }
    OS << "\\0\"";
}

void ConstantAggregate::print(std::ostream &OS) const {
    OS << m_Type->get_name() << " [ ";
    for (auto &V : m_Values) {
        V->print(OS);
        OS << ", ";
    }
    OS << " ]";
}
