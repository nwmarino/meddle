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
1:
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
    _x := slot i64, align 8

1:
    str i64 42 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    ret i64 $2
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
1:
    $2 := icmp_ne i64 1, i64 0
    brif i1 $2, #3, #4

3 (1):
    ret

4 (1):
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
1:
    $2 := icmp_ne i64 1, i64 0
    brif i1 $2, #3, #4

3 (1):
    ret

4 (1):
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
1:
    $2 := icmp_ne i64 1, i64 0
    brif i1 $2, #3, #4

3 (1):
    ret

4 (1):
    $5 := icmp_ne i64 2, i64 0
    brif i1 $5, #6, #7

6 (4):
    ret

7 (4):
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
1:
    jmp #2

2 (1):
    $3 := icmp_ne i64 1, i64 0
    brif i1 $3, #5, #4

4 (2):
    ret

5 (2):
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
1:
    jmp #2

2 (1, 4):
    $3 := icmp_ne i64 1, i64 0
    brif i1 $3, #5, #4

4 (2):
    jmp #2

5 (2):
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
1:
    jmp #2

2 (1):
    $3 := icmp_ne i64 1, i64 0
    brif i1 $3, #5, #4

4 (2):
    jmp #5

5 (2, 4):
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
1:
    jmp #2

2 (1, 6):
    $3 := icmp_ne i64 1, i64 0
    brif i1 $3, #8, #4

4 (2):
    $5 := icmp_ne i64 2, i64 0
    brif i1 $5, #6, #7

6 (4):
    jmp #2

7 (4):
    jmp #8

8 (2, 7):
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

__const.str :: readonly i8[7] "hello\n\0", align 1

test :: () -> void {
    _x := slot i8[7], align 1

1:
    cpy i64 7, i8[7]* @__const.str, align 1 -> i8[7]* _x, align 1
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
    _x := slot i32, align 4

1:
    $2 := trunc i64 5 -> i32
    str i32 $2 -> i32* _x, align 4
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
    _x := slot i64, align 8

1:
    $2 := trunc i64 5 -> i32
    $3 := sext i32 $2 -> i64
    str i64 $3 -> i64* _x, align 8
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
    _x := slot f32, align 4

1:
    $2 := ftrunc f64 3.140000 -> f32
    str f32 $2 -> f32* _x, align 4
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
    _x := slot f64, align 8

1:
    $2 := ftrunc f64 3.140000 -> f32
    $3 := fext f32 $2 -> f64
    str f64 $3 -> f64* _x, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);
    
    delete seg;
    delete unit;
}

#define INT_CGN_15 R"(test::() { fix x: i64 = cast<i64> 3.14; })"
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
    _x := slot i64, align 8

1:
    $2 := fp2si f64 3.140000 -> i64
    str i64 $2 -> i64* _x, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define INT_CGN_16 R"(test::() { fix x: u64 = cast<u64> 3.14; })"
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
    _x := slot i64, align 8

1:
    $2 := fp2ui f64 3.140000 -> i64
    str i64 $2 -> i64* _x, align 8
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
    _x := slot f64, align 8

1:
    $2 := si2fp i64 5 -> f64
    str f64 $2 -> f64* _x, align 8
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
    _x := slot f64, align 8

1:
    $2 := ui2fp i64 5 -> f64
    str f64 $2 -> f64* _x, align 8
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
    _x := slot i64*, align 8

1:
    $2 := reint void* nil -> i64*
    str i64* $2 -> i64** _x, align 8
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
1:
    jmp #2

2 (1):
    $3 := icmp_eq i64 5, i64 1
    brif i1 $3, #4, #6

4 (2):
    $5 := trunc i64 0 -> i32
    ret i32 $5

6 (2):
    $7 := icmp_eq i64 5, i64 2
    brif i1 $7, #8, #10

8 (6):
    $9 := trunc i64 42 -> i32
    ret i32 $9

10 (6):
    $11 := trunc i64 1 -> i32
    ret i32 $11
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
    _x := slot i1, align 1

1:
    str i1 1 -> i1* _x, align 1
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
    _x := slot i1, align 1

1:
    str i1 0 -> i1* _x, align 1
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
    _x := slot i64, align 8

1:
    str i64 4 -> i64* _x, align 8
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
    File file = File("", "", "", BINARY_ASSIGN_SMALL);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    str i64 1 -> i64* _x, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_ASSIGN_BIG R"(test::() { mut x: char[8] = "ayoayo\n"; x = "alrcya\n"; })"
