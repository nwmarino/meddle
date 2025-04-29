#include "function.h"
#include "segment.h"
#include "type.h"
#include <fstream>

using namespace mir;

Segment::Segment(const Target &T) : m_Target(T) {
    m_Types["i1"] = new IntegerType(IntegerType::Kind::Int1);
    m_Types["i8"] = new IntegerType(IntegerType::Kind::Int8);
    m_Types["i16"] = new IntegerType(IntegerType::Kind::Int16);
    m_Types["i32"] = new IntegerType(IntegerType::Kind::Int32);
    m_Types["i64"] = new IntegerType(IntegerType::Kind::Int64);
    m_Types["f32"] = new FloatType(FloatType::Kind::Float32);
    m_Types["f64"] = new FloatType(FloatType::Kind::Float64);
}

Segment::~Segment() {
    for (auto &[ String, ty ] : m_Types)
        delete ty;

    for (auto &[ String, data ] : m_Data)
        delete data;

    for (auto &[ String, fn ] : m_Functions)
        delete fn;
}

void Segment::add_data(Data *D) {
    assert(!get_data(D->get_name()) && "Data with name already exists.");
    m_Data[D->get_name()] = D;
}

Data *Segment::get_data(String N) const {
    auto it = m_Data.find(N);
    if (it != m_Data.end())
        return it->second;

    return nullptr;
}

void Segment::remove_data(Data *D) {
    auto it = m_Data.find(D->get_name());
    if (it != m_Data.end()) {
        m_Data.erase(it);
        delete D;
    } else
        assert(false && "Data with name does not exist in this segment.");
}

void Segment::add_function(Function *F) {
    assert(!get_function(F->get_name()) && "Function with name already exists.");
    m_Functions[F->get_name()] = F;
}

Function *Segment::get_function(String N) const {
    auto it = m_Functions.find(N);
    if (it != m_Functions.end())
        return it->second;

    return nullptr;
}

std::vector<Function *> Segment::get_functions() const {
    std::vector<Function *> F;
    for (auto &[_, F_] : m_Functions)
        F.push_back(F_);
    return F;
}

void Segment::remove_function(Function *F) {
    auto it = m_Functions.find(F->get_name());
    if (it != m_Functions.end()) {
        m_Functions.erase(it);
        delete F;
    } else
        assert(false && "Function with name does not exist in this segment.");
}

void Segment::print(std::ofstream &OS) const {
    OS << "target :: ";

    switch (m_Target.Arch) {
        case Target::Arch::X86_64: OS << "x86_64 ";
    }

    switch (m_Target.OpSys) {
        case Target::OS::Linux: OS << "linux ";
    }

    switch (m_Target.ABI) {
        case Target::ABI::SysV: OS << "system_v ";
    }

    OS << "\n\n";

    for (auto &[ String, Type ] : m_Types) {
        if (!Type->is_struct_ty())
            continue;

        StructType *ST = static_cast<StructType *>(Type);
        OS << "struct " << ST->get_name() << " { ";
        for (auto &M: ST->get_members())
            OS << M->get_name() << (M != ST->get_members().back() ? ",\n" : "\n");

        OS << " }\n";
    }

    for (auto &[ String, Data ] : m_Data)
        Data->print(OS);

    if (!m_Data.empty())
        OS << "\n";

    for (auto &[ String, Function ] : m_Functions)
        Function->print(OS);
}
