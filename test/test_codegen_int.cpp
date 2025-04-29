#include "../compiler/cgn/codegen.h"
#include "../compiler/lexer/lexer.h"
#include "../compiler/mir/basicblock.h"
#include "../compiler/mir/function.h"
#include "../compiler/mir/segment.h"
#include "../compiler/parser/parser.h"
#include "../compiler/tree/decl.h"
#include "../compiler/tree/sema.h"

#include "gtest/gtest.h"
#include <gtest/gtest.h>

using mir::Segment;
using mir::Target;

namespace meddle {

namespace test {

class IntegratedCodegenTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

#define INTCGN_1 R"(test::() i64 { ret 0; })"
TEST_F(IntegratedCodegenTest, Return_Zero) {
    File file = File("", "", "", INTCGN_1);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    NameResolution NR = NameResolution(Options(), unit);
    Sema sema = Sema(Options(), unit);

    Target target = Target(Target::Arch::X86_64, Target::OS::Linux, 
                           Target::ABI::SysV);

    Segment *seg = new Segment(target);
    CGN cgn = CGN(Options(), unit, seg);

    std::vector<mir::Function *> FNs = seg->get_functions();
    EXPECT_EQ(FNs.size(), 1);

    mir::Function *FN = FNs.at(0);
    EXPECT_EQ(FN->get_args().size(), 0);
    EXPECT_EQ(FN->get_parent(), seg);

    mir::BasicBlock *entry = FN->head();
    EXPECT_NE(entry, nullptr);
    EXPECT_EQ(entry->get_parent(), FN);
    EXPECT_EQ(entry->get_name(), "entry");
    EXPECT_EQ(entry->has_preds(), false);
    EXPECT_EQ(entry->has_succs(), false);
    EXPECT_EQ(entry->has_terminator(), true);

    mir::Inst *inst = entry->head();
    EXPECT_NE(inst, nullptr);
    EXPECT_EQ(inst->get_parent(), entry);
    EXPECT_EQ(inst->get_next(), nullptr);
    EXPECT_EQ(inst->get_prev(), nullptr);

    mir::RetInst *ret = dynamic_cast<mir::RetInst *>(inst);
    EXPECT_NE(ret, nullptr);
    
    mir::Value *val = ret->get_value();
    EXPECT_NE(val, nullptr);

    mir::ConstantInt *integer = dynamic_cast<mir::ConstantInt *>(val);
    EXPECT_NE(integer, nullptr);
    EXPECT_EQ(integer->get_value(), 0);
        
    delete seg;
    delete unit;
}

} // namespace test

} // namespace meddle
