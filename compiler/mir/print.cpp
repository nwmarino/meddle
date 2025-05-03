#include "basicblock.h"
#include "function.h"
#include "inst.h"
#include "segment.h"
#include "value.h"

#include <iomanip>
#include <iostream>
#include <ostream>
#include <unordered_map>

using namespace mir;

static String get_printed_name(Value const *V) {
    return V->get_name();
}

static void print_phi(std::ostream &OS, PHINode *I) {
    OS << "$" << get_printed_name(I) << " := phi " << I->get_type()->get_name();
    if (I->get_incoming().empty())
        return;
    
    OS << " ";

    for (auto &[ Value, Block ] : I->get_incoming()) {
        OS << "[ ";
        Block->print(OS);
        OS << ", ";
        Value->print(OS);
        
        if (Value != I->get_incoming().back().first)
            OS << " ], ";
        else
            OS << " ]";
    }
}

static void print_ap(std::ostream &OS, APInst *I) {
    OS << "$" << get_printed_name(I) << " := ap " << I->get_type()->get_name() << ", ";
    I->get_source()->print(OS);
    OS << ", ";
    I->get_index()->print(OS);
}

static void print_store(std::ostream &OS, StoreInst *I) {
    OS << "str ";
    I->get_value()->print(OS);
    OS << " -> ";
    I->get_dest()->print(OS);
    if (I->get_offset()) {
        OS << " + ";
        I->get_offset()->print(OS);
    }
}

static void print_load(std::ostream &OS, LoadInst *I) {
    OS << "$" << get_printed_name(I) << " := load ";
    I->get_source()->print(OS);
    if (I->get_offset()) {
        OS << " + ";
        I->get_offset()->print(OS);
    }
}

static void print_cpy(std::ostream &OS, CpyInst *I) {
    OS << "cpy i64 " << I->get_size() << ", ";
    I->get_source()->print(OS);
    OS << ", align " << I->get_source_align() << " -> ";
    I->get_dest()->print(OS);
    OS << ", align " << I->get_dest_align();
}

static void print_brif(std::ostream &OS, BrifInst *I) {
    OS << "brif ";
    I->get_cond()->print(OS);
    OS << ", ";
    I->get_true_dest()->print(OS);
    OS << ", ";
    I->get_false_dest()->print(OS);
}

static void print_jmp(std::ostream &OS, JMPInst *I) {
    OS << "jmp ";
    I->get_dest()->print(OS);
}

static void print_ret(std::ostream &OS, RetInst *I) {
    OS << "ret";
    if (I->get_value()) {
        OS << " ";
        I->get_value()->print(OS);
    }
}

static void print_binop(std::ostream &OS, BinopInst *I) {
    OS << "$" << get_printed_name(I) << " := ";
    switch (I->get_kind()) {
    case BinopInst::Kind::Add:
        OS << "add ";
        break;
    case BinopInst::Kind::Sub:
        OS << "sub ";
        break;
    case BinopInst::Kind::SMul:
        OS << "smul ";
        break;
    case BinopInst::Kind::UMul:
        OS << "umul ";
        break;
    case BinopInst::Kind::SDiv:
        OS << "sdiv ";
        break;
    case BinopInst::Kind::UDiv:
        OS << "udiv ";
        break;
    case BinopInst::Kind::SRem:
        OS << "srem ";
        break;
    case BinopInst::Kind::URem:
        OS << "urem ";
        break;
    case BinopInst::Kind::FAdd:
        OS << "fadd ";
        break;
    case BinopInst::Kind::FSub:
        OS << "fsub ";
        break;
    case BinopInst::Kind::FMul:
        OS << "fmul ";
        break;
    case BinopInst::Kind::FDiv:
        OS << "fdiv ";
        break;
    case BinopInst::Kind::And:
        OS << "and ";
        break;
    case BinopInst::Kind::Or:
        OS << "or ";
        break;
    case BinopInst::Kind::Xor:
        OS << "xor ";
        break;
    case BinopInst::Kind::Shl:
        OS << "shl ";
        break;
    case BinopInst::Kind::AShr:
        OS << "ashr ";
        break;
    case BinopInst::Kind::LShr:
        OS << "lshr ";
        break;
    }

    I->get_lval()->print(OS);
    OS << ", ";
    I->get_rval()->print(OS);
}

static void print_unop(std::ostream &OS, UnopInst *I) {
    OS << "$" << get_printed_name(I) << " := ";
    switch (I->get_kind()) {
    case UnopInst::Kind::Not:
        OS << "not ";
        break;
    case UnopInst::Kind::Neg:
        OS << "neg ";
        break;
    case UnopInst::Kind::FNeg:
        OS << "fneg ";
        break;
    case UnopInst::Kind::SExt:
        OS << "sext ";
        break;
    case UnopInst::Kind::ZExt:
        OS << "zext ";
        break;
    case UnopInst::Kind::Trunc:
        OS << "trunc ";
        break;
    case UnopInst::Kind::FExt:
        OS << "fext ";
        break;
    case UnopInst::Kind::FTrunc:
        OS << "ftrunc ";
        break;
    case UnopInst::Kind::SI2FP:
        OS << "si2fp ";
        break;
    case UnopInst::Kind::UI2FP:
        OS << "ui2fp ";
        break;
    case UnopInst::Kind::FP2SI:
        OS << "fp2si ";
        break;
    case UnopInst::Kind::FP2UI:
        OS << "fp2ui ";
        break;
    case UnopInst::Kind::Reint:
        OS << "reint ";
        break;
    case UnopInst::Kind::Ptr2Int:
        OS << "ptr2int ";
        break;
    case UnopInst::Kind::Int2Ptr:
        OS << "int2ptr ";
        break;
    }

    I->get_value()->print(OS);
    if (I->get_type() != I->get_value()->get_type())
        OS << " -> " << I->get_type()->get_name();
}

