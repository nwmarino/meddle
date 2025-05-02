#include "../compiler/cgn/codegen.h"
#include "../compiler/lexer/lexer.h"
#include "../compiler/mir/basicblock.h"
#include "../compiler/mir/function.h"
#include "../compiler/mir/segment.h"
#include "../compiler/parser/parser.h"
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
    str i64 42 -> i64* $x
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

test :: () -> void {
entry:
    %int.cmp := icmp_ne i64 1, i64 0
    brif i1 %int.cmp, if.then, if.merge

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

test :: () -> void {
entry:
    %int.cmp := icmp_ne i64 1, i64 0
    brif i1 %int.cmp, if.then, if.else

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

test :: () -> void {
entry:
    %int.cmp := icmp_ne i64 1, i64 0
    brif i1 %int.cmp, if.then, if.else

if.then:
    ret

if.else:
    %int.cmp1 := icmp_ne i64 2, i64 0
    brif i1 %int.cmp1, if.then1, if.else1

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

test :: () -> void {
entry:
    jmp until.cond

until.cond:
    %int.cmp := icmp_ne i64 1, i64 0
    brif i1 %int.cmp, until.merge, until.body

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

test :: () -> void {
entry:
    jmp until.cond

until.cond:
    %int.cmp := icmp_ne i64 1, i64 0
    brif i1 %int.cmp, until.merge, until.body

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

test :: () -> void {
entry:
    jmp until.cond

until.cond:
    %int.cmp := icmp_ne i64 1, i64 0
    brif i1 %int.cmp, until.merge, until.body

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

test :: () -> void {
entry:
    jmp until.cond

until.cond:
    %int.cmp := icmp_ne i64 1, i64 0
    brif i1 %int.cmp, until.merge, until.body

until.body:
    %int.cmp1 := icmp_ne i64 2, i64 0
    brif i1 %int.cmp1, if.then, if.else

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

#define INT_CGN_10 R"(test::() { fix x: char[7] = "hello\n"; })"
TEST_F(IntegratedCodegenTest, String_Declaration_Copy) {
    File file = File("", "", "", INT_CGN_10);
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

str :: readonly i8[7] "hello\n\0", align 1

test :: () -> void {
    $x := slot i8[7], align 1

entry:
    cpy i64 7, i8[7]* @str, align 1 -> i8[7]* $x, align 1
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);
    
    delete seg;
    delete unit;
}

#define INT_CGN_11 R"(test::() { fix x: i32 = cast<i32> 5; })"
TEST_F(IntegratedCodegenTest, Truncate_Integer) {
    File file = File("", "", "", INT_CGN_11);
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

test :: () -> void {
    $x := slot i32, align 4

entry:
    %cast.trunc := trunc i64 5 -> i32
    str i32 %cast.trunc -> i32* $x
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);
    
    delete seg;
    delete unit;
}

#define INT_CGN_12 R"(test::() { fix x: i64 = cast<i64> cast<i32> 5;})"
TEST_F(IntegratedCodegenTest, Truncate_SExt_Integer) {
    File file = File("", "", "", INT_CGN_12);
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

test :: () -> void {
    $x := slot i64, align 8

entry:
    %cast.trunc := trunc i64 5 -> i32
    %cast.sext := sext i32 %cast.trunc -> i64
    str i64 %cast.sext -> i64* $x
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);
    
    delete seg;
    delete unit;
}

#define INT_CGN_13 R"(test::() { fix x: f32 = cast<f32> 3.14; })"
TEST_F(IntegratedCodegenTest, Truncate_Float) {
    File file = File("", "", "", INT_CGN_13);
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

test :: () -> void {
    $x := slot f32, align 4

entry:
    %cast.ftrunc := ftrunc f64 3.14 -> f32
    str f32 %cast.ftrunc -> f32* $x
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);
    
    delete seg;
    delete unit;
}

#define INT_CGN_14 R"(test::() { fix x: f64 = cast<f64> cast<f32> 3.14; })"
TEST_F(IntegratedCodegenTest, Truncate_FExt_Float) {
    File file = File("", "", "", INT_CGN_14);
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

test :: () -> void {
    $x := slot f64, align 8

entry:
    %cast.ftrunc := ftrunc f64 3.14 -> f32
    %cast.fext := fext f32 %cast.ftrunc -> f64
    str f64 %cast.fext -> f64* $x
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);
    
    delete seg;
    delete unit;
}

#define INT_CGN_15 R"(test::() { fix x: i64 = 3.14; })"
TEST_F(IntegratedCodegenTest, Float_To_SInt) {
    File file = File("", "", "", INT_CGN_15);
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

test :: () -> void {
    $x := slot i64, align 8

entry:
    %cast.cvt := fp2si f64 3.14 -> i64
    str i64 %cast.cvt -> i64* $x
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define INT_CGN_16 R"(test::() { fix x: u64 = 3.14; })"
TEST_F(IntegratedCodegenTest, Float_To_UInt) {
    File file = File("", "", "", INT_CGN_16);
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

test :: () -> void {
    $x := slot i64, align 8

entry:
    %cast.cvt := fp2ui f64 3.14 -> i64
    str i64 %cast.cvt -> i64* $x
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define INT_CGN_17 R"(test::() { fix x: f64 = 5; })"
TEST_F(IntegratedCodegenTest, SInt_To_Float) {
    File file = File("", "", "", INT_CGN_17);
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

test :: () -> void {
    $x := slot f64, align 8

entry:
    %cast.cvt := si2fp i64 5 -> f64
    str f64 %cast.cvt -> f64* $x
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define INT_CGN_18 R"(test::() { fix x: f64 = cast<u64> 5; })"
TEST_F(IntegratedCodegenTest, UInt_To_Float) {
    File file = File("", "", "", INT_CGN_18);
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

test :: () -> void {
    $x := slot f64, align 8

entry:
    %cast.cvt := ui2fp i64 5 -> f64
    str f64 %cast.cvt -> f64* $x
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define INT_CGN_19 R"(test::() { fix x: i64* = nil; })"
TEST_F(IntegratedCodegenTest, Pointer_Reinterpret) {
    File file = File("", "", "", INT_CGN_19);
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

test :: () -> void {
    $x := slot i64*, align 8

entry:
    %cast.ptr := reint void* nil -> i64*
    str i64* %cast.ptr -> i64** $x
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define INT_CGN_20 R"(test::() i32 { match 5 { 1 -> { ret 0; } 2 -> { ret 42; } _ -> ret 1; } })"
TEST_F(IntegratedCodegenTest, Match_Basic) {
    File file = File("", "", "", INT_CGN_20);
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

test :: () -> i32 {
entry:
    jmp match.chain

match.chain:
    %match.cmp := icmp_eq i64 5, i64 1
    brif i1 %match.cmp, match.case, match.chain1

match.case:
    %cast.trunc := trunc i64 0 -> i32
    ret i32 %cast.trunc

match.chain1:
    %match.cmp1 := icmp_eq i64 5, i64 2
    brif i1 %match.cmp1, match.case1, match.def

match.case1:
    %cast.trunc1 := trunc i64 42 -> i32
    ret i32 %cast.trunc1

match.def:
    %cast.trunc2 := trunc i64 1 -> i32
    ret i32 %cast.trunc2
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define INT_CGN_21 R"(test::() { fix x: bool = true; })"
TEST_F(IntegratedCodegenTest, Bool_Declaration) {
    File file = File("", "", "", INT_CGN_21);
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

test :: () -> void {
    $x := slot i8, align 1

entry:
    str i8 1 -> i8* $x
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define INT_CGN_22 R"(test::() { fix x: bool = (false); })"
TEST_F(IntegratedCodegenTest, Bool_Declaration_Parentheses) {
    File file = File("", "", "", INT_CGN_22);
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

test :: () -> void {
    $x := slot i8, align 1

entry:
    str i8 0 -> i8* $x
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define SIZEOF_BASIC R"(test::() { fix x: u64 = sizeof<i32>; })"
TEST_F(IntegratedCodegenTest, Sizeof_Basic) {
    File file = File("", "", "", SIZEOF_BASIC);
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

test :: () -> void {
    $x := slot i64, align 8

entry:
    str i64 4 -> i64* $x
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define GLOBAL_VAR_BASIC R"(global :: fix i64 = 0;)"
TEST_F(IntegratedCodegenTest, Global_Var_Basic) {
    File file = File("", "", "", GLOBAL_VAR_BASIC);
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

global :: readonly i64 0, align 8

)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_ASSIGN_SMALL R"(test::() { mut x: i64 = 0; x = 1; })"
TEST_F(IntegratedCodegenTest, Binary_Small_Assign) {

}

#define BINARY_ASSIGN_BIG R"(test::() { mut x: char[8] = "ayoayo\n"; x = "alrcya\n"; })"
TEST_F(IntegratedCodegenTest, Binary_Big_Assign) {

}

#define BINARY_INT_ADD_ASSIGN R"(test::() { mut x: i64 = 0; x += 1; })"
TEST_F(IntegratedCodegenTest, Binary_Int_Add_Assign) {

}

#define BINARY_FP_ADD_ASSIGN R"(test::() { mut x: f64 = 0.0; x += 1.0; })"
TEST_F(IntegratedCodegenTest, Binary_FP_Add_Assign) {

}

#define BINARY_INT_SUB_ASSIGN R"(test::() { mut x: i64 = 0; x -= 1; })"
TEST_F(IntegratedCodegenTest, Binary_Int_Sub_Assign) {

}

#define BINARY_FP_SUB_ASSIGN R"(test::() { mut x: f64 = 0.0; x -= 1.0; })"
TEST_F(IntegratedCodegenTest, Binary_FP_Sub_Assign) {

}

#define BINARY_SINT_MUL_ASSIGN R"(test::() { mut x: i64 = 0; x *= 1; })"
TEST_F(IntegratedCodegenTest, Binary_SInt_Mul_Assign) {

}

#define BINARY_UINT_MUL_ASSIGN R"(test::() { mut x: u64 = 0; x *= 1; })"
TEST_F(IntegratedCodegenTest, Binary_UInt_Mul_Assign) {
    
}

#define BINARY_FP_MUL_ASSIGN R"(test::() { mut x: f64 = 0.0; x *= 1.0; })"
TEST_F(IntegratedCodegenTest, Binary_FP_Mul_Assign) {

}

#define BINARY_SINT_DIV_ASSIGN R"(test::() { mut x: i64 = 0; x /= 1; })"
TEST_F(IntegratedCodegenTest, Binary_SInt_Div_Assign) {

}

#define BINARY_UINT_DIV_ASSIGN R"(test::() { mut x: u64 = 0; x /= 1; })"
TEST_F(IntegratedCodegenTest, Binary_UInt_Div_Assign) {

}

#define BINARY_FP_DIV_ASSIGN R"(test::() { mut x: f64 = 0.0; x /= 1.0; })"
TEST_F(IntegratedCodegenTest, Binary_FP_Div_Assign) {

}

#define BINARY_SINT_MOD_ASSIGN R"(test::() { mut x: i64 = 0; x %= 1; })"
TEST_F(IntegratedCodegenTest, Binary_SInt_Mod_Assign) {

}

#define BINARY_UINT_MOD_ASSIGN R"(test::() { mut x: u64 = 0; x %= 1; })"
TEST_F(IntegratedCodegenTest, Binary_UInt_Mod_Assign) {

}

#define BINARY_AND_ASSIGN R"(test::() { mut x: i64 = 0; x &= 1; })"
TEST_F(IntegratedCodegenTest, Binary_And_Assign) {

}

#define BINARY_OR_ASSIGN R"(test::() { mut x: i64 = 0; x |= 1; })"
TEST_F(IntegratedCodegenTest, Binary_Or_Assign) {

}

#define BINARY_XOR_ASSIGN R"(test::() { mut x: i64 = 0; x ^= 1; })"
TEST_F(IntegratedCodegenTest, Binary_Xor_Assign) {

}

#define BINARY_SHL_ASSIGN R"(test::() { mut x: i64 = 0; x <<= 1; })"
TEST_F(IntegratedCodegenTest, Binary_Shl_Assign) {

}

#define BINARY_SINT_SHR_ASSIGN R"(test::() { mut x: i64 = 0; x >>= 2; })"
TEST_F(IntegratedCodegenTest, Binary_SInt_Shr_Assign) {

}

#define BINARY_UINT_SHR_ASSIGN R"(test::() { mut x: u64 = 0; x >>= 2; })"
TEST_F(IntegratedCodegenTest, Binary_UInt_Shr_Assign) {

}

#define BINARY_INT_EQUALS R"(test::() { mut x: i64 = 0; x == 1; })"
TEST_F(IntegratedCodegenTest, Binary_Int_Equals) {

}

#define BINARY_INT_NOT_EQUALS R"(test::() { mut x: i64 = 0; x != 1; })"
TEST_F(IntegratedCodegenTest, Binary_Int_Not_Equals) {

}

#define BINARY_SINT_LESS R"(test::() { mut x: i64 = 0; x < 1; })"
TEST_F(IntegratedCodegenTest, Binary_SInt_Less) {

}

#define BINARY_UINT_LESS R"(test::() { mut x: u64 = 0; x < 1; })"
TEST_F(IntegratedCodegenTest, Binary_UInt_Less) {

}

#define BINARY_FP_LESS R"(test::() { mut x: f64 = 0.0; x < 1.0; })"
TEST_F(IntegratedCodegenTest, Binary_FP_Less) {

}

#define BINARY_PTR_LESS R"(test::() { mut x: i64* = nil; x < nil; })"
TEST_F(IntegratedCodegenTest, Binary_Ptr_Less) {

}

#define BINARY_SINT_LESS_EQUALS R"(test::() { mut x: i64 = 0; x <= 1; })"
TEST_F(IntegratedCodegenTest, Binary_SInt_Less_Equals) {

}

#define BINARY_UINT_LESS_EQUALS R"(test::() { mut x: u64 = 0; x <= 1; })"
TEST_F(IntegratedCodegenTest, Binary_UInt_Less_Equals) {

}

#define BINARY_FP_LESS_EQUALS R"(test::() { mut x: f64 = 0.0; x <= 1.0; })"
TEST_F(IntegratedCodegenTest, Binary_FP_Less_Equals) {

}

#define BINARY_PTR_LESS_EQUALS R"(test::() { mut x: i64* = nil; x <= nil; })"
TEST_F(IntegratedCodegenTest, Binary_Ptr_Less_Equals) {

}

#define BINARY_SINT_GREATER R"(test::() { mut x: i64 = 0; x > 1; })"
TEST_F(IntegratedCodegenTest, Binary_SInt_Greater) {

}

#define BINARY_UINT_GREATER R"(test::() { mut x: u64 = 0; x > 1; })"
TEST_F(IntegratedCodegenTest, Binary_UInt_Greater) {

}

#define BINARY_FP_GREATER R"(test::() { mut x: f64 = 0.0; x > 1.0; })"
TEST_F(IntegratedCodegenTest, Binary_FP_Greater) {

}

#define BINARY_PTR_GREATER R"(test::() { mut x: i64* = nil; x > nil; })"
TEST_F(IntegratedCodegenTest, Binary_Ptr_Greater) {

}

#define BINARY_SINT_GREATER_EQUALS R"(test::() { mut x: i64 = 0; x >= 1; })"
TEST_F(IntegratedCodegenTest, Binary_SInt_Greater_Equals) {

}

#define BINARY_UINT_GREATER_EQUALS R"(test::() { mut x: u64 = 0; x >= 1; })"
TEST_F(IntegratedCodegenTest, Binary_UInt_Greater_Equals) {

}

#define BINARY_FP_GREATER_EQUALS R"(test::() { mut x: f64 = 0.0; x >= 1.0; })"
TEST_F(IntegratedCodegenTest, Binary_FP_Greater_Equals) {

}

#define BINARY_PTR_GREATER_EQUALS R"(test::() { mut x: i64* = nil; x >= nil; })"
TEST_F(IntegratedCodegenTest, Binary_Ptr_Greater_Equals) {

}

#define BINARY_AND R"(test::() { mut x: i64 = 0; x & 1; })"
TEST_F(IntegratedCodegenTest, Binary_And) {

}

#define BINARY_OR R"(test::() { mut x: i64 = 0; x | 1; })"
TEST_F(IntegratedCodegenTest, Binary_Or) {

}

#define BINARY_XOR R"(test::() { mut x: i64 = 0; x ^ 1; })"
TEST_F(IntegratedCodegenTest, Binary_Xor) {

}

#define BINARY_SHL R"(test::() { mut x: i64 = 0; x << 2; })"
TEST_F(IntegratedCodegenTest, Binary_Shl) {

}

#define BINARY_SINT_SHR R"(test::() { mut x: i64 = 0; x >> 2; })"
TEST_F(IntegratedCodegenTest, Binary_SInt_Shr) {

}

#define BINARY_UINT_SHR R"(test::() { mut x: u64 = 0; x >> 2; })"
TEST_F(IntegratedCodegenTest, Binary_UInt_Shr) {

}

#define BINARY_INT_ADD R"(test::() { mut x: i64 = 0; x + 1; })"
TEST_F(IntegratedCodegenTest, Binary_Int_Add) {

}

#define BINARY_FP_ADD R"(test::() { mut x: f64 = 0.0; x + 1.0; })"
TEST_F(IntegratedCodegenTest, Binary_FP_Add) {

}

#define BINARY_INT_SUB R"(test::() { mut x: i64 = 0; x - 1; })"
TEST_F(IntegratedCodegenTest, Binary_Int_Sub) {

}

#define BINARY_FP_SUB R"(test::() { mut x: f64 = 0.0; x - 1.0; })"
TEST_F(IntegratedCodegenTest, Binary_FP_Sub) {

}

#define BINARY_SINT_MUL R"(test::() { mut x: i64 = 0; x * 1; })"
TEST_F(IntegratedCodegenTest, Binary_SInt_Mul) {

}

#define BINARY_UINT_MUL R"(test::() { mut x: u64 = 0; x * 1; })"
TEST_F(IntegratedCodegenTest, Binary_UInt_Mul) {

}

#define BINARY_FP_MUL R"(test::() { mut x: f64 = 0.0; x * 1.0; })"
TEST_F(IntegratedCodegenTest, Binary_FP_Mul) {

}

#define BINARY_SINT_DIV R"(test::() { mut x: i64 = 0; x / 1; })"
TEST_F(IntegratedCodegenTest, Binary_SInt_Div) {

}

#define BINARY_UINT_DIV R"(test::() { mut x: u64 = 0; x / 1; })"
TEST_F(IntegratedCodegenTest, Binary_UInt_Div) {

}

#define BINARY_FP_DIV R"(test::() { mut x: f64 = 0.0; x / 1.0; })"
TEST_F(IntegratedCodegenTest, Binary_FP_Div) {

}

#define BINARY_SINT_MOD R"(test::() { mut x: i64 = 0; x % 1; })"
TEST_F(IntegratedCodegenTest, Binary_SInt_Mod) {

}

#define BINARY_UINT_MOD R"(test::() { mut x: u64 = 0; x % 1; })"
TEST_F(IntegratedCodegenTest, Binary_UInt_Mod) {

}

} // namespace test

} // namespace meddle
