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

#define NESTED_USE_1 R"($public bar :: () i64 { ret 0; })"
#define NESTED_USE_2 R"(use "samples/bar.mdl"; foo :: () i64 { ret bar(); })"
TEST_F(MultiUnitTest, Two_Files_Nested_Use) {
    std::ofstream F1("samples/bar.mdl");
    F1 << NESTED_USE_1;
    F1.close();

    std::ofstream F2("lib.mdl");
    F2 << NESTED_USE_2;
    F2.close();

    std::vector<File> files = { parseInputFile("lib.mdl"), parseInputFile("samples/bar.mdl") };
    UnitManager units;
    
    for (auto &file : files) {
        Lexer lexer = Lexer(file);
        TokenStream stream = lexer.unwrap();
        Parser parser = Parser(file, stream);
        units.addUnit(parser.get());
    }

    EXPECT_NO_FATAL_FAILURE(units.drive(Options()));

    std::remove("lib.mdl");
    std::remove("samples/bar.mdl");
}

#define NAMED_USE_1 R"($public bar :: () i64 { ret 0; })"
#define NAMED_USE_2 R"(use Bar = "bar"; foo :: () i64 { ret Bar::bar(); })"
TEST_F(MultiUnitTest, Two_Files_Named_Use) {
    std::ofstream F1("bar.mdl");
    F1 << NAMED_USE_1;
    F1.close();

    std::ofstream F2("lib.mdl");
    F2 << NAMED_USE_2;
    F2.close();

    std::vector<File> files = { parseInputFile("lib.mdl"), parseInputFile("bar.mdl") };
    UnitManager units;
    
    for (auto &file : files) {
        Lexer lexer = Lexer(file);
        TokenStream stream = lexer.unwrap();
        Parser parser = Parser(file, stream);
        units.addUnit(parser.get());
    }

    EXPECT_NO_FATAL_FAILURE(units.drive(Options()));

    std::remove("lib.mdl");
    std::remove("bar.mdl");
}

#define NAMED_USE_BAD_1 R"(bar :: () i64 { ret 0; })"
#define NAMED_USE_BAD_2 R"(use Bar = "bar"; foo :: () i64 { ret Bar::bar(); })"
TEST_F(MultiUnitTest, Two_Files_Named_Use_Bad) {
    std::ofstream F1("bar.mdl");
    F1 << NAMED_USE_BAD_1;
    F1.close();

    std::ofstream F2("lib.mdl");
    F2 << NAMED_USE_BAD_2;
    F2.close();

    std::vector<File> files = { parseInputFile("lib.mdl"), parseInputFile("bar.mdl") };
    UnitManager units;
    
    for (auto &file : files) {
        Lexer lexer = Lexer(file);
        TokenStream stream = lexer.unwrap();
        Parser parser = Parser(file, stream);
        units.addUnit(parser.get());
    }

    EXPECT_EXIT(units.drive(Options()), ::testing::ExitedWithCode(1), ".*");

    std::remove("lib.mdl");
    std::remove("bar.mdl");
}

#define UNNAMED_USE_TYPE_1 R"($public box :: { x: i64, y: i32 })"
#define UNNAMED_USE_TYPE_2 R"(use "box"; test :: () box { ret box { x: 1, y: 2 }; })"
TEST_F(MultiUnitTest, Two_Files_Unnamed_Use_Type) {
    std::ofstream F1("box.mdl");
    F1 << UNNAMED_USE_TYPE_1;
    F1.close();

    std::ofstream F2("lib.mdl");
    F2 << UNNAMED_USE_TYPE_2;
    F2.close();

    std::vector<File> files = { parseInputFile("lib.mdl"), parseInputFile("box.mdl") };
    UnitManager units;
    
    for (auto &file : files) {
        Lexer lexer = Lexer(file);
        TokenStream stream = lexer.unwrap();
        Parser parser = Parser(file, stream);
        units.addUnit(parser.get());
    }

    EXPECT_NO_FATAL_FAILURE(units.drive(Options()));

    std::remove("lib.mdl");
    std::remove("box.mdl");
}

#define UNNAMED_USE_TYPE_BAD_1 R"(box :: { x: i64, y: i32 })"
#define UNNAMED_USE_TYPE_BAD_2 R"(use "box"; test :: () box { ret box { x: 1, y: 2 }; })"
TEST_F(MultiUnitTest, Two_Files_Unnamed_Use_Type_Bad) {
    std::ofstream F1("box.mdl");
    F1 << UNNAMED_USE_TYPE_BAD_1;
    F1.close();

    std::ofstream F2("lib.mdl");
    F2 << UNNAMED_USE_TYPE_BAD_2;
    F2.close();

    std::vector<File> files = { parseInputFile("lib.mdl"), parseInputFile("box.mdl") };
    UnitManager units;
    
    for (auto &file : files) {
        Lexer lexer = Lexer(file);
        TokenStream stream = lexer.unwrap();
        Parser parser = Parser(file, stream);
        units.addUnit(parser.get());
    }

    EXPECT_EXIT(units.drive(Options()), ::testing::ExitedWithCode(1), ".*");

    std::remove("lib.mdl");
    std::remove("box.mdl");
}