static void print_cmp(std::ostream &OS, CMPInst *I) {
    OS << "$" << get_printed_name(I) << " := ";
    switch (I->get_kind()) {
    case CMPInst::Kind::ICMP_EQ:
        OS << "icmp_eq ";
        break;
    case CMPInst::Kind::ICMP_NE:
        OS << "icmp_ne ";
        break;
    case CMPInst::Kind::ICMP_SLT:
        OS << "icmp_slt ";
        break;
    case CMPInst::Kind::ICMP_ULT:
        OS << "icmp_ult ";
        break;
    case CMPInst::Kind::ICMP_SLE:
        OS << "icmp_sle ";
        break;
    case CMPInst::Kind::ICMP_ULE:
        OS << "icmp_ule ";
        break;
    case CMPInst::Kind::ICMP_SGT:
        OS << "icmp_sgt ";
        break;
    case CMPInst::Kind::ICMP_UGT:
        OS << "icmp_ugt ";
        break;
    case CMPInst::Kind::ICMP_SGE:
        OS << "icmp_sge ";
        break;
    case CMPInst::Kind::ICMP_UGE:
        OS << "icmp_uge ";
        break;
    case CMPInst::Kind::FCMP_OEQ:
        OS << "fcmp_oeq ";
        break;
    case CMPInst::Kind::FCMP_ONE:
        OS << "fcmp_one ";
        break;
    case CMPInst::Kind::FCMP_OLT:
        OS << "fcmp_olt ";
        break;
    case CMPInst::Kind::FCMP_OLE:
        OS << "fcmp_ole ";
        break;
    case CMPInst::Kind::FCMP_OGT:
        OS << "fcmp_ogt ";
        break;
    case CMPInst::Kind::FCMP_OGE:
        OS << "fcmp_oge ";
        break;
    case CMPInst::Kind::PCMP_EQ:
        OS << "pcmp_eq ";
        break;
    case CMPInst::Kind::PCMP_NE:
        OS << "pcmp_ne ";
        break;
    case CMPInst::Kind::PCMP_LT:
        OS << "pcmp_lt ";
        break;
    case CMPInst::Kind::PCMP_LE:
        OS << "pcmp_le ";
        break;
    case CMPInst::Kind::PCMP_GT:
        OS << "pcmp_gt ";
        break;
    case CMPInst::Kind::PCMP_GE:
        OS << "pcmp_ge ";
        break;
    }

    I->get_lval()->print(OS);
    OS << ", ";
    I->get_rval()->print(OS);
}

static void print_block(std::ostream &OS, BasicBlock *BB) {
    OS << get_printed_name(BB);

    if (BB->has_preds()) {
        OS << " (";
        for (auto &P : BB->get_preds()) {
            OS << get_printed_name(P)
               << (P != BB->get_preds().back() ? ", " : "");
        }

        OS << ")";
    }

    OS << ":\n";
    
    for (Inst *curr = BB->head(); curr != nullptr; curr = curr->get_next()) {
        OS << "    ";
        if (auto *I = dynamic_cast<PHINode *>(curr))
            print_phi(OS, I);
        else if (auto *I = dynamic_cast<APInst *>(curr))
            print_ap(OS, I);
        else if (auto *I = dynamic_cast<StoreInst *>(curr))
            print_store(OS, I);
        else if (auto *I = dynamic_cast<LoadInst *>(curr))
            print_load(OS, I);
        else if (auto *I = dynamic_cast<CpyInst *>(curr))
            print_cpy(OS, I);
        else if (auto *I = dynamic_cast<BrifInst *>(curr))
            print_brif(OS, I);
        else if (auto *I = dynamic_cast<JMPInst *>(curr))
            print_jmp(OS, I);
        else if (auto *I = dynamic_cast<RetInst *>(curr))
            print_ret(OS, I);
        else if (auto *I = dynamic_cast<BinopInst *>(curr))
            print_binop(OS, I);
        else if (auto *I = dynamic_cast<UnopInst *>(curr))
            print_unop(OS, I);
        else if (auto *I = dynamic_cast<CMPInst *>(curr))
            print_cmp(OS, I);

        OS << "\n";
    }
}

static void print_data(std::ostream &OS, Data *D) {
    OS << get_printed_name(D) << " :: " 
       << (D->is_read_only() ? "readonly " : " ");
    D->get_value()->print(OS);
    OS << ", align " << D->get_align() << "\n";
}

