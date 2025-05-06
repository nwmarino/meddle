#ifndef MEDDLE_UNITMAN_H
#define MEDDLE_UNITMAN_H

#include "decl.h"
#include "nameres.h"
#include "sema.h"
#include "unit.h"
#include "../core/logger.h"

#include <algorithm>
#include <filesystem>
#include <vector>

using Path = std::filesystem::path;
using namespace std::filesystem;

namespace meddle {

class UnitManager final {
    std::unordered_map<String, TranslationUnit *> m_Units;

    void resolveImports(UseDecl *use, TranslationUnit *parent) {
        assert(use && "Use cannot be null.");
        assert(use->getUnit() && "Use must have its target resolved.");

        // If the use is named, i.e. `use Name = "..."`, then importing happens
        // by way of the `::` operator.
        if (use->isNamed())
            return;

        std::vector<NamedDecl *> Imports;
        if (use->getSymbols().empty()) {
            // If the use is not named, i.e. `use "..."`, then all named symbols
            // exported by the unit are to be added.
            Imports = use->getUnit()->getExports();
        } else {
            // If the use specifies only certain symbols, i.e.
            // `use { Foo, Bar } = "..."`, then only those symbols are to be 
            // added. They must be exported by the unit and actually exist.
            std::vector<NamedDecl *> exports = use->getUnit()->getExports();
            Imports.reserve(use->getSymbols().size());
            for (auto &name : use->getSymbols()) {
                NamedDecl *target = nullptr;

                for (auto &exp : exports) {
                    if (exp->getName() == name) {
                        target = exp;
                        break;
                    }
                }

                if (!target) {
                    fatal(
                        "name '" + name + "' does not exist in: '" + 
                        use->getUnit()->getFile().path + "'", &use->getMetadata()
                    );
                }

                if (!target->hasPublicRune()) {
                    fatal("name '" + name + "' exists, but is not marked public", 
                        &use->getMetadata());
                }

                Imports.push_back(target);
            }
        }

        Scope *scope = parent->getScope();
        Context *ctx = parent->getContext();
        for (auto &Import : Imports) {
            scope->addDecl(Import);

            if (TypeDecl *TD = dynamic_cast<TypeDecl *>(Import))
                ctx->addType(TD->getDefinedType());
        }
    }

    bool resolveUsesUtil(TranslationUnit *U, std::vector<TranslationUnit *> V, 
                         std::vector<TranslationUnit *> S) {
        if (std::find(V.begin(), V.end(), U) != V.end()) {
            // If this package has been visited before: no cycle.
            if (!S.empty())
                S.pop_back();
    
            return false;
        }
    
        // Mark the current package as having been visited.
        V.push_back(U);
        S.push_back(U);
    
        for (auto &Use : U->getUses()) {
            // Resolve the use dependency.
            TranslationUnit *dep = resolveUse(Use, U->getFile().path);
            if (!dep) {
                fatal(
                    "unresolved unit: " + Use->getPath(), 
                    &Use->getMetadata()
                );
            }

            Use->setUnit(dep);
    
            if (std::find(V.begin(), V.end(), dep) != V.end()
            && resolveUsesUtil(dep, V, S)) {
                fatal(
                    "cyclical use in file '" + U->getFile().path
                    + "', using: '" + dep->getFile().path + "'", 
                    &Use->getMetadata()
                );
            } else if (std::find(S.begin(), S.end(), dep) != S.end()) {
                return true;
            }

            resolveImports(Use, U);
        }
    
        return false;
    }

    TranslationUnit *resolveUse(UseDecl *use, const Path &importer) const {
        auto absPath = canonical(importer.parent_path() / use->getPath());
        auto key = absPath.string();

        if (m_Units.count(key))
            return m_Units.at(key);

        return nullptr;
    }

    void sanitate() {
        for (auto &[ Path, Unit ] : m_Units)
            Unit->getContext()->sanitate();
    }

    void resolveUses() {
        std::vector<TranslationUnit *> Visited;
        std::vector<TranslationUnit *> Stack;

        for (auto &[ Path, Unit ] : m_Units)
            if (std::find(Visited.begin(), Visited.end(), Unit) == Visited.end())
                resolveUsesUtil(Unit, Visited, Stack);
    }

public:
    UnitManager() = default;

    ~UnitManager() {
        for (auto &U : m_Units)
            delete U.second;
    }

    void addUnit(TranslationUnit *U) {
        assert(U && "Unit cannot be null.");
        assert(!U->getFile().path.empty() && "Unit must have a path.");

        auto absPath = canonical(U->getFile().path);
        auto key = absPath.string();

        if (m_Units.count(key))
            fatal("multiple files with same path: " + key, nullptr);

        m_Units[key] = U;
    }

    void addVirtUnit(TranslationUnit *U) {
        assert(U && "Unit cannot be null.");
        assert(!U->getFile().path.empty() && "Unit must have a path.");

        if (m_Units.count(U->getFile().path))
            fatal("multiple files with same path: " + U->getFile().path, nullptr);

        m_Units[U->getFile().path] = U;
    }

    std::vector<TranslationUnit *> getUnits() const {
        std::vector<TranslationUnit *> units;
        for (auto &[ Path, Unit ] : m_Units)
            units.push_back(Unit);

        return units;
    }

    void drive(const Options &opts) {
        resolveUses();
        
        for (auto &[ Path, Unit ] : m_Units)
            Unit->getContext()->sanitate();

        for (auto &[ Path, Unit ] : m_Units)
            NameResolution NR = NameResolution(opts, Unit, NameResolution::Phase::Shallow);

        for (auto &[ Path, Unit ] : m_Units)
            NameResolution NR = NameResolution(opts, Unit, NameResolution::Phase::Recurse);

        for (auto &[ Path, Unit ] : m_Units)
            Sema sema = Sema(opts, Unit);
    }
};

};

#endif // MEDDLE_UNITMAN_H
