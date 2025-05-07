#include "codegen.h"
#include "../tree/expr.h"
#include "../mir/builder.h"

#include <cassert>

using namespace meddle;

void CGN::visit(RuneSyscallExpr *expr) {
    mir::Type *ty = cgn_type(expr->getType());
    mir::Value *num = mir::ConstantInt::get(m_Segment, m_Builder.get_i64_ty(), 
        expr->getSyscallNum());
    std::vector<mir::Value *> args;
    args.reserve(expr->getArgs().size());

    for (auto &A : expr->getArgs()) {
        m_VC = ValueContext::RValue;
        A->accept(this);
        assert(m_Value && "Syscall argument does not produce a value.");
        args.push_back(m_Value);
    }

    m_Value = m_Builder.build_syscall(num, args, 
        m_Opts.NamedMIR ? "syscall" : "");
}