TEST_F(IntegratedCodegenTest, Binary_Big_Assign) {
    File file = File("", "", "", BINARY_ASSIGN_BIG);
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

__const.str2 :: readonly i8[8] "alrcya\n\0", align 1
__const.str1 :: readonly i8[8] "ayoayo\n\0", align 1

test :: () -> void {
    _x := slot i8[8], align 1

1:
    cpy i64 8, i8[8]* @__const.str1, align 1 -> i8[8]* _x, align 1
    cpy i64 8, i8[8]* @__const.str2, align 1 -> i8[8]* _x, align 1
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_INT_ADD_ASSIGN R"(test::() { mut x: i64 = 0; x += 1; })"
TEST_F(IntegratedCodegenTest, Binary_Int_Add_Assign) {
    File file = File("", "", "", BINARY_INT_ADD_ASSIGN);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := add i64 $2, i64 1
    str i64 $3 -> i64* _x, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_FP_ADD_ASSIGN R"(test::() { mut x: f64 = 0.5; x += 3.14; })"
TEST_F(IntegratedCodegenTest, Binary_FP_Add_Assign) {
    File file = File("", "", "", BINARY_FP_ADD_ASSIGN);
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
    _x := slot f64, align 8

1:
    str f64 0.500000 -> f64* _x, align 8
    $2 := load f64* _x, align 8
    $3 := fadd f64 $2, f64 3.140000
    str f64 $3 -> f64* _x, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_INT_SUB_ASSIGN R"(test::() { mut x: i64 = 0; x -= 1; })"
TEST_F(IntegratedCodegenTest, Binary_Int_Sub_Assign) {
    File file = File("", "", "", BINARY_INT_SUB_ASSIGN);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := sub i64 $2, i64 1
    str i64 $3 -> i64* _x, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_FP_SUB_ASSIGN R"(test::() { mut x: f64 = 0.0; x -= 1.0; })"
TEST_F(IntegratedCodegenTest, Binary_FP_Sub_Assign) {
    File file = File("", "", "", BINARY_FP_SUB_ASSIGN);
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
    _x := slot f64, align 8

1:
    str f64 0.000000 -> f64* _x, align 8
    $2 := load f64* _x, align 8
    $3 := fsub f64 $2, f64 1.000000
    str f64 $3 -> f64* _x, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_SINT_MUL_ASSIGN R"(test::() { mut x: i64 = 0; x *= 2; })"
TEST_F(IntegratedCodegenTest, Binary_SInt_Mul_Assign) {
    File file = File("", "", "", BINARY_SINT_MUL_ASSIGN);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := smul i64 $2, i64 2
    str i64 $3 -> i64* _x, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_UINT_MUL_ASSIGN R"(test::() { mut x: u64 = 0; x *= 2; })"
TEST_F(IntegratedCodegenTest, Binary_UInt_Mul_Assign) {
    File file = File("", "", "", BINARY_UINT_MUL_ASSIGN);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := umul i64 $2, i64 2
    str i64 $3 -> i64* _x, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_FP_MUL_ASSIGN R"(test::() { mut x: f64 = 0.0; x *= 1.14; })"
TEST_F(IntegratedCodegenTest, Binary_FP_Mul_Assign) {
    File file = File("", "", "", BINARY_FP_MUL_ASSIGN);
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
    _x := slot f64, align 8

1:
    str f64 0.000000 -> f64* _x, align 8
    $2 := load f64* _x, align 8
    $3 := fmul f64 $2, f64 1.140000
    str f64 $3 -> f64* _x, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_SINT_DIV_ASSIGN R"(test::() { mut x: i64 = 0; x /= 15; })"
TEST_F(IntegratedCodegenTest, Binary_SInt_Div_Assign) {
    File file = File("", "", "", BINARY_SINT_DIV_ASSIGN);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := sdiv i64 $2, i64 15
    str i64 $3 -> i64* _x, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_UINT_DIV_ASSIGN R"(test::() { mut x: u64 = 0; x /= 12; })"
TEST_F(IntegratedCodegenTest, Binary_UInt_Div_Assign) {
    File file = File("", "", "", BINARY_UINT_DIV_ASSIGN);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := udiv i64 $2, i64 12
    str i64 $3 -> i64* _x, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_FP_DIV_ASSIGN R"(test::() { mut x: f64 = 1.2; x /= 1.23; })"
TEST_F(IntegratedCodegenTest, Binary_FP_Div_Assign) {
    File file = File("", "", "", BINARY_FP_DIV_ASSIGN);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    FunctionDecl *A = dynamic_cast<FunctionDecl *>(unit->getDecls()[0]);
    CompoundStmt *B = dynamic_cast<CompoundStmt *>(A->getBody());
    ExprStmt *C = dynamic_cast<ExprStmt *>(B->getStmts()[1]);
    BinaryExpr *D = dynamic_cast<BinaryExpr *>(C->getExpr());
    FloatLiteral *E = dynamic_cast<FloatLiteral *>(D->getRHS());
    EXPECT_EQ(E->getValue(), 1.230000);

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
    _x := slot f64, align 8

1:
    str f64 1.200000 -> f64* _x, align 8
    $2 := load f64* _x, align 8
    $3 := fdiv f64 $2, f64 1.230000
    str f64 $3 -> f64* _x, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_SINT_MOD_ASSIGN R"(test::() { mut x: i64 = 12; x %= 2; })"
TEST_F(IntegratedCodegenTest, Binary_SInt_Mod_Assign) {
    File file = File("", "", "", BINARY_SINT_MOD_ASSIGN);
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
    _x := slot i64, align 8

1:
    str i64 12 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := srem i64 $2, i64 2
    str i64 $3 -> i64* _x, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_UINT_MOD_ASSIGN R"(test::() { mut x: u64 = 5; x %= 3; })"
TEST_F(IntegratedCodegenTest, Binary_UInt_Mod_Assign) {
    File file = File("", "", "", BINARY_UINT_MOD_ASSIGN);
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
    _x := slot i64, align 8

1:
    str i64 5 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := urem i64 $2, i64 3
    str i64 $3 -> i64* _x, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_AND_ASSIGN R"(test::() { mut x: i64 = 0; x &= 1; })"
TEST_F(IntegratedCodegenTest, Binary_And_Assign) {
    File file = File("", "", "", BINARY_AND_ASSIGN);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := and i64 $2, i64 1
    str i64 $3 -> i64* _x, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_OR_ASSIGN R"(test::() { mut x: i64 = 0; x |= 1; })"
TEST_F(IntegratedCodegenTest, Binary_Or_Assign) {
    File file = File("", "", "", BINARY_OR_ASSIGN);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := or i64 $2, i64 1
    str i64 $3 -> i64* _x, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_XOR_ASSIGN R"(test::() { mut x: i64 = 0; x ^= 1; })"
TEST_F(IntegratedCodegenTest, Binary_Xor_Assign) {
    File file = File("", "", "", BINARY_XOR_ASSIGN);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := xor i64 $2, i64 1
    str i64 $3 -> i64* _x, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_SHL_ASSIGN R"(test::() { mut x: i64 = 0; x <<= 1; })"
TEST_F(IntegratedCodegenTest, Binary_Shl_Assign) {
    File file = File("", "", "", BINARY_SHL_ASSIGN);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := shl i64 $2, i64 1
    str i64 $3 -> i64* _x, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_SINT_SHR_ASSIGN R"(test::() { mut x: i64 = 0; x >>= 2; })"
TEST_F(IntegratedCodegenTest, Binary_SInt_Shr_Assign) {
    File file = File("", "", "", BINARY_SINT_SHR_ASSIGN);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := ashr i64 $2, i64 2
    str i64 $3 -> i64* _x, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_UINT_SHR_ASSIGN R"(test::() { mut x: u64 = 0; x >>= 2; })"
TEST_F(IntegratedCodegenTest, Binary_UInt_Shr_Assign) {
    File file = File("", "", "", BINARY_UINT_SHR_ASSIGN);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := lshr i64 $2, i64 2
    str i64 $3 -> i64* _x, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_INT_EQUALS R"(test::() { mut x: i64 = 0; x == 1; })"
TEST_F(IntegratedCodegenTest, Binary_Int_Equals) {
    File file = File("", "", "", BINARY_INT_EQUALS);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := icmp_eq i64 $2, i64 1
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_INT_NOT_EQUALS R"(test::() { mut x: i64 = 0; x != 1; })"
TEST_F(IntegratedCodegenTest, Binary_Int_Not_Equals) {
    File file = File("", "", "", BINARY_INT_NOT_EQUALS);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := icmp_ne i64 $2, i64 1
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_SINT_LESS R"(test::() { mut x: i64 = 0; x < 1; })"
TEST_F(IntegratedCodegenTest, Binary_SInt_Less) {
    File file = File("", "", "", BINARY_SINT_LESS);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := icmp_slt i64 $2, i64 1
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_UINT_LESS R"(test::() { mut x: u64 = 0; x < 1; })"
TEST_F(IntegratedCodegenTest, Binary_UInt_Less) {
    File file = File("", "", "", BINARY_UINT_LESS);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := icmp_ult i64 $2, i64 1
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_FP_LESS R"(test::() { mut x: f64 = 0.0; x < 1.0; })"
TEST_F(IntegratedCodegenTest, Binary_FP_Less) {
    File file = File("", "", "", BINARY_FP_LESS);
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
    _x := slot f64, align 8

1:
    str f64 0.000000 -> f64* _x, align 8
    $2 := load f64* _x, align 8
    $3 := fcmp_olt f64 $2, f64 1.000000
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_PTR_LESS R"(test::() { mut x: i64* = nil; x < nil; })"
TEST_F(IntegratedCodegenTest, Binary_Ptr_Less) {
    File file = File("", "", "", BINARY_PTR_LESS);
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
    _x := slot i64*, align 8

1:
    $2 := reint void* nil -> i64*
    str i64* $2 -> i64** _x, align 8
    $3 := load i64** _x, align 8
    $4 := reint void* nil -> i64*
    $5 := pcmp_lt i64* $3, i64* $4
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_SINT_LESS_EQUALS R"(test::() { mut x: i64 = 0; x <= 1; })"
TEST_F(IntegratedCodegenTest, Binary_SInt_Less_Equals) {
    File file = File("", "", "", BINARY_SINT_LESS_EQUALS);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := icmp_sle i64 $2, i64 1
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_UINT_LESS_EQUALS R"(test::() { mut x: u64 = 0; x <= 1; })"
TEST_F(IntegratedCodegenTest, Binary_UInt_Less_Equals) {
    File file = File("", "", "", BINARY_UINT_LESS_EQUALS);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := icmp_ule i64 $2, i64 1
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_FP_LESS_EQUALS R"(test::() { mut x: f64 = 0.0; x <= 1.0; })"
TEST_F(IntegratedCodegenTest, Binary_FP_Less_Equals) {
    File file = File("", "", "", BINARY_FP_LESS_EQUALS);
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
    _x := slot f64, align 8

1:
    str f64 0.000000 -> f64* _x, align 8
    $2 := load f64* _x, align 8
    $3 := fcmp_ole f64 $2, f64 1.000000
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_PTR_LESS_EQUALS R"(test::() { mut x: i64* = nil; x <= nil; })"
TEST_F(IntegratedCodegenTest, Binary_Ptr_Less_Equals) {
    File file = File("", "", "", BINARY_PTR_LESS_EQUALS);
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
    _x := slot i64*, align 8

1:
    $2 := reint void* nil -> i64*
    str i64* $2 -> i64** _x, align 8
    $3 := load i64** _x, align 8
    $4 := reint void* nil -> i64*
    $5 := pcmp_le i64* $3, i64* $4
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_SINT_GREATER R"(test::() { mut x: i64 = 0; x > 1; })"
TEST_F(IntegratedCodegenTest, Binary_SInt_Greater) {
    File file = File("", "", "", BINARY_SINT_GREATER);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := icmp_sgt i64 $2, i64 1
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_UINT_GREATER R"(test::() { mut x: u64 = 0; x > 1; })"
TEST_F(IntegratedCodegenTest, Binary_UInt_Greater) {
    File file = File("", "", "", BINARY_UINT_GREATER);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := icmp_ugt i64 $2, i64 1
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_FP_GREATER R"(test::() { mut x: f64 = 0.0; x > 1.0; })"
TEST_F(IntegratedCodegenTest, Binary_FP_Greater) {
    File file = File("", "", "", BINARY_FP_GREATER);
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
    _x := slot f64, align 8

1:
    str f64 0.000000 -> f64* _x, align 8
    $2 := load f64* _x, align 8
    $3 := fcmp_ogt f64 $2, f64 1.000000
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_PTR_GREATER R"(test::() { mut x: i64* = nil; x > nil; })"
TEST_F(IntegratedCodegenTest, Binary_Ptr_Greater) {
    File file = File("", "", "", BINARY_PTR_GREATER);
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
    _x := slot i64*, align 8

1:
    $2 := reint void* nil -> i64*
    str i64* $2 -> i64** _x, align 8
    $3 := load i64** _x, align 8
    $4 := reint void* nil -> i64*
    $5 := pcmp_gt i64* $3, i64* $4
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_SINT_GREATER_EQUALS R"(test::() { mut x: i64 = 0; x >= 1; })"
TEST_F(IntegratedCodegenTest, Binary_SInt_Greater_Equals) {
    File file = File("", "", "", BINARY_SINT_GREATER_EQUALS);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := icmp_sge i64 $2, i64 1
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_UINT_GREATER_EQUALS R"(test::() { mut x: u64 = 0; x >= 1; })"
TEST_F(IntegratedCodegenTest, Binary_UInt_Greater_Equals) {
    File file = File("", "", "", BINARY_UINT_GREATER_EQUALS);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := icmp_uge i64 $2, i64 1
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_FP_GREATER_EQUALS R"(test::() { mut x: f64 = 0.0; x >= 1.0; })"
TEST_F(IntegratedCodegenTest, Binary_FP_Greater_Equals) {
    File file = File("", "", "", BINARY_FP_GREATER_EQUALS);
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
    _x := slot f64, align 8

1:
    str f64 0.000000 -> f64* _x, align 8
    $2 := load f64* _x, align 8
    $3 := fcmp_oge f64 $2, f64 1.000000
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_PTR_GREATER_EQUALS R"(test::() { mut x: i64* = nil; x >= nil; })"
TEST_F(IntegratedCodegenTest, Binary_Ptr_Greater_Equals) {
    File file = File("", "", "", BINARY_PTR_GREATER_EQUALS);
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
    _x := slot i64*, align 8

1:
    $2 := reint void* nil -> i64*
    str i64* $2 -> i64** _x, align 8
    $3 := load i64** _x, align 8
    $4 := reint void* nil -> i64*
    $5 := pcmp_ge i64* $3, i64* $4
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_AND R"(test::() { mut x: i64 = 0; x & 1; })"
TEST_F(IntegratedCodegenTest, Binary_And) {
    File file = File("", "", "", BINARY_AND);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := and i64 $2, i64 1
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_OR R"(test::() { mut x: i64 = 0; x | 1; })"
TEST_F(IntegratedCodegenTest, Binary_Or) {
    File file = File("", "", "", BINARY_OR);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := or i64 $2, i64 1
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_XOR R"(test::() { mut x: i64 = 0; x ^ 1; })"
TEST_F(IntegratedCodegenTest, Binary_Xor) {
    File file = File("", "", "", BINARY_XOR);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := xor i64 $2, i64 1
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_SHL R"(test::() { mut x: i64 = 0; x << 2; })"
TEST_F(IntegratedCodegenTest, Binary_Shl) {
    File file = File("", "", "", BINARY_SHL);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := shl i64 $2, i64 2
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_SINT_SHR R"(test::() { mut x: i64 = 0; x >> 2; })"
TEST_F(IntegratedCodegenTest, Binary_SInt_Shr) {
    File file = File("", "", "", BINARY_SINT_SHR);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := ashr i64 $2, i64 2
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_UINT_SHR R"(test::() { mut x: u64 = 0; x >> 2; })"
TEST_F(IntegratedCodegenTest, Binary_UInt_Shr) {
    File file = File("", "", "", BINARY_UINT_SHR);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := lshr i64 $2, i64 2
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_INT_ADD R"(test::() { mut x: i64 = 0; x + 1; })"
TEST_F(IntegratedCodegenTest, Binary_Int_Add) {
    File file = File("", "", "", BINARY_INT_ADD);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := add i64 $2, i64 1
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_FP_ADD R"(test::() { mut x: f64 = 0.0; x + 1.0; })"
TEST_F(IntegratedCodegenTest, Binary_FP_Add) {
    File file = File("", "", "", BINARY_FP_ADD);
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
    _x := slot f64, align 8

1:
    str f64 0.000000 -> f64* _x, align 8
    $2 := load f64* _x, align 8
    $3 := fadd f64 $2, f64 1.000000
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_INT_SUB R"(test::() { mut x: i64 = 0; x - 1; })"
TEST_F(IntegratedCodegenTest, Binary_Int_Sub) {
    File file = File("", "", "", BINARY_INT_SUB);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := sub i64 $2, i64 1
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_FP_SUB R"(test::() { mut x: f64 = 0.0; x - 1.0; })"
TEST_F(IntegratedCodegenTest, Binary_FP_Sub) {
    File file = File("", "", "", BINARY_FP_SUB);
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
    _x := slot f64, align 8

1:
    str f64 0.000000 -> f64* _x, align 8
    $2 := load f64* _x, align 8
    $3 := fsub f64 $2, f64 1.000000
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_SINT_MUL R"(test::() { mut x: i64 = 0; x * 1; })"
TEST_F(IntegratedCodegenTest, Binary_SInt_Mul) {
    File file = File("", "", "", BINARY_SINT_MUL);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := smul i64 $2, i64 1
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_UINT_MUL R"(test::() { mut x: u64 = 0; x * 1; })"
TEST_F(IntegratedCodegenTest, Binary_UInt_Mul) {
    File file = File("", "", "", BINARY_UINT_MUL);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := umul i64 $2, i64 1
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_FP_MUL R"(test::() { mut x: f64 = 0.0; x * 1.0; })"
TEST_F(IntegratedCodegenTest, Binary_FP_Mul) {
    File file = File("", "", "", BINARY_FP_MUL);
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
    _x := slot f64, align 8

1:
    str f64 0.000000 -> f64* _x, align 8
    $2 := load f64* _x, align 8
    $3 := fmul f64 $2, f64 1.000000
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_SINT_DIV R"(test::() { mut x: i64 = 0; x / 1; })"
TEST_F(IntegratedCodegenTest, Binary_SInt_Div) {
    File file = File("", "", "", BINARY_SINT_DIV);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := sdiv i64 $2, i64 1
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_UINT_DIV R"(test::() { mut x: u64 = 0; x / 1; })"
TEST_F(IntegratedCodegenTest, Binary_UInt_Div) {
    File file = File("", "", "", BINARY_UINT_DIV);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := udiv i64 $2, i64 1
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_FP_DIV R"(test::() { mut x: f64 = 0.0; x / 1.0; })"
TEST_F(IntegratedCodegenTest, Binary_FP_Div) {
    File file = File("", "", "", BINARY_FP_DIV);
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
    _x := slot f64, align 8

1:
    str f64 0.000000 -> f64* _x, align 8
    $2 := load f64* _x, align 8
    $3 := fdiv f64 $2, f64 1.000000
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_SINT_MOD R"(test::() { mut x: i64 = 0; x % 1; })"
TEST_F(IntegratedCodegenTest, Binary_SInt_Mod) {
    File file = File("", "", "", BINARY_SINT_MOD);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := srem i64 $2, i64 1
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_UINT_MOD R"(test::() { mut x: u64 = 0; x % 1; })"
TEST_F(IntegratedCodegenTest, Binary_UInt_Mod) {
    File file = File("", "", "", BINARY_UINT_MOD);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := urem i64 $2, i64 1
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_LOGIC_AND R"(test::() { mut x: i64 = 0; x && 1; })"
TEST_F(IntegratedCodegenTest, Binary_Logical_And_Basic) {
    File file = File("", "", "", BINARY_LOGIC_AND);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := icmp_ne i64 $2, i64 0
    brif i1 $3, #4, #6

4 (1):
    $5 := icmp_ne i64 1, i64 0
    jmp #6

6 (1, 4):
    $7 := phi i1 [ #1, i1 0 ], [ #4, i1 $5 ]
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_LOGIC_OR R"(test::() { mut x: i64 = 0; x || 1; })"
TEST_F(IntegratedCodegenTest, Binary_Logical_Or_Basic) {
    File file = File("", "", "", BINARY_LOGIC_OR);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := icmp_ne i64 $2, i64 0
    brif i1 $3, #6, #4

4 (1):
    $5 := icmp_ne i64 1, i64 0
    jmp #6

6 (1, 4):
    $7 := phi i1 [ #1, i1 1 ], [ #4, i1 $5 ]
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define UNARY_INT_INCREMENT R"(test::() i64 { mut x: i64 = 7; ret x++; })"
TEST_F(IntegratedCodegenTest, Unary_Int_Increment) {
    File file = File("", "", "", UNARY_INT_INCREMENT);
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
    _x := slot i64, align 8

1:
    str i64 7 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := add i64 $2, i64 1
    str i64 $3 -> i64* _x, align 8
    ret i64 $2
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define UNARY_FP_INCREMENT R"(test::() f64 { mut x: f64 = 3.14; ret ++x; })"
TEST_F(IntegratedCodegenTest, Unary_FP_Increment) {
    File file = File("", "", "", UNARY_FP_INCREMENT);
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

test :: () -> f64 {
    _x := slot f64, align 8

1:
    str f64 3.140000 -> f64* _x, align 8
    $2 := load f64* _x, align 8
    $3 := fadd f64 $2, f64 1.000000
    str f64 $3 -> f64* _x, align 8
    ret f64 $3
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define UNARY_INT_DECREMENT R"(test::() i64 { mut x: i64 = 12; ret x--; })"
TEST_F(IntegratedCodegenTest, Unary_Int_Decrement) {
    File file = File("", "", "", UNARY_INT_DECREMENT);
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
    _x := slot i64, align 8

1:
    str i64 12 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := sub i64 $2, i64 1
    str i64 $3 -> i64* _x, align 8
    ret i64 $2
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define UNARY_FP_DECREMENT R"(test::() f64 { mut x: f64 = 13.2; ret --x; })"
TEST_F(IntegratedCodegenTest, Unary_FP_Decrement) {
    File file = File("", "", "", UNARY_FP_DECREMENT);
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

test :: () -> f64 {
    _x := slot f64, align 8

1:
    str f64 13.200000 -> f64* _x, align 8
    $2 := load f64* _x, align 8
    $3 := fsub f64 $2, f64 1.000000
    str f64 $3 -> f64* _x, align 8
    ret f64 $3
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define UNARY_BITWISE_NOT R"(test::() i64 { mut x: i64 = 42; ret ~x; })"
TEST_F(IntegratedCodegenTest, Unary_Bitwise_Not) {
    File file = File("", "", "", UNARY_BITWISE_NOT);
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
    _x := slot i64, align 8

1:
    str i64 42 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := not i64 $2
    ret i64 $3
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define UNARY_LOGIC_NOT R"(test::() bool { mut x: bool = false; ret !x; })"
TEST_F(IntegratedCodegenTest, Unary_Logic_Not) {
    File file = File("", "", "", UNARY_LOGIC_NOT);
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

test :: () -> i1 {
    _x := slot i1, align 1

1:
    str i1 0 -> i1* _x, align 1
    $2 := load i1* _x, align 1
    $3 := xor i1 $2, i1 1
    ret i1 $3
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define UNARY_INT_NEG R"(test::() i64 { mut x: i64 = 42; ret -x; })"
TEST_F(IntegratedCodegenTest, Unary_Int_Negative) {
    File file = File("", "", "", UNARY_INT_NEG);
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
    _x := slot i64, align 8

1:
    str i64 42 -> i64* _x, align 8
    $2 := load i64* _x, align 8
    $3 := neg i64 $2
    ret i64 $3
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define UNARY_FP_NER R"(test::() f64 { mut x: f64 = 3.14; ret -x; })"
TEST_F(IntegratedCodegenTest, Unary_FP_Negative) {
    File file = File("", "", "", UNARY_FP_NER);
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

test :: () -> f64 {
    _x := slot f64, align 8

1:
    str f64 3.140000 -> f64* _x, align 8
    $2 := load f64* _x, align 8
    $3 := fneg f64 $2
    ret f64 $3
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define UNARY_DEREFERENCE_LVALUE R"(test::() { mut x: i64* = nil; *x = 5; })"
TEST_F(IntegratedCodegenTest, Unary_Deref_LValue) {
    File file = File("", "", "", UNARY_DEREFERENCE_LVALUE);
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
    _x := slot i64*, align 8

1:
    $2 := reint void* nil -> i64*
    str i64* $2 -> i64** _x, align 8
    $3 := load i64** _x, align 8
    str i64 5 -> i64* $3, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define UNARY_DEREFERENCE_RVALUE R"(test::() i64 { mut x: i64* = nil; ret *x; })"
TEST_F(IntegratedCodegenTest, Unary_Deref_RValue) {
    File file = File("", "", "", UNARY_DEREFERENCE_RVALUE);
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
    _x := slot i64*, align 8

1:
    $2 := reint void* nil -> i64*
    str i64* $2 -> i64** _x, align 8
    $3 := load i64** _x, align 8
    $4 := load i64* $3, align 8
    ret i64 $4
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define UNARY_ADDRESS_OF R"(test::() { mut x: i64 = 42; mut y: i64* = &x; })"
TEST_F(IntegratedCodegenTest, Unary_Address_Of) {
    File file = File("", "", "", UNARY_ADDRESS_OF);
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
    _y := slot i64*, align 8
    _x := slot i64, align 8

1:
    str i64 42 -> i64* _x, align 8
    str i64* _x -> i64** _y, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define UNARY_PTR_INCREMENT R"(test::() i64* { mut x: i64* = nil; ret x++; })"
TEST_F(IntegratedCodegenTest, Unary_Ptr_Increment) {
    File file = File("", "", "", UNARY_PTR_INCREMENT);
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

test :: () -> i64* {
    _x := slot i64*, align 8

1:
    $2 := reint void* nil -> i64*
    str i64* $2 -> i64** _x, align 8
    $3 := load i64** _x, align 8
    $4 := ap i64*, i64* $3, i64 1
    str i64* $4 -> i64** _x, align 8
    ret i64* $3
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define UNARY_PTR_DECREMENT R"(test::() i64* { mut x: i64* = nil; ret --x; })"
TEST_F(IntegratedCodegenTest, Unary_Ptr_Decrement) {
    File file = File("", "", "", UNARY_PTR_DECREMENT);
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

test :: () -> i64* {
    _x := slot i64*, align 8

1:
    $2 := reint void* nil -> i64*
    str i64* $2 -> i64** _x, align 8
    $3 := load i64** _x, align 8
    $4 := ap i64*, i64* $3, i64 -1
    str i64* $4 -> i64** _x, align 8
    ret i64* $4
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_PTR_ARITH_ADD R"(test::() { mut x: i64* = nil; x = x + 3; })"
TEST_F(IntegratedCodegenTest, Pointer_Arith_Add) {
    File file = File("", "", "", BINARY_PTR_ARITH_ADD);
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
    _x := slot i64*, align 8

1:
    $2 := reint void* nil -> i64*
    str i64* $2 -> i64** _x, align 8
    $3 := load i64** _x, align 8
    $4 := ap i64*, i64* $3, i64 3
    str i64* $4 -> i64** _x, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_PTR_ARITH_ADD_ASSIGN R"(test::() { mut x: i64* = nil; x += 3; })"
TEST_F(IntegratedCodegenTest, Pointer_Arith_Add_Assign) {
    File file = File("", "", "", BINARY_PTR_ARITH_ADD_ASSIGN);
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
    _x := slot i64*, align 8

1:
    $2 := reint void* nil -> i64*
    str i64* $2 -> i64** _x, align 8
    $3 := load i64** _x, align 8
    $4 := ap i64*, i64* $3, i64 3
    str i64* $4 -> i64** _x, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_PTR_ARITH_SUB R"(test::() { mut x: i64* = nil; x = x - 2; })"
TEST_F(IntegratedCodegenTest, Pointer_Arith_Sub) {
    File file = File("", "", "", BINARY_PTR_ARITH_SUB);
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
    _x := slot i64*, align 8

1:
    $2 := reint void* nil -> i64*
    str i64* $2 -> i64** _x, align 8
    $3 := load i64** _x, align 8
    $4 := neg i64 2
    $5 := ap i64*, i64* $3, i64 $4
    str i64* $5 -> i64** _x, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define BINARY_PTR_ARITH_SUB_ASSIGN R"(test::() { mut x: i64* = nil; x -= 2; })"
TEST_F(IntegratedCodegenTest, Pointer_Arith_Sub_Assign) {
    File file = File("", "", "", BINARY_PTR_ARITH_SUB_ASSIGN);
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
    _x := slot i64*, align 8

1:
    $2 := reint void* nil -> i64*
    str i64* $2 -> i64** _x, align 8
    $3 := load i64** _x, align 8
    $4 := neg i64 2
    $5 := ap i64*, i64* $3, i64 $4
    str i64* $5 -> i64** _x, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define ARRAY_INIT_BASIC R"(test::() { mut x: i64[3] = [1, 2, 3]; })"
TEST_F(IntegratedCodegenTest, Array_Init_Basic) {
    File file = File("", "", "", ARRAY_INIT_BASIC);
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
    _x := slot i64[3], align 8

1:
    $2 := ap i64*, i64[3]* _x, i64 0
    str i64 1 -> i64* $2, align 8
    $3 := ap i64*, i64[3]* _x, i64 1
    str i64 2 -> i64* $3, align 8
    $4 := ap i64*, i64[3]* _x, i64 2
    str i64 3 -> i64* $4, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define ARRAY_INIT_WITH R"(test::() { fix x: i64[3] = [ 1, 2, 3 ]; mut y: i64[3] = x; })"
TEST_F(IntegratedCodegenTest, Array_Assign_Basic) {
    File file = File("", "", "", ARRAY_INIT_WITH);
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
    _y := slot i64[3], align 8
    _x := slot i64[3], align 8

1:
    $2 := ap i64*, i64[3]* _x, i64 0
    str i64 1 -> i64* $2, align 8
    $3 := ap i64*, i64[3]* _x, i64 1
    str i64 2 -> i64* $3, align 8
    $4 := ap i64*, i64[3]* _x, i64 2
    str i64 3 -> i64* $4, align 8
    cpy i64 24, i64[3]* _x, align 8 -> i64[3]* _y, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define ARRAY_ASSIGN_TO R"(test::() { mut x: i64[2] = [1, 2]; x = [3, 4]; })"
TEST_F(IntegratedCodegenTest, Array_Assign_To) {
    File file = File("", "", "", ARRAY_ASSIGN_TO);
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
    _x := slot i64[2], align 8

1:
    $2 := ap i64*, i64[2]* _x, i64 0
    str i64 1 -> i64* $2, align 8
    $3 := ap i64*, i64[2]* _x, i64 1
    str i64 2 -> i64* $3, align 8
    $4 := ap i64*, i64[2]* _x, i64 0
    str i64 3 -> i64* $4, align 8
    $5 := ap i64*, i64[2]* _x, i64 1
    str i64 4 -> i64* $5, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define ARRAY_REASSIGN_TO R"(test::() { mut x: i64[2] = [1, 2]; mut y: i64[2] = [3, 4]; y = x; })"
TEST_F(IntegratedCodegenTest, Array_Reassign_To) {
    File file = File("", "", "", ARRAY_REASSIGN_TO);
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
    _y := slot i64[2], align 8
    _x := slot i64[2], align 8

1:
    $2 := ap i64*, i64[2]* _x, i64 0
    str i64 1 -> i64* $2, align 8
    $3 := ap i64*, i64[2]* _x, i64 1
    str i64 2 -> i64* $3, align 8
    $4 := ap i64*, i64[2]* _y, i64 0
    str i64 3 -> i64* $4, align 8
    $5 := ap i64*, i64[2]* _y, i64 1
    str i64 4 -> i64* $5, align 8
    cpy i64 16, i64[2]* _x, align 8 -> i64[2]* _y, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define SUBSCRIPT_ARRAY_BASIC R"(test::() i64 { mut x: i64[2] = [1, 2]; ret x[1]; })"
TEST_F(IntegratedCodegenTest, Subscript_Array_Basic) {
    File file = File("", "", "", SUBSCRIPT_ARRAY_BASIC);
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
    _x := slot i64[2], align 8

1:
    $2 := ap i64*, i64[2]* _x, i64 0
    str i64 1 -> i64* $2, align 8
    $3 := ap i64*, i64[2]* _x, i64 1
    str i64 2 -> i64* $3, align 8
    $4 := ap i64*, i64[2]* _x, i64 1
    $5 := load i64* $4, align 8
    ret i64 $5
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define SUBSCRIPT_PTR_BASIC R"(test::() i64 { mut x: i64* = nil; ret x[3]; })"
TEST_F(IntegratedCodegenTest, Subscript_Ptr_Basic) {
    File file = File("", "", "", SUBSCRIPT_PTR_BASIC);
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
    _x := slot i64*, align 8

1:
    $2 := reint void* nil -> i64*
    str i64* $2 -> i64** _x, align 8
    $3 := load i64** _x, align 8
    $4 := ap i64*, i64* $3, i64 3
    $5 := load i64* $4, align 8
    ret i64 $5
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define FUNCTION_AGGREGATE_RETURN_BASIC R"(test::() i64[3] { ret [1, 2, 3]; })"
TEST_F(IntegratedCodegenTest, Function_Aggregate_Return_Basic) {
    File file = File("", "", "", FUNCTION_AGGREGATE_RETURN_BASIC);
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

test :: (aret i64[3]* %1) -> void {
2:
    $3 := ap i64*, i64[3]* %1, i64 0
    str i64 1 -> i64* $3, align 8
    $4 := ap i64*, i64[3]* %1, i64 1
    str i64 2 -> i64* $4, align 8
    $5 := ap i64*, i64[3]* %1, i64 2
    str i64 3 -> i64* $5, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define FUNCTION_AGGREGATE_RETURN_REF R"(test::() i64[3] { mut x: i64[3] = [1, 2, 3]; ret x; })"
TEST_F(IntegratedCodegenTest, Function_Aggregate_Return_Reference) {
    File file = File("", "", "", FUNCTION_AGGREGATE_RETURN_REF);
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

test :: (aret i64[3]* %1) -> void {
    _x := slot i64[3], align 8

2:
    $3 := ap i64*, i64[3]* _x, i64 0
    str i64 1 -> i64* $3, align 8
    $4 := ap i64*, i64[3]* _x, i64 1
    str i64 2 -> i64* $4, align 8
    $5 := ap i64*, i64[3]* _x, i64 2
    str i64 3 -> i64* $5, align 8
    cpy i64 24, i64[3]* _x, align 8 -> i64[3]* %1, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define FUNCTION_AGGREGATE_ARG_BASIC R"(test::(x: i64[3]) { ret; })"
TEST_F(IntegratedCodegenTest, Function_Aggregate_Arg_Basic) {
    File file = File("", "", "", FUNCTION_AGGREGATE_ARG_BASIC);
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

test :: (aarg i64[3]* %x) -> void {
    _x := slot i64[3], align 8

1:
    cpy i64 24, i64[3]* %x, align 8 -> i64[3]* _x, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define FUNCTION_AGGREGATE_ARG_MANY R"(test::(x: i64[1], y: i64[2], z: i64[3]) { ret; })"
TEST_F(IntegratedCodegenTest, Function_Aggregate_Arg_Many) {
    File file = File("", "", "", FUNCTION_AGGREGATE_ARG_MANY);
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

test :: (aarg i64[1]* %x, aarg i64[2]* %y, aarg i64[3]* %z) -> void {
    _z := slot i64[3], align 8
    _y := slot i64[2], align 8
    _x := slot i64[1], align 8

1:
    cpy i64 8, i64[1]* %x, align 8 -> i64[1]* _x, align 8
    cpy i64 16, i64[2]* %y, align 8 -> i64[2]* _y, align 8
    cpy i64 24, i64[3]* %z, align 8 -> i64[3]* _z, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define CALL_VOID_BASIC R"(foo::() { ret; } test::() { foo(); })"
TEST_F(IntegratedCodegenTest, Call_Void_Basic) {
    File file = File("", "", "", CALL_VOID_BASIC);
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
2:
    call void foo()
    ret
}

foo :: () -> void {
1:
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define CALL_TYPED_BASIC R"(foo::() i64 { ret 42; } test::() { foo(); })"
TEST_F(IntegratedCodegenTest, Call_Typed_Basic) {
    File file = File("", "", "", CALL_TYPED_BASIC);
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
2:
    $3 := call i64 foo()
    ret
}

foo :: () -> i64 {
1:
    ret i64 42
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define CALL_ARGS_BASIC R"(foo::(x: i64) i64 { ret x; } test::() { foo(42); })"
TEST_F(IntegratedCodegenTest, Call_Args_Basic) {
    File file = File("", "", "", CALL_ARGS_BASIC);
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
3:
    $4 := call i64 foo(i64 42)
    ret
}

foo :: (i64 %x) -> i64 {
    _x := slot i64, align 8

1:
    str i64 %x -> i64* _x, align 8
    $2 := load i64* _x, align 8
    ret i64 $2
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define CALL_ARGS_RVALUE_BASIC R"(foo::(x: i64) i64 { ret x; } test::() { mut x: i64 = 42; foo(x); })"
TEST_F(IntegratedCodegenTest, Call_Args_RValue_Basic) {
    File file = File("", "", "", CALL_ARGS_RVALUE_BASIC);
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
    _x := slot i64, align 8

3:
    str i64 42 -> i64* _x, align 8
    $4 := load i64* _x, align 8
    $5 := call i64 foo(i64 $4)
    ret
}

foo :: (i64 %x) -> i64 {
    _x := slot i64, align 8

1:
    str i64 %x -> i64* _x, align 8
    $2 := load i64* _x, align 8
    ret i64 $2
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define CALL_AGGREGATE_ARRAY_RETURN_BASIC R"(foo::() i64[3] { ret [1, 2, 3]; } test::() { mut x: i64[3] = foo(); })"
TEST_F(IntegratedCodegenTest, Call_Aggregate_Array_Return_Basic) {
    File file = File("", "", "", CALL_AGGREGATE_ARRAY_RETURN_BASIC);
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
    _x := slot i64[3], align 8

6:
    call void foo(i64[3]* _x)
    ret
}

foo :: (aret i64[3]* %1) -> void {
2:
    $3 := ap i64*, i64[3]* %1, i64 0
    str i64 1 -> i64* $3, align 8
    $4 := ap i64*, i64[3]* %1, i64 1
    str i64 2 -> i64* $4, align 8
    $5 := ap i64*, i64[3]* %1, i64 2
    str i64 3 -> i64* $5, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define CALL_AGGREGATE_ARRAY_ARG_REF_BASIC R"(foo::(x: i64[3]) i64 { ret x[1]; } test::() { mut x: i64[3] = [1, 2, 3]; foo(x); })"
TEST_F(IntegratedCodegenTest, Call_Aggregate_Array_Arg_Ref_Basic) {
    File file = File("", "", "", CALL_AGGREGATE_ARRAY_ARG_REF_BASIC);
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
    _8 := slot i64[3], align 8
    _x := slot i64[3], align 8

4:
    $5 := ap i64*, i64[3]* _x, i64 0
    str i64 1 -> i64* $5, align 8
    $6 := ap i64*, i64[3]* _x, i64 1
    str i64 2 -> i64* $6, align 8
    $7 := ap i64*, i64[3]* _x, i64 2
    str i64 3 -> i64* $7, align 8
    cpy i64 24, i64[3]* _x, align 8 -> i64[3]* _8, align 8
    $9 := call i64 foo(i64[3]* _8)
    ret
}

foo :: (aarg i64[3]* %x) -> i64 {
    _x := slot i64[3], align 8

1:
    cpy i64 24, i64[3]* %x, align 8 -> i64[3]* _x, align 8
    $2 := ap i64*, i64[3]* _x, i64 1
    $3 := load i64* $2, align 8
    ret i64 $3
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define CALL_AGGREGATE_ARRAY_ARG_INIT_BASIC R"(foo::(x: i64[3]) i64 { ret x[1]; } test::() { mut x: i64 = foo([1, 2, 3]); })"
TEST_F(IntegratedCodegenTest, Call_Aggregate_Array_Arg_Init_Basic) {
    File file = File("", "", "", CALL_AGGREGATE_ARRAY_ARG_INIT_BASIC);
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
    _5 := slot i64[3], align 8
    _x := slot i64, align 8

4:
    $6 := ap i64*, i64[3]* _5, i64 0
    str i64 1 -> i64* $6, align 8
    $7 := ap i64*, i64[3]* _5, i64 1
    str i64 2 -> i64* $7, align 8
    $8 := ap i64*, i64[3]* _5, i64 2
    str i64 3 -> i64* $8, align 8
    $9 := call i64 foo(i64[3]* _5)
    str i64 $9 -> i64* _x, align 8
    ret
}

foo :: (aarg i64[3]* %x) -> i64 {
    _x := slot i64[3], align 8

1:
    cpy i64 24, i64[3]* %x, align 8 -> i64[3]* _x, align 8
    $2 := ap i64*, i64[3]* _x, i64 1
    $3 := load i64* $2, align 8
    ret i64 $3
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}
#define CALL_AGGREGATE_ARRAY_RETURN_AGGREGATE_ARRAY_ARG_BASIC R"(foo::(x: i64[3]) i64[3] { x[1] = 42; ret x; } test::() { mut x: i64[3] = [1, 2, 3]; mut y: i64[3] = foo(x); })"
TEST_F(IntegratedCodegenTest, Call_Aggregate_Array_Return_And_Aggregate_Array_Arg_Basic) {
    File file = File("", "", "", CALL_AGGREGATE_ARRAY_RETURN_AGGREGATE_ARRAY_ARG_BASIC);
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
    _8 := slot i64[3], align 8
    _y := slot i64[3], align 8
    _x := slot i64[3], align 8

4:
    $5 := ap i64*, i64[3]* _x, i64 0
    str i64 1 -> i64* $5, align 8
    $6 := ap i64*, i64[3]* _x, i64 1
    str i64 2 -> i64* $6, align 8
    $7 := ap i64*, i64[3]* _x, i64 2
    str i64 3 -> i64* $7, align 8
    cpy i64 24, i64[3]* _x, align 8 -> i64[3]* _8, align 8
    call void foo(i64[3]* _y, i64[3]* _8)
    ret
}

foo :: (aret i64[3]* %1, aarg i64[3]* %x) -> void {
    _x := slot i64[3], align 8

2:
    cpy i64 24, i64[3]* %x, align 8 -> i64[3]* _x, align 8
    $3 := ap i64*, i64[3]* _x, i64 1
    str i64 42 -> i64* $3, align 8
    cpy i64 24, i64[3]* _x, align 8 -> i64[3]* %1, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define AGGREGATE_RETURN__RETURN_ARRAY_INIT R"(test::() i64[2] { ret [1, 2]; })"
TEST_F(IntegratedCodegenTest, AArg_Return_Array_Init) {
    File file = File("", "", "", AGGREGATE_RETURN__RETURN_ARRAY_INIT);
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

test :: (aret i64[2]* %1) -> void {
2:
    $3 := ap i64*, i64[2]* %1, i64 0
    str i64 1 -> i64* $3, align 8
    $4 := ap i64*, i64[2]* %1, i64 1
    str i64 2 -> i64* $4, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define NESTED_ARRAY_INIT R"(test::() { mut x: i64[2][3] = [[1, 2], [3, 4], [5, 6]]; })"
TEST_F(IntegratedCodegenTest, Nested_Array_Init) {
    File file = File("", "", "", NESTED_ARRAY_INIT);
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
    _x := slot i64[2][3], align 8

1:
    $2 := ap i64[2]*, i64[2][3]* _x, i64 0
    $3 := ap i64*, i64[2]* $2, i64 0
    str i64 1 -> i64* $3, align 8
    $4 := ap i64*, i64[2]* $2, i64 1
    str i64 2 -> i64* $4, align 8
    $5 := ap i64[2]*, i64[2][3]* _x, i64 1
    $6 := ap i64*, i64[2]* $5, i64 0
    str i64 3 -> i64* $6, align 8
    $7 := ap i64*, i64[2]* $5, i64 1
    str i64 4 -> i64* $7, align 8
    $8 := ap i64[2]*, i64[2][3]* _x, i64 2
    $9 := ap i64*, i64[2]* $8, i64 0
    str i64 5 -> i64* $9, align 8
    $10 := ap i64*, i64[2]* $8, i64 1
    str i64 6 -> i64* $10, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define SPEC_ENUM_BASIC R"(Colors :: i64 { Red } test :: () { mut x: Colors = Colors::Red; })"
TEST_F(IntegratedCodegenTest, Spec_Enum_Basic) {
    File file = File("", "", "", SPEC_ENUM_BASIC);
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
    _x := slot i64, align 8

1:
    str i64 0 -> i64* _x, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define STRUCT_BASIC R"(box :: { x: i64, y: f64 })"
TEST_F(IntegratedCodegenTest, Struct_Basic) {
    File file = File("", "", "", STRUCT_BASIC);
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

box :: type { i64, f64 }
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define STRUCT_METHOD_BASIC R"(box :: { x: i64, y: bool, foo :: (self: box*) { ret; } })"
TEST_F(IntegratedCodegenTest, Struct_Method_Basic) {
    File file = File("", "", "", STRUCT_METHOD_BASIC);
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

box :: type { i64, i1 }

box.foo :: (box* %self) -> void {
    _self := slot box*, align 8

1:
    str box* %self -> box** _self, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define STRUCT_METHOD_FIELD_REF R"(box :: { x: i64, foo :: (self: box*) i64 { ret x; } })"
TEST_F(IntegratedCodegenTest, Struct_Method_Field_Ref) {
    File file = File("", "", "", STRUCT_METHOD_FIELD_REF);
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

box :: type { i64 }

box.foo :: (box* %self) -> i64 {
    _self := slot box*, align 8

1:
    str box* %self -> box** _self, align 8
    $2 := load box** _self, align 8
    $3 := ap i64*, box* $2, i64 0
    $4 := load i64* $3, align 8
    ret i64 $4
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define STRUCT_ACCESS_VAL_BASIC R"(box :: { x: i64, y: i32 } test :: () i32 { mut a: box; ret a.x; })"
TEST_F(IntegratedCodegenTest, Struct_Access_Val_Basic) {
    File file = File("", "", "", STRUCT_ACCESS_VAL_BASIC);
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

box :: type { i64, i32 }

test :: () -> i32 {
    _a := slot box, align 8

1:
    $2 := ap i64*, box* _a, i64 0
    $3 := load i64* $2, align 8
    $4 := trunc i64 $3 -> i32
    ret i32 $4
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define STRUCT_ACCESS_PTR_BASIC R"(box :: { x: i64, y: i32 } test :: () i32 { mut a: box* = nil; ret a.y; } )"
TEST_F(IntegratedCodegenTest, Struct_Access_Ptr_Basic) {
    File file = File("", "", "", STRUCT_ACCESS_PTR_BASIC);
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

box :: type { i64, i32 }

test :: () -> i32 {
    _a := slot box*, align 8

1:
    $2 := reint void* nil -> box*
    str box* $2 -> box** _a, align 8
    $3 := load box** _a, align 8
    $4 := ap i32*, box* $3, i64 1
    $5 := load i32* $4, align 4
    ret i32 $5
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define SPEC_STRUCT_METHOD_BASIC R"(Color :: { x: i64, foo :: (self: Color*) i64 { ret x; } } test :: () { mut x: Color* = nil; mut y: i64 = Color::foo(x); })"
TEST_F(IntegratedCodegenTest, Spec_Struct_Method_Basic) {
    File file = File("", "", "", SPEC_STRUCT_METHOD_BASIC);
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

Color :: type { i64 }

test :: () -> void {
    _y := slot i64, align 8
    _x := slot Color*, align 8

5:
    $6 := reint void* nil -> Color*
    str Color* $6 -> Color** _x, align 8
    $7 := load Color** _x, align 8
    $8 := call i64 Color.foo(Color* $7)
    str i64 $8 -> i64* _y, align 8
    ret
}

Color.foo :: (Color* %self) -> i64 {
    _self := slot Color*, align 8

1:
    str Color* %self -> Color** _self, align 8
    $2 := load Color** _self, align 8
    $3 := ap i64*, Color* $2, i64 0
    $4 := load i64* $3, align 8
    ret i64 $4
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define SPEC_STRUCT_ASSOCIATED_BASIC R"(Color :: { x: i64, $associated foo :: () i64 { ret 42; } } test :: () { mut x: i64 = Color::foo(); })"
TEST_F(IntegratedCodegenTest, Spec_Struct_Associated_Basic) {
    File file = File("", "", "", SPEC_STRUCT_ASSOCIATED_BASIC);
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

Color :: type { i64 }

test :: () -> void {
    _x := slot i64, align 8

2:
    $3 := call i64 Color.foo()
    str i64 $3 -> i64* _x, align 8
    ret
}

Color.foo :: () -> i64 {
1:
    ret i64 42
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define METHOD_CALL_BASIC R"(box :: { x: i64, foo :: (self: box*) i64 { ret 42; } } test :: () { mut x: box; x.foo(); })"
TEST_F(IntegratedCodegenTest, Method_Call_Basic) {
    File file = File("", "", "", METHOD_CALL_BASIC);
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

box :: type { i64 }

test :: () -> void {
    _x := slot box, align 8

2:
    $3 := call i64 box.foo(box* _x)
    ret
}

box.foo :: (box* %self) -> i64 {
    _self := slot box*, align 8

1:
    str box* %self -> box** _self, align 8
    ret i64 42
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define METHOD_CALL_ARGS_BASIC R"(box :: { x: i64, foo :: (self: box*, y: i64) i64 { ret y; } } test :: () { mut x: box; x.foo(42); })"
TEST_F(IntegratedCodegenTest, Method_Call_Args_Basic) {
    File file = File("", "", "", METHOD_CALL_ARGS_BASIC);
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

box :: type { i64 }

test :: () -> void {
    _x := slot box, align 8

3:
    $4 := call i64 box.foo(box* _x, i64 42)
    ret
}

box.foo :: (box* %self, i64 %y) -> i64 {
    _y := slot i64, align 8
    _self := slot box*, align 8

1:
    str box* %self -> box** _self, align 8
    str i64 %y -> i64* _y, align 8
    $2 := load i64* _y, align 8
    ret i64 $2
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define STRUCT_INIT_BASIC R"(box :: { x: i64, y: i32 } test :: () { mut x: box = box { x: 1, y: 2 }; })"
TEST_F(IntegratedCodegenTest, Struct_Init_Basic) {
    File file = File("", "", "", STRUCT_INIT_BASIC);
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

box :: type { i64, i32 }

test :: () -> void {
    _x := slot box, align 8

1:
    $2 := ap i64*, box* _x, i64 0
    str i64 1 -> i64* $2, align 8
    $3 := ap i32*, box* _x, i64 1
    $4 := trunc i64 2 -> i32
    str i32 $4 -> i32* $3, align 4
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define STRUCT_INIT_PARTIAL R"(box :: { x: i64, y: i8, z: f32 } test :: () { mut x: box = box { x: 1, z: 2.14 }; })"
TEST_F(IntegratedCodegenTest, Struct_Init_Partial) {
    File file = File("", "", "", STRUCT_INIT_PARTIAL);
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

box :: type { i64, i8, f32 }

test :: () -> void {
    _x := slot box, align 8

1:
    $2 := ap i64*, box* _x, i64 0
    str i64 1 -> i64* $2, align 8
    $3 := ap f32*, box* _x, i64 2
    $4 := ftrunc f64 2.140000 -> f32
    str f32 $4 -> f32* $3, align 4
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define NESTED_STRUCT_INIT R"(sa :: { x: i64, y: i8 } sb :: { a: sa, b: f32 } test :: () { mut x: sb = sb { a: sa { x: 0, y: 1 }, b: 3.14 }; })"
TEST_F(IntegratedCodegenTest, Nested_Struct_Init) {
    File file = File("", "", "", NESTED_STRUCT_INIT);
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

sb :: type { sa, f32 }
sa :: type { i64, i8 }

test :: () -> void {
    _x := slot sb, align 8

1:
    $2 := ap sa*, sb* _x, i64 0
    $3 := ap i64*, sa* $2, i64 0
    str i64 0 -> i64* $3, align 8
    $4 := ap i8*, sa* $2, i64 1
    $5 := trunc i64 1 -> i8
    str i8 $5 -> i8* $4, align 1
    $6 := ap f32*, sb* _x, i64 1
    $7 := ftrunc f64 3.140000 -> f32
    str f32 $7 -> f32* $6, align 4
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define ARRAY_NESTED_STRUCT_INIT R"(box :: { x: i64, y: bool } test :: () { mut x: box[2] = [ box { x: 1, y: true }, box { x: 0, y: false } ]; })"
TEST_F(IntegratedCodegenTest, Array_Nested_Struct_Init) {
    File file = File("", "", "", ARRAY_NESTED_STRUCT_INIT);
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

box :: type { i64, i1 }

test :: () -> void {
    _x := slot box[2], align 8

1:
    $2 := ap box*, box[2]* _x, i64 0
    $3 := ap i64*, box* $2, i64 0
    str i64 1 -> i64* $3, align 8
    $4 := ap i1*, box* $2, i64 1
    str i1 1 -> i1* $4, align 1
    $5 := ap box*, box[2]* _x, i64 1
    $6 := ap i64*, box* $5, i64 0
    str i64 0 -> i64* $6, align 8
    $7 := ap i1*, box* $5, i64 1
    str i1 0 -> i1* $7, align 1
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define STRUCT_NESTED_ARRAY_INIT R"(box :: { x: i32[3], y: i32 } test :: () { mut x: box = box { x: [1, 2, 3], y: 4 }; })"
TEST_F(IntegratedCodegenTest, Struct_Nested_Array_Init) {
    File file = File("", "", "", STRUCT_NESTED_ARRAY_INIT);
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

box :: type { i32[3], i32 }

test :: () -> void {
    _x := slot box, align 4

1:
    $2 := ap i32[3]*, box* _x, i64 0
    $3 := ap i32*, i32[3]* $2, i64 0
    $4 := trunc i64 1 -> i32
    str i32 $4 -> i32* $3, align 4
    $5 := ap i32*, i32[3]* $2, i64 1
    $6 := trunc i64 2 -> i32
    str i32 $6 -> i32* $5, align 4
    $7 := ap i32*, i32[3]* $2, i64 2
    $8 := trunc i64 3 -> i32
    str i32 $8 -> i32* $7, align 4
    $9 := ap i32*, box* _x, i64 1
    $10 := trunc i64 4 -> i32
    str i32 $10 -> i32* $9, align 4
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define CALL_AGGREGATE_STRUCT_RETURN_BASIC R"(box :: { x: i64, y: i32 } foo::() box { ret box { x: 1, y: 2 }; } test :: () { mut x: box = foo(); })"
TEST_F(IntegratedCodegenTest, Call_Aggregate_Struct_Return_Basic) {
    File file = File("", "", "", CALL_AGGREGATE_STRUCT_RETURN_BASIC);
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

box :: type { i64, i32 }

test :: () -> void {
    _x := slot box, align 8

6:
    call void foo(box* _x)
    ret
}

foo :: (aret box* %1) -> void {
2:
    $3 := ap i64*, box* %1, i64 0
    str i64 1 -> i64* $3, align 8
    $4 := ap i32*, box* %1, i64 1
    $5 := trunc i64 2 -> i32
    str i32 $5 -> i32* $4, align 4
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define CALL_AGGREGATE_STRUCT_ARG_REF_BASIC R"(box :: { x: i64, y: i32 } foo :: (x: box) i64 { ret x.x; } test :: () { mut x: box = box { x: 1, y: 2 }; foo(x); })"
TEST_F(IntegratedCodegenTest, Call_Aggregate_Struct_Arg_Ref_Basic) {
    File file = File("", "", "", CALL_AGGREGATE_STRUCT_ARG_REF_BASIC);
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

box :: type { i64, i32 }

test :: () -> void {
    _8 := slot box, align 8
    _x := slot box, align 8

4:
    $5 := ap i64*, box* _x, i64 0
    str i64 1 -> i64* $5, align 8
    $6 := ap i32*, box* _x, i64 1
    $7 := trunc i64 2 -> i32
    str i32 $7 -> i32* $6, align 4
    cpy i64 16, box* _x, align 8 -> box* _8, align 8
    $9 := call i64 foo(box* _8)
    ret
}

foo :: (aarg box* %x) -> i64 {
    _x := slot box, align 8

1:
    cpy i64 16, box* %x, align 8 -> box* _x, align 8
    $2 := ap i64*, box* _x, i64 0
    $3 := load i64* $2, align 8
    ret i64 $3
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define CALL_AGGREGATE_STRUCT_ARG_INIT_BASIC R"(box :: { x: i64, y: i32 } foo :: (x: box) i32 { ret x.y; } test :: () { mut x: i32 = foo(box { x: 1, y: 2 }); })"
TEST_F(IntegratedCodegenTest, Call_Aggregate_Struct_Arg_Init_Basic) {
    File file = File("", "", "", CALL_AGGREGATE_STRUCT_ARG_INIT_BASIC);
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

box :: type { i64, i32 }

test :: () -> void {
    _5 := slot box, align 8
    _x := slot i32, align 4

4:
    $6 := ap i64*, box* _5, i64 0
    str i64 1 -> i64* $6, align 8
    $7 := ap i32*, box* _5, i64 1
    $8 := trunc i64 2 -> i32
    str i32 $8 -> i32* $7, align 4
    $9 := call i32 foo(box* _5)
    str i32 $9 -> i32* _x, align 4
    ret
}

foo :: (aarg box* %x) -> i32 {
    _x := slot box, align 8

1:
    cpy i64 16, box* %x, align 8 -> box* _x, align 8
    $2 := ap i32*, box* _x, i64 1
    $3 := load i32* $2, align 4
    ret i32 $3
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define CALL_AGGREGATE_STRUCT_RETURN_AGGREGATE_STRUCT_ARG_BASIC R"(box :: { x: i64, y: bool } foo :: (x: box) box { x.y = true; ret x; } test :: () { mut x: box = box { x: 1, y: false }; mut y: box = foo(x); })"
TEST_F(IntegratedCodegenTest, Call_Aggregate_Struct_Return_Aggregate_Struct_Arg_Basic) {
    File file = File("", "", "", CALL_AGGREGATE_STRUCT_RETURN_AGGREGATE_STRUCT_ARG_BASIC);
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

box :: type { i64, i1 }

test :: () -> void {
    _7 := slot box, align 8
    _y := slot box, align 8
    _x := slot box, align 8

4:
    $5 := ap i64*, box* _x, i64 0
    str i64 1 -> i64* $5, align 8
    $6 := ap i1*, box* _x, i64 1
    str i1 0 -> i1* $6, align 1
    cpy i64 16, box* _x, align 8 -> box* _7, align 8
    call void foo(box* _y, box* _7)
    ret
}

foo :: (aret box* %1, aarg box* %x) -> void {
    _x := slot box, align 8

2:
    cpy i64 16, box* %x, align 8 -> box* _x, align 8
    $3 := ap i1*, box* _x, i64 1
    str i1 1 -> i1* $3, align 1
    cpy i64 16, box* _x, align 8 -> box* %1, align 8
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define AGGREGATE_RETURN__RETURN_STRUCT_INIT R"(box :: { x: i64, y: i8 } test :: () box { ret box { x: 1, y: 2 }; })"
TEST_F(IntegratedCodegenTest, Aggregate_Return__Return_Struct_Init) {
    File file = File("", "", "", AGGREGATE_RETURN__RETURN_STRUCT_INIT);
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

box :: type { i64, i8 }

test :: (aret box* %1) -> void {
2:
    $3 := ap i64*, box* %1, i64 0
    str i64 1 -> i64* $3, align 8
    $4 := ap i8*, box* %1, i64 1
    $5 := trunc i64 2 -> i8
    str i8 $5 -> i8* $4, align 1
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define METHOD_CALL_AGGREGATE_STRUCT_RETURN_BASIC R"(box :: { x: i64, y: f32, foo :: (self: box*) box { ret box { x: 1, y: 3.1 }; } } test :: () { mut x: box; mut y: box = x.foo(); })"
TEST_F(IntegratedCodegenTest, Method_Call_Aggregate_Struct_Return_Basic) {
    File file = File("", "", "", METHOD_CALL_AGGREGATE_STRUCT_RETURN_BASIC);
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

box :: type { i64, f32 }

test :: () -> void {
    _y := slot box, align 8
    _x := slot box, align 8

6:
    call void box.foo(box* _y, box* _x)
    ret
}

box.foo :: (aret box* %1, box* %self) -> void {
    _self := slot box*, align 8

2:
    str box* %self -> box** _self, align 8
    $3 := ap i64*, box* %1, i64 0
    str i64 1 -> i64* $3, align 8
    $4 := ap f32*, box* %1, i64 1
    $5 := ftrunc f64 3.100000 -> f32
    str f32 $5 -> f32* $4, align 4
    ret
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

#define METHOD_CALL_AGGREGATE_STRUCT_ARG_REF_BASIC R"(box :: { x: i64, y: bool, foo :: (self: box*) i64 { ret self.x; } } test :: () { mut x: box = box { x: 1, y: false }; x.foo(); })"
TEST_F(IntegratedCodegenTest, Method_Call_Aggregate_Struct_Arg_Ref_Basic) {
    File file = File("", "", "", METHOD_CALL_AGGREGATE_STRUCT_ARG_REF_BASIC);
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

box :: type { i64, i1 }

test :: () -> void {
    _x := slot box, align 8

5:
    $6 := ap i64*, box* _x, i64 0
    str i64 1 -> i64* $6, align 8
    $7 := ap i1*, box* _x, i64 1
    str i1 0 -> i1* $7, align 1
    $8 := call i64 box.foo(box* _x)
    ret
}

box.foo :: (box* %self) -> i64 {
    _self := slot box*, align 8

1:
    str box* %self -> box** _self, align 8
    $2 := load box** _self, align 8
    $3 := ap i64*, box* $2, i64 0
    $4 := load i64* $3, align 8
    ret i64 $4
}
)";
    EXPECT_EQ(ss.str(), expected);

    delete seg;
    delete unit;
}

} // namespace test

} // namespace meddle
