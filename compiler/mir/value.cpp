#include "type.h"
#include "value.h"

#include <fstream>

using namespace mir;

void Data::print(std::ofstream &OS) const {
    OS << get_name() << " := data ";
    m_Value->print(OS);
    OS << ", align " << m_Align;
    if (m_ReadOnly)
        OS << ", readonly";
    OS << "\n";
}

void ConstantInt::print(std::ofstream &OS) const {
    OS << m_Type->get_name() << " " << m_Value;
}

void ConstantFP::print(std::ofstream &OS) const {
    OS << m_Type->get_name() << " " << m_Value;
}

void ConstantNil::print(std::ofstream &OS) const {
    OS << m_Type->get_name() << " nil";
}

void ConstantString::print(std::ofstream &OS) const {
    OS << m_Type->get_name() << " \"";
    for (auto &c : m_Value) {
        if (c == '\\')
            OS << "\\\\";
        else if (c == '"')
            OS << "\\\"";
        else
            OS << c;
    }

    OS << "\"";
}

void ConstantAggregate::print(std::ofstream &OS) const {
    OS << m_Type->get_name() << " [ ";
    for (auto &V : m_Values) {
        V->print(OS);
        OS << ", ";
    }
    OS << " ]";
}