static void print_arg(std::ostream &OS, Argument *A) {
    if (A->hasAArgAttribute())
        OS << "aarg ";
    if (A->hasARetAttribute())
        OS << "aret ";

    OS << A->get_type()->get_name() << " %" << A->get_name();
}

static void print_slot(std::ostream &OS, Slot *S) {
    OS << "_" << get_printed_name(S) << " := slot " 
       << S->get_alloc_type()->get_name() << ", align " << S->get_align();
}

static void print_function(std::ostream &OS, Function *FN) {
    OS << get_printed_name(FN) << " :: (";
    for (auto &A : FN->get_args()) {
        print_arg(OS, A);
        if (A != FN->get_args().back())
            OS << ", ";
    }

    OS << ") -> " << FN->get_return_ty()->get_name();

    if (!FN->head()) {
        OS << ";\n";
        return;
    }

    OS << " {\n";

    for (auto &S : FN->get_slots()) {
        OS << "    ";
        print_slot(OS, S);
        OS << "\n";
    }

    if (!FN->get_slots().empty())
        OS << "\n";

    for (BasicBlock *curr = FN->head(); curr != nullptr; curr = curr->get_next()) {
        print_block(OS, curr);
        if (curr->get_next())
            OS << "\n";
    }

    OS << "}\n";
}

void Segment::print(std::ostream &OS) const {
    OS << "target :: ";

    switch (m_Target.Arch) {
        case Arch::X86_64: OS << "x86_64 ";
    }

    switch (m_Target.OpSys) {
        case OS::Linux: OS << "linux ";
    }

    switch (m_Target.ABI) {
        case ABI::SystemV: OS << "system_v";
    }

    OS << "\n\n";

    for (auto &[ String, Type ] : m_Types) {
        if (!Type->is_struct_ty())
            continue;

        StructType *ST = static_cast<StructType *>(Type);
        OS << "struct " << ST->get_name() << " { ";
        for (auto &M: ST->get_members())
            OS << M->get_name() << (M != ST->get_members().back() ? ",\n" : "\n");

        OS << " }";
    }

    for (auto &[ String, Data ] : m_Data)
        print_data(OS, Data);

    if (!m_Data.empty())
        OS << "\n";
    
    unsigned i = 0;
    for (auto &[ String, Function ] : m_Functions) {
        print_function(OS, Function);
        if (++i != m_Functions.size())
            OS << "\n";
    }
}

void Argument::print(std::ostream &OS) const {
    OS << get_type()->get_name() << " %" << get_printed_name(this);
}

void Function::print(std::ostream &OS) const {
    OS << get_return_ty()->get_name() << " " << get_printed_name(this);
}

void BasicBlock::print(std::ostream &OS) const {
    OS << "#" << get_printed_name(this);
}

void Data::print(std::ostream &OS) const {
    OS << get_type()->get_name() << " @" << get_printed_name(this);
}

void Slot::print(std::ostream &OS) const {
    OS << get_type()->get_name() << " _" << get_printed_name(this);
}

void ConstantInt::print(std::ostream &OS) const {
    OS << m_Type->get_name() << " " << m_Value;
}

void ConstantFP::print(std::ostream &OS) const {
    OS << m_Type->get_name() << " " << std::fixed << std::setprecision(6) 
       << m_Value;
}

void ConstantNil::print(std::ostream &OS) const {
    OS << m_Type->get_name() << " nil";
}

void ConstantString::print(std::ostream &OS) const {
    OS << m_Type->get_name() << " \"";
    for (unsigned i = 0, n = m_Value.size(); i != n; ++i) {
        switch (m_Value[i]) {
        case '\n':
            OS << "\\n";
            break;
        case '\t':
            OS << "\\t";
            break;
        case '\r':
            OS << "\\r";
            break;
        case '\0':
            OS << "\\0";
            break;
        case '\\':
            OS << "\\\\";
            break;
        case '\"':
            OS << "\\\"";
            break;
        default:
            OS << m_Value[i];
            break;
        }
    }
    OS << "\\0\"";
}

void ConstantAggregate::print(std::ostream &OS) const {
    OS << m_Type->get_name() << " [ ";
    for (auto &V : m_Values) {
        V->print(OS);
        OS << ", ";
    }
    OS << " ]";
}

void PHINode::print(std::ostream &OS) const {
    OS << get_type()->get_name() << " $" << get_printed_name(this);
}

void APInst::print(std::ostream &OS) const {
    OS << get_type()->get_name() << " $" << get_printed_name(this);
}

void LoadInst::print(std::ostream &OS) const {
    OS << get_type()->get_name() << " $" << get_printed_name(this);
}

void BinopInst::print(std::ostream &OS) const {
    OS << get_type()->get_name() << " $" << get_printed_name(this);
}

void UnopInst::print(std::ostream &OS) const {
    OS << get_type()->get_name() << " $" << get_printed_name(this);
}

void CMPInst::print(std::ostream &OS) const {
    OS << get_type()->get_name() << " $" << get_printed_name(this);
}
