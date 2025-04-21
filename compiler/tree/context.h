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
    Context(TranslationUnit *U) : m_Unit(U) {}
    ~Context() {
        for (auto &[N, T] : m_Types)
            delete T;

        for (auto &FT : m_FunctionTypes)
            delete FT;

        for (auto &R : m_Results)
            delete R;
    }

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
