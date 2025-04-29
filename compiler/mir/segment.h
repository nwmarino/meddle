#ifndef SEGMENT_H
#define SEGMENT_H

#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

using String = std::string;

namespace mir {

class Data;
class Function;
class Type;
class StructType;

struct Target final {
    enum class Arch {
        X86_64,
    } Arch;

    enum class OS {
        Linux,
    } OpSys;

    enum class ABI {
        SysV,
    } ABI;

    Target(enum Arch Arch, enum OS OS, enum ABI ABI)
      : Arch(Arch), OpSys(OS), ABI(ABI) {}
};

class Segment final {
    friend class Builder;
    friend class ArrayType;
    friend class FunctionType;
    friend class PointerType;
    friend class StructType;

    Target m_Target;

    std::unordered_map<String, Type *> m_Types = {};
    std::unordered_map<String, Data *> m_Data = {};
    std::unordered_map<String, Function *> m_Functions = {};

public:
    Segment(const Target &T);

    ~Segment();

    void add_data(Data *D);

    Data *get_data(String N) const;

    void remove_data(Data *D);

    void add_function(Function *F);

    Function *get_function(String N) const;

    std::vector<Function *> get_functions() const;

    void remove_function(Function *F);

    void print(std::ofstream &OS) const;
};

} // namespace mir

#endif // SEGMENT_H
