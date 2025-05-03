#include "basicblock.h"
#include "function.h"
#include "segment.h"
#include "type.h"

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

bool DataLayout::is_scalar_ty(Type *T) const {
    switch (T->get_ty_kind()) {
    case TypeKind::I1:
    case TypeKind::I8:
    case TypeKind::I16:
    case TypeKind::I32:
    case TypeKind::I64:
    case TypeKind::F32:
    case TypeKind::F64:
    case TypeKind::Pointer:
        return true;
    default:
        return false;
    }
}

unsigned DataLayout::get_struct_member_offset(StructType *T, unsigned Idx) {
    unsigned offset = 0;
    for (unsigned i = 0; i < Idx; ++i) {
        Type *M = T->get_member(i);
        unsigned align = get_type_align(M);
        offset = align_to(offset, align) + get_type_size(M);
    }

    return align_to(offset, get_type_align(T->get_member(Idx)));
}

Segment::Segment(const Target &T) : m_Target(T), m_Layout(T.get_data_layout()) {
    m_Types["i1"] = new IntegerType(IntegerType::Kind::Int1);
    m_Types["i8"] = new IntegerType(IntegerType::Kind::Int8);
    m_Types["i16"] = new IntegerType(IntegerType::Kind::Int16);
    m_Types["i32"] = new IntegerType(IntegerType::Kind::Int32);
    m_Types["i64"] = new IntegerType(IntegerType::Kind::Int64);
    m_Types["f32"] = new FloatType(FloatType::Kind::Float32);
    m_Types["f64"] = new FloatType(FloatType::Kind::Float64);
    m_Types["void"] = new VoidType();
    m_I1Zero = new ConstantInt(m_Types.at("i1"), 0);
    m_I1One = new ConstantInt(m_Types.at("i1"), 1);
}

Segment::~Segment() {
    for (auto &T : m_Types)
        delete T.second;
    m_Types.clear();

    for (auto &FN : m_Functions)
        delete FN.second;
    m_Functions.clear();

    for (auto &D : m_Data)
        delete D.second;
    m_Data.clear();

    for (auto &[ Type, Nil ] : m_NilPool)
        delete Nil;
    m_NilPool.clear();

    for (auto &[ Value, Int ] : m_I8Pool)
        delete Int;
    m_I8Pool.clear();

    for (auto &[ Value, Int ] : m_I16Pool)
        delete Int;
    m_I16Pool.clear();

    for (auto &[ Value, Int ] : m_I32Pool)
        delete Int;
    m_I32Pool.clear();

    for (auto &[ Value, Int ] : m_I64Pool)
        delete Int;
    m_I64Pool.clear();

    for (auto &[ Value, FP ] : m_F32Pool)
        delete FP;
    m_F32Pool.clear();

    for (auto &[ Value, FP ] : m_F64Pool)
        delete FP;
    m_F64Pool.clear();

    delete m_I1Zero;
    delete m_I1One;
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
