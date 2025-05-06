#ifndef MEDDLE_UNITMAN_H
#define MEDDLE_UNITMAN_H

#include "decl.h"
#include "nameres.h"
#include "sema.h"
#include "unit.h"
#include "../core/logger.h"

#include <algorithm>
#include <filesystem>
#include <system_error>
#include <vector>

using Path = std::filesystem::path;
using namespace std::filesystem;

namespace meddle {

class UnitManager final {
    std::unordered_map<String, TranslationUnit *> m_Units;

    void resolveImports(UseDecl *use, TranslationUnit *parent);

    bool resolveUsesUtil(TranslationUnit *U, std::vector<TranslationUnit *> V, 
                         std::vector<TranslationUnit *> S);

    TranslationUnit *resolveUse(UseDecl *use, const String &importer) const;

    void sanitate();

    void resolveUses();

public:
    UnitManager() = default;

    ~UnitManager() {
        for (auto &U : m_Units)
            delete U.second;
    }

    void addUnit(TranslationUnit *U);

    void addVirtUnit(TranslationUnit *U);

    std::vector<TranslationUnit *> getUnits() const;

    void drive(const Options &opts) ;
};

};

#endif // MEDDLE_UNITMAN_H
