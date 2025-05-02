#include "basicblock.h"
#include "inst.h"
#include "type.h"

#include <cassert>
#include <map>

using namespace mir;

static std::map<String, unsigned> g_Dict = {};

void mir::clear_inst_dict() { g_Dict.clear(); }

Inst::Inst(BasicBlock *P) : Value("", nullptr), m_Parent(P) {
    m_Parent->append(this);
}

Inst::Inst(String N, Type *T, BasicBlock *P) : Value(N, T), m_Parent(P) {
    m_Parent->append(this);

    if (N.empty())
        return;

    if (g_Dict[N] == 0) {
        m_Name = N;
        g_Dict[N] = 1;
    } else {
        m_Name = N + std::to_string(g_Dict[N]);
        g_Dict[N]++;
    }
}
