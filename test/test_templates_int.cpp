#include "../compiler/cgn/codegen.h"
#include "../compiler/lexer/lexer.h"
#include "../compiler/mir/basicblock.h"
#include "../compiler/mir/function.h"
#include "../compiler/mir/segment.h"
#include "../compiler/parser/parser.h"
#include "../compiler/tree/unitman.h"

#include <fstream>
#include <gtest/gtest.h>
#include <sstream>

using mir::Segment;
using mir::Target;

namespace meddle {

namespace test {

class IntegratedTemplateTest : public ::testing::Test {
protected:
    std::ofstream m_File;

    void SetUp() override {
        mir::clear_bb_dict();
        mir::clear_inst_dict();
        
        // create a new file /test.mdl
        m_File = std::ofstream("/test.mdl", std::ios::out | std::ios::trunc);
    }

    void TearDown() override {
        m_File.close();
    }
};

#define TEMPLATE_FUNCTION_SPEC R"(foo<T> :: (x: T) -> T { ret x + 1; } bar :: () -> i64 { ret foo<i32>(5); })"
TEST_F(IntegratedTemplateTest, Templated_Function_Specialized) {
    File file = File("test.mdl", "/", "/test.mdl", TEMPLATE_FUNCTION_SPEC);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    UnitManager units;
    units.addVirtUnit(unit);
    units.drive(Options());

    Target target = Target(mir::Arch::X86_64, mir::OS::Linux, 
                           mir::ABI::SystemV);

    Segment *seg = new Segment(target);
    CGN cgn = CGN(Options(), unit, seg);

    std::stringstream ss;
    seg->print(ss);

    String expected = R"(target :: x86_64 linux system_v

bar :: () -> i64 {
4:
    $5 := trunc i64 5 -> i32
    $6 := call i32 foo<i32>(i32 $5)
    $7 := sext i32 $6 -> i64
    ret i64 $7
}

foo<i32> :: (i32 %x) -> i32 {
    _x := slot i32, align 4

1:
    str i32 %x -> i32* _x, align 4
    $2 := load i32* _x, align 4
    $3 := add i32 $2, i64 1
    ret i32 $3
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
}

} // namespace test

} // namespace meddle
