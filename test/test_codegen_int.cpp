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
#include <sstream>

using mir::Segment;
using mir::Target;

namespace meddle {

namespace test {

class IntegratedCodegenTest : public ::testing::Test {
protected:
    void SetUp() override {
        mir::clear_bb_dict();
        mir::clear_inst_dict();
    }

    void TearDown() override {}
};

#define INT_CGN_1 R"(test::() i64 { ret 0; })"
TEST_F(IntegratedCodegenTest, Return_Zero) {
    File file = File("", "", "", INT_CGN_1);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    NameResolution NR = NameResolution(Options(), unit);
    Sema sema = Sema(Options(), unit);

    Target target = Target(mir::Arch::X86_64, mir::OS::Linux, 
                           mir::ABI::SystemV);

    Segment *seg = new Segment(target);
    CGN cgn = CGN(Options(), unit, seg);

    std::stringstream ss;
    seg->print(ss);

    String expected = R"(target :: x86_64 linux system_v

test :: () -> i64 {
entry:
    ret i64 0
}

)";
    EXPECT_EQ(ss.str(), expected);
        
    delete seg;
    delete unit;
}

#define INT_CGN_2 R"(test::() i64 { fix x: i64 = 42; ret x; })"
TEST_F(IntegratedCodegenTest, Return_Reference) {
    File file = File("", "", "", INT_CGN_2);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    NameResolution NR = NameResolution(Options(), unit);
    Sema sema = Sema(Options(), unit);

    Target target = Target(mir::Arch::X86_64, mir::OS::Linux, 
                           mir::ABI::SystemV);

    Segment *seg = new Segment(target);
    CGN cgn = CGN(Options(), unit, seg);

    std::stringstream ss;
    seg->print(ss);

    String expected = R"(target :: x86_64 linux system_v

test :: () -> i64 {
    $x := slot i64, align 8

entry:
    store i64 42, i64* $x
    %x.val := load i64* $x
    ret i64 %x.val
}

)";
    EXPECT_EQ(ss.str(), expected);
    
    delete seg;
    delete unit;
}

#define INT_CGN_3 R"(test::() { if 1 { ret; } })"
TEST_F(IntegratedCodegenTest, If_Then) {
    File file = File("", "", "", INT_CGN_3);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    NameResolution NR = NameResolution(Options(), unit);
    Sema sema = Sema(Options(), unit);

    Target target = Target(mir::Arch::X86_64, mir::OS::Linux, 
                           mir::ABI::SystemV);

    Segment *seg = new Segment(target);
    CGN cgn = CGN(Options(), unit, seg);

    std::stringstream ss;
    seg->print(ss);

    String expected = R"(target :: x86_64 linux system_v

test :: () {
entry:
    brif i64 1, if.then, if.merge

if.then:
    ret

if.merge:
    ret
}

)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define INT_CGN_4 R"(test::() { if 1 { ret; } else { ret; } })"
TEST_F(IntegratedCodegenTest, If_Then_Else) {
    File file = File("", "", "", INT_CGN_4);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    NameResolution NR = NameResolution(Options(), unit);
    Sema sema = Sema(Options(), unit);

    Target target = Target(mir::Arch::X86_64, mir::OS::Linux, 
                           mir::ABI::SystemV);

    Segment *seg = new Segment(target);
    CGN cgn = CGN(Options(), unit, seg);

    std::stringstream ss;
    seg->print(ss);

    String expected = R"(target :: x86_64 linux system_v

test :: () {
entry:
    brif i64 1, if.then, if.else

if.then:
    ret

if.else:
    ret
}

)";
    EXPECT_EQ(ss.str(), expected);
    
    delete seg;
    delete unit;
}

#define INT_CGN_5 R"(test::() { if 1 { ret; } else if 2 { ret; } else { ret; } })"
TEST_F(IntegratedCodegenTest, If_Then_ElseIf_Else) {
    File file = File("", "", "", INT_CGN_5);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    NameResolution NR = NameResolution(Options(), unit);
    Sema sema = Sema(Options(), unit);

    Target target = Target(mir::Arch::X86_64, mir::OS::Linux, 
                           mir::ABI::SystemV);

    Segment *seg = new Segment(target);
    CGN cgn = CGN(Options(), unit, seg);

    std::stringstream ss;
    seg->print(ss);

    String expected = R"(target :: x86_64 linux system_v

test :: () {
entry:
    brif i64 1, if.then, if.else

if.then:
    ret

if.else:
    brif i64 2, if.then1, if.else1

if.then1:
    ret

if.else1:
    ret
}

)";
    EXPECT_EQ(ss.str(), expected);
    
    delete seg;
    delete unit;
}

