#ifndef MEDDLE_TREE_CONTEXT_H
#define MEDDLE_TREE_CONTEXT_H

#include "type.h"

#include <unordered_map>
#include <string>

using String = std::string;

namespace meddle {

class TranslationUnit;

class Context final {
    friend class TranslationUnit;
    friend class Type;
    friend class PrimitiveType;
    friend class ArrayType;
    friend class PointerType;
    friend class FunctionType;
    friend class EnumType;
    friend class StructType;
    friend class TemplateStructType;
    friend class DependentTemplateStructType;
    friend class NameResolution;

    TranslationUnit *m_Unit;
    std::unordered_map<String, PrimitiveType *> m_Primitives;
    std::unordered_map<String, ArrayType *> m_Arrays;
    std::unordered_map<String, PointerType *> m_Pointers;
    std::unordered_map<String, EnumType *> m_Enums;
    std::unordered_map<String, StructType *> m_Structs;
    std::unordered_map<String, TemplateStructType *> m_StructSpecs;
    std::unordered_map<String, DependentTemplateStructType *> m_Dependents;
    std::unordered_map<String, DeferredType *> m_Deferred;
    std::unordered_map<String, Type *> m_Externals;
    std::vector<FunctionType *> m_FunctionTypes;

    Type *resolveType(const String &name, const Scope *scope,
                      const Metadata &md, bool specialize);

public:
    Context(TranslationUnit *U = nullptr);

    ~Context();

    Type *getBoolType() const { return m_Primitives.at("bool"); }

    Type *getVoidType() const { return m_Primitives.at("void"); }

    Type *getCharType() const { return m_Primitives.at("char"); }

    Type *getI8Type() const { return m_Primitives.at("i8"); }

    Type *getI16Type() const { return m_Primitives.at("i16"); }

    Type *getI32Type() const { return m_Primitives.at("i32"); }

    Type *getI64Type() const { return m_Primitives.at("i64"); }

    Type *getU8Type() const { return m_Primitives.at("u8"); }

    Type *getU16Type() const { return m_Primitives.at("u16"); }

    Type *getU32Type() const { return m_Primitives.at("u32"); }

    Type *getU64Type() const { return m_Primitives.at("u64"); }

    Type *getF32Type() const { return m_Primitives.at("f32"); }

    Type *getF64Type() const { return m_Primitives.at("f64"); }

    Type *getType(const String &N, const Scope *S);

    void importType(Type *T, const String &N = "");

    void sanitate();
};

} // namespace meddle

#endif // MEDDLE_TREE_CONTEXT_H