#define LISTED_USE_TYPE_1 R"($public box :: { x: i64, y: i32 })"
#define LISTED_USE_TYPE_2 R"(use { box } = "box"; test :: () box { ret box { x: 1, y: 2 }; })"
TEST_F(MultiUnitTest, Two_Files_Listed_Use_Type) {
    std::ofstream F1("box.mdl");
    F1 << LISTED_USE_TYPE_1;
    F1.close();

    std::ofstream F2("lib.mdl");
    F2 << LISTED_USE_TYPE_2;
    F2.close();

    std::vector<File> files = { parseInputFile("lib.mdl"), parseInputFile("box.mdl") };
    UnitManager units;
    
    for (auto &file : files) {
        Lexer lexer = Lexer(file);
        TokenStream stream = lexer.unwrap();
        Parser parser = Parser(file, stream);
        units.addUnit(parser.get());
    }

    EXPECT_NO_FATAL_FAILURE(units.drive(Options()));

    std::remove("lib.mdl");
    std::remove("box.mdl");
}

#define LISTED_USE_TYPE_BAD_1 R"(box :: { x: i64, y: i32 })"
#define LISTED_USE_TYPE_BAD_2 R"(use { box } = "box"; test :: () box { ret box { x: 1, y: 2 }; })"
TEST_F(MultiUnitTest, Two_Files_Listed_Use_Type_Bad) {
    std::ofstream F1("box.mdl");
    F1 << LISTED_USE_TYPE_BAD_1;
    F1.close();

    std::ofstream F2("lib.mdl");
    F2 << LISTED_USE_TYPE_BAD_2;
    F2.close();

    std::vector<File> files = { parseInputFile("lib.mdl"), parseInputFile("box.mdl") };
    UnitManager units;
    
    for (auto &file : files) {
        Lexer lexer = Lexer(file);
        TokenStream stream = lexer.unwrap();
        Parser parser = Parser(file, stream);
        units.addUnit(parser.get());
    }

    EXPECT_EXIT(units.drive(Options()), ::testing::ExitedWithCode(1), ".*");

    std::remove("lib.mdl");
    std::remove("box.mdl");
}

#define NAMED_USE_TYPE_1 R"($public box :: { x: i64, y: i32 })"
#define NAMED_USE_TYPE_2 R"(use Boxes = "box"; test :: () Boxes::box { ret Boxes::box { x: 1, y: 2 }; })"
TEST_F(MultiUnitTest, Two_Files_Named_Use_Type) {
    std::ofstream F1("box.mdl");
    F1 << NAMED_USE_TYPE_1;
    F1.close();

    std::ofstream F2("lib.mdl");
    F2 << NAMED_USE_TYPE_2;
    F2.close();

    std::vector<File> files = { parseInputFile("lib.mdl"), parseInputFile("box.mdl") };
    UnitManager units;
    
    for (auto &file : files) {
        Lexer lexer = Lexer(file);
        TokenStream stream = lexer.unwrap();
        Parser parser = Parser(file, stream);
        units.addUnit(parser.get());
    }

    EXPECT_NO_FATAL_FAILURE(units.drive(Options()));

    std::remove("lib.mdl");
    std::remove("box.mdl");
}

#define NAMED_USE_TYPE_BAD_1 R"(box :: { x: i64, y: i32 })"
#define NAMED_USE_TYPE_BAD_2 R"(use Boxes = "box"; test :: () box { ret Boxes::box { x: 1, y: 2 }; })"
TEST_F(MultiUnitTest, Two_Files_Named_Use_Type_Bad) {
    std::ofstream F1("box.mdl");
    F1 << NAMED_USE_TYPE_BAD_1;
    F1.close();

    std::ofstream F2("lib.mdl");
    F2 << NAMED_USE_TYPE_BAD_2;
    F2.close();

    std::vector<File> files = { parseInputFile("lib.mdl"), parseInputFile("box.mdl") };
    UnitManager units;
    
    for (auto &file : files) {
        Lexer lexer = Lexer(file);
        TokenStream stream = lexer.unwrap();
        Parser parser = Parser(file, stream);
        units.addUnit(parser.get());
    }

    EXPECT_EXIT(units.drive(Options()), ::testing::ExitedWithCode(1), ".*");

    std::remove("lib.mdl");
    std::remove("box.mdl");
}

#define NAMED_USE_ENUM_1 R"($public Colors :: i64 { Red, Blue })"
#define NAMED_USE_ENUM_2 R"(use CLI = "cli"; test :: () CLI::Colors { ret CLI::Red; })"
TEST_F(MultiUnitTest, Two_Files_Named_Use_Enum) {
    std::ofstream F1("cli.mdl");
    F1 << NAMED_USE_ENUM_1;
    F1.close();

    std::ofstream F2("lib.mdl");
    F2 << NAMED_USE_ENUM_2;
    F2.close();

    std::vector<File> files = { parseInputFile("lib.mdl"), parseInputFile("cli.mdl") };
    UnitManager units;
    
    for (auto &file : files) {
        Lexer lexer = Lexer(file);
        TokenStream stream = lexer.unwrap();
        Parser parser = Parser(file, stream);
        units.addUnit(parser.get());
    }

    EXPECT_NO_FATAL_FAILURE(units.drive(Options()));

    std::remove("lib.mdl");
    std::remove("cli.mdl");
}

} // namespace test

} // namespace meddle