#define INT_CGN_6 R"(test::() { until 1 { ret; } })"
TEST_F(IntegratedCodegenTest, Until_Basic) {
    File file = File("", "", "", INT_CGN_6);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    NameResolution NR = NameResolution(Options(), unit);
    Sema sema = Sema(Options(), unit);

    Target target = Target(mir::Arch::X86_64, mir::OS::Linux, 
                           mir::ABI::SystemV);

    Segment *seg = new Segment(target);
    CGN cgn = CGN(Options(), unit, seg);

    std::stringstream ss;
    seg->print(ss);

    String expected = R"(target :: x86_64 linux system_v

test :: () {
entry:
    jmp until.cond

until.cond:
    brif i64 1, until.merge, until.body

until.body:
    ret

until.merge:
    ret
}

)";
    EXPECT_EQ(ss.str(), expected);
    
    delete seg;
    delete unit;
}

#define INT_CGN_7 R"(test::() { until 1 { continue; } })"
TEST_F(IntegratedCodegenTest, Until_Continue) {
    File file = File("", "", "", INT_CGN_7);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    NameResolution NR = NameResolution(Options(), unit);
    Sema sema = Sema(Options(), unit);

    Target target = Target(mir::Arch::X86_64, mir::OS::Linux, 
                           mir::ABI::SystemV);

    Segment *seg = new Segment(target);
    CGN cgn = CGN(Options(), unit, seg);

    std::stringstream ss;
    seg->print(ss);

    String expected = R"(target :: x86_64 linux system_v

test :: () {
entry:
    jmp until.cond

until.cond:
    brif i64 1, until.merge, until.body

until.body:
    jmp until.cond

until.merge:
    ret
}

)";
    EXPECT_EQ(ss.str(), expected);
    
    delete seg;
    delete unit;
}

#define INT_CGN_8 R"(test::() { until 1 { break; } })"
TEST_F(IntegratedCodegenTest, Until_Break) {
    File file = File("", "", "", INT_CGN_8);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    NameResolution NR = NameResolution(Options(), unit);
    Sema sema = Sema(Options(), unit);

    Target target = Target(mir::Arch::X86_64, mir::OS::Linux, 
                           mir::ABI::SystemV);

    Segment *seg = new Segment(target);
    CGN cgn = CGN(Options(), unit, seg);

    std::stringstream ss;
    seg->print(ss);

    String expected = R"(target :: x86_64 linux system_v

test :: () {
entry:
    jmp until.cond

until.cond:
    brif i64 1, until.merge, until.body

until.body:
    jmp until.merge

until.merge:
    ret
}

)";
    EXPECT_EQ(ss.str(), expected);
    
    delete seg;
    delete unit;
}

#define INT_CGN_9 R"(test::() { until 1 { if 2 { continue; } else break; } })"
TEST_F(IntegratedCodegenTest, Until_If_Continue_Break) {
    File file = File("", "", "", INT_CGN_9);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    NameResolution NR = NameResolution(Options(), unit);
    Sema sema = Sema(Options(), unit);

    Target target = Target(mir::Arch::X86_64, mir::OS::Linux, 
                           mir::ABI::SystemV);

    Segment *seg = new Segment(target);
    CGN cgn = CGN(Options(), unit, seg);

    std::stringstream ss;
    seg->print(ss);

    String expected = R"(target :: x86_64 linux system_v

test :: () {
entry:
    jmp until.cond

until.cond:
    brif i64 1, until.merge, until.body

until.body:
    brif i64 2, if.then, if.else

if.then:
    jmp until.cond

if.else:
    jmp until.merge

until.merge:
    ret
}

)";
    EXPECT_EQ(ss.str(), expected);
    
    delete seg;
    delete unit;
}

} // namespace test

} // namespace meddle
