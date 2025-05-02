#ifndef SEGMENT_H
#define SEGMENT_H

#include "value.h"

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

using String = std::string;

namespace mir {

class ConstantInt;
class ConstantFP;
class ConstantNil;
class Data;
class DataLayout;
class Function;
class Type;
class StructType;
enum class TypeKind;

enum class Arch {
    X86_64,
};

enum class OS {
    Linux,
};

enum class ABI {
    SystemV,
};

struct Target final {
    Arch Arch;
    OS OpSys;
    ABI ABI;

    Target(enum Arch Arch, enum OS OpSys, enum ABI ABI)
      : Arch(Arch), OpSys(OpSys), ABI(ABI) {}

    DataLayout get_data_layout() const;
};

class DataLayout final {
    Arch m_Arch;
    bool m_LittleEndian;
    unsigned m_PointerSize;
    unsigned m_PointerAlign;

    struct LayoutRule final {
        unsigned size;
        unsigned abiAlign;
    };

    std::unordered_map<TypeKind, LayoutRule> m_Rules = {};

    static unsigned align_to(unsigned offset, unsigned align);

public:
    DataLayout(enum Arch arch, bool littleEndian, unsigned pointerSZ, 
               unsigned pointerAL);

    unsigned get_type_size(Type *T) const;
    unsigned get_type_align(Type *T) const;

    unsigned get_pointer_size(unsigned addrSpace = 0) const;
    unsigned get_pointer_align(unsigned addrSpace = 0) const;
    
    bool is_little_endian() const;
    bool is_big_endian() const;

    bool is_scalar_ty(Type *T) const;
};

class Segment final {
    friend class Builder;
    friend class ArrayType;
    friend class FunctionType;
    friend class PointerType;
    friend class StructType;
    friend class ConstantInt;
    friend class ConstantFP;
    friend class ConstantNil;

    Target m_Target;
    DataLayout m_Layout;

    std::unordered_map<String, Type *> m_Types = {};
    std::unordered_map<String, Data *> m_Data = {};
    std::unordered_map<String, Function *> m_Functions = {};

    ConstantInt *m_I1Zero = nullptr;
    ConstantInt *m_I1One = nullptr;
    std::unordered_map<Type *, ConstantNil *> m_NilPool = {};
    std::unordered_map<int8_t, ConstantInt *> m_I8Pool = {};
    std::unordered_map<int16_t, ConstantInt *> m_I16Pool = {};
    std::unordered_map<int32_t, ConstantInt *> m_I32Pool = {};
    std::unordered_map<int64_t, ConstantInt *> m_I64Pool = {};
    std::unordered_map<float, ConstantFP *> m_F32Pool = {};
    std::unordered_map<double, ConstantFP *> m_F64Pool = {};

public:
    Segment(const Target &T);

    ~Segment();

    const DataLayout &get_data_layout() const { return m_Layout; }

    void add_data(Data *D);

    Data *get_data(String N) const;

    void remove_data(Data *D);

    void add_function(Function *F);

    Function *get_function(String N) const;

    std::vector<Function *> get_functions() const;

    void remove_function(Function *F);

    void print(std::ostream &OS) const;
};

} // namespace mir

#endif // SEGMENT_H
