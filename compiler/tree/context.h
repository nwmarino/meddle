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

    TranslationUnit *m_Unit;
    std::unordered_map<String, Type *> m_Types;
    std::vector<FunctionType *> m_FunctionTypes;
    std::vector<TypeResult *> m_Results;

public:
    Context(TranslationUnit *U);
    ~Context() {
        for (auto &[N, T] : m_Types)
            delete T;

        for (auto &FT : m_FunctionTypes)
            delete FT;

        for (auto &R : m_Results)
            delete R;
    }

    Type *getBoolType() const { return m_Types.at("bool"); }
    Type *getVoidType() const { return m_Types.at("void"); }
    Type *getCharType() const { return m_Types.at("char"); }
    Type *getI8Type() const { return m_Types.at("i8"); }
    Type *getI16Type() const { return m_Types.at("i16"); }
    Type *getI32Type() const { return m_Types.at("i32"); }
    Type *getI64Type() const { return m_Types.at("i64"); }
    Type *getU8Type() const { return m_Types.at("u8"); }
    Type *getU16Type() const { return m_Types.at("u16"); }
    Type *getU32Type() const { return m_Types.at("u32"); }
    Type *getU64Type() const { return m_Types.at("u64"); }
    Type *getF32Type() const { return m_Types.at("f32"); }
    Type *getF64Type() const { return m_Types.at("f64"); }
    
    void addType(Type *T);

    void addType(FunctionType *T) { m_FunctionTypes.push_back(T); }

    Type *getType(const String &N) const;

    //ArrayType *getArrayType(Type *E, unsigned long S);

    //PointerType *getPointerType(Type *P);

    Type *produceType(const String &N, const Metadata &M);

    void scrubRefs();
};

} // namespace meddle

#endif // MEDDLE_TREE_CONTEXT_H
