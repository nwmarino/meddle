#include "basicblock.h"
#include "function.h"
#include "segment.h"
#include "type.h"

#include <fstream>

using namespace mir;

DataLayout Target::get_data_layout() const {
    switch (Arch) {
        case Arch::X86_64:
            return DataLayout(Arch, true, 8, 8);
    }

    assert(false && "Unsupported architecture.");
}

DataLayout::DataLayout(enum Arch arch, bool littleEndian, unsigned pointerSZ, 
                       unsigned pointerAL)
    : m_Arch(arch), m_LittleEndian(littleEndian), m_PointerSize(pointerSZ),
      m_PointerAlign(pointerAL) 
{
    m_Rules[TypeKind::I1] = { 1, 1 };
    m_Rules[TypeKind::I8] = { 1, 1 };
    m_Rules[TypeKind::I16] = { 2, 2 };
    m_Rules[TypeKind::I32] = { 4, 4 };
    m_Rules[TypeKind::I64] = { 8, 8 };
    m_Rules[TypeKind::F32] = { 4, 4 };
    m_Rules[TypeKind::F64] = { 8, 8 };
}

unsigned DataLayout::get_type_size(Type *T) const {
    switch (T->get_ty_kind()) {
        case TypeKind::Pointer:
            return get_pointer_size();
        case TypeKind::Array:
        {
            ArrayType *AT = static_cast<ArrayType *>(T);
            return get_type_size(AT->get_element()) * AT->get_size();
        }
        case TypeKind::Struct: 
        {
            StructType *ST = static_cast<StructType *>(T);
            unsigned offset = 0;
            for (const auto& M : ST->get_members()) {
                unsigned align = get_type_align(M);
                offset = align_to(offset, align);
                offset += get_type_size(M);
            }
            return align_to(offset, get_type_align(T));
        }
        default:
            return m_Rules.at(T->get_ty_kind()).size;
    }
}

unsigned DataLayout::get_type_align(Type *T) const {
    switch (T->get_ty_kind()) {
        case TypeKind::Pointer:
            return get_pointer_align();
        case TypeKind::Array:
        {
            ArrayType *AT = static_cast<ArrayType *>(T);
            return get_type_align(AT->get_element());
        }
        case TypeKind::Struct: 
        {
            StructType *ST = static_cast<StructType *>(T);
            unsigned maxAlign = 1;
            for (const auto& M : ST->get_members())
                maxAlign = std::max(maxAlign, get_type_align(M));
            return maxAlign;
        }
        default:
            return m_Rules.at(T->get_ty_kind()).abiAlign;
    }
}

unsigned DataLayout::align_to(unsigned offset, unsigned align) {
    return (offset + align - 1) & ~(align - 1);
}

unsigned DataLayout::get_pointer_size(unsigned addrSpace) const {
    return m_PointerSize;
}

unsigned DataLayout::get_pointer_align(unsigned addrSpace) const {
    return m_PointerAlign;
}
    
bool DataLayout::is_little_endian() const {
    return m_LittleEndian;
}

bool DataLayout::is_big_endian() const {
    return !m_LittleEndian;
}

Segment::Segment(const Target &T) : m_Target(T), m_Layout(T.get_data_layout()) {
    m_Types["i1"] = new IntegerType(IntegerType::Kind::Int1);
    m_Types["i8"] = new IntegerType(IntegerType::Kind::Int8);
    m_Types["i16"] = new IntegerType(IntegerType::Kind::Int16);
    m_Types["i32"] = new IntegerType(IntegerType::Kind::Int32);
    m_Types["i64"] = new IntegerType(IntegerType::Kind::Int64);
    m_Types["f32"] = new FloatType(FloatType::Kind::Float32);
    m_Types["f64"] = new FloatType(FloatType::Kind::Float64);
}

Segment::~Segment() {
    for (auto &T : m_Types)
        delete T.second;

    for (auto &FN : m_Functions)
        delete FN.second;

    for (auto &D : m_Data)
        delete D.second;
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

static void print_data(std::ostream &OS, Data *D) {
    OS << "#" << D->get_name() << " := " << (D->is_read_only() ? "readonly " : " ");
    D->get_value()->print(OS);
    OS << ", align " << D->get_align() << "\n";
}

static void print_slot(std::ostream &OS, Slot *S) {
    OS << "$" << S->get_name() << " := slot " << S->get_alloc_type()->get_name() 
       << ", align " << S->get_align();
}

static void print_function(std::ostream &OS, Function *F) {
    OS << F->get_name() << " :: (";
    for (auto &A : F->get_args()) {

    }

    OS << ")";
    FunctionType *FT = static_cast<FunctionType *>(F->get_type());
    if (FT->get_return_type())
        OS << " -> " << FT->get_return_type()->get_name();

    if (!F->head()) {
        OS << ";\n";
        return;
    }

    OS << " {\n";

    for (auto &S : F->get_slots()) {
        OS << "    ";
        print_slot(OS, S);
        OS << "\n";
    }

    if (!F->get_slots().empty())
        OS << "\n";

    for (BasicBlock *curr = F->head(); curr != nullptr; curr = curr->get_next()) {
        curr->print(OS);
        if (curr->get_next())
            OS << "\n";
    }

    OS << "}\n\n";
}

void Segment::print(std::ostream &OS) const {
    OS << "target :: ";

    switch (m_Target.Arch) {
        case Arch::X86_64: OS << "x86_64 ";
    }

    switch (m_Target.OpSys) {
        case OS::Linux: OS << "linux ";
    }

    switch (m_Target.ABI) {
        case ABI::SystemV: OS << "system_v";
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
        print_data(OS, Data);

    if (!m_Data.empty())
        OS << "\n";

    for (auto &[ String, Function ] : m_Functions)
        print_function(OS, Function);
}
