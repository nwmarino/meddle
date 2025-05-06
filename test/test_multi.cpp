#include "../compiler/parser/parser.h"
#include "../compiler/lexer/lexer.h"
#include "../compiler/tree/decl.h"
#include "../compiler/tree/unitman.h"

#include "gtest/gtest.h"
#include <fstream>
#include <gtest/gtest.h>

namespace meddle {

namespace test {

class MultiUnitTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

#define UNNAMED_BASIC_1 R"($public bar :: () i64 { ret 42; })"
#define UNNAMED_BASIC_2 R"(use "bar"; foo :: () i64 { ret bar(); })"
TEST_F(MultiUnitTest, Two_Files_Unnamed_Basic) {
    std::ofstream F1("bar.mdl");
    F1 << UNNAMED_BASIC_1;
    F1.close();

    std::ofstream F2("foo.mdl");
    F2 << UNNAMED_BASIC_2;
    F2.close();

    std::vector<File> files = { parseInputFile("bar.mdl"), parseInputFile("foo.mdl") };
    UnitManager units;
    
    for (auto &file : files) {
        Lexer lexer = Lexer(file);
        TokenStream stream = lexer.unwrap();
        Parser parser = Parser(file, stream);
        units.addUnit(parser.get());
    }

    EXPECT_NO_FATAL_FAILURE(units.drive(Options()));

    std::remove("bar.mdl");
    std::remove("foo.mdl");
}

#define UNNAMED_BASIC_BAD_1 R"(bar :: () i64 { ret 42; })"
#define UNNAMED_BASIC_BAD_2 R"(use "bar"; foo :: () i64 { ret bar(); })"
TEST_F(MultiUnitTest, Two_Files_Unnamed_Basic_Bad) {
    std::ofstream F1("bar.mdl");
    F1 << UNNAMED_BASIC_BAD_1;
    F1.close();

    std::ofstream F2("foo.mdl");
    F2 << UNNAMED_BASIC_BAD_2;
    F2.close();

    std::vector<File> files = { parseInputFile("bar.mdl"), parseInputFile("foo.mdl") };
    UnitManager units;
    
    for (auto &file : files) {
        Lexer lexer = Lexer(file);
        TokenStream stream = lexer.unwrap();
        Parser parser = Parser(file, stream);
        units.addUnit(parser.get());
    }

    EXPECT_EXIT(units.drive(Options()), ::testing::ExitedWithCode(1), ".*");

    std::remove("bar.mdl");
    std::remove("foo.mdl");
}

#define LISTED_BASIC_1 R"($public foo :: () i64 { ret 0; } $public bar :: () i64 { ret 1; })"
#define LISTED_BASIC_2 R"(use { bar } = "other"; zoo :: () i64 { ret bar(); })"
TEST_F(MultiUnitTest, Two_Files_Listed_Basic) {
    std::ofstream F1("other.mdl");
    F1 << LISTED_BASIC_1;
    F1.close();

    std::ofstream F2("lib.mdl");
    F2 << LISTED_BASIC_2;
    F2.close();

    std::vector<File> files = { parseInputFile("lib.mdl"), parseInputFile("other.mdl") };
    UnitManager units;
    
    for (auto &file : files) {
        Lexer lexer = Lexer(file);
        TokenStream stream = lexer.unwrap();
        Parser parser = Parser(file, stream);
        units.addUnit(parser.get());
    }

    EXPECT_NO_FATAL_FAILURE(units.drive(Options()));

    std::remove("lib.mdl");
    std::remove("other.mdl");
}

#define LISTED_BASIC_BAD_1 R"($public foo :: () i64 { ret 0; } $public bar :: () i64 { ret 1; })"
#define LISTED_BASIC_BAD_2 R"(use { foo } = "other"; zoo :: () i64 { ret bar(); })"
TEST_F(MultiUnitTest, Two_Files_Listed_Basic_Bad) {
    std::ofstream F1("other.mdl");
    F1 << LISTED_BASIC_BAD_1;
    F1.close();

    std::ofstream F2("lib.mdl");
    F2 << LISTED_BASIC_BAD_2;
    F2.close();

    std::vector<File> files = { parseInputFile("lib.mdl"), parseInputFile("other.mdl") };
    UnitManager units;
    
    for (auto &file : files) {
        Lexer lexer = Lexer(file);
        TokenStream stream = lexer.unwrap();
        Parser parser = Parser(file, stream);
        units.addUnit(parser.get());
    }

    EXPECT_EXIT(units.drive(Options()), ::testing::ExitedWithCode(1), ".*");

    std::remove("lib.mdl");
    std::remove("other.mdl");
}

} // namespace test

} // namespace meddle
