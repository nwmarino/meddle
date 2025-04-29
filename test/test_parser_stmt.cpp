#include "../compiler/parser/parser.h"
#include "../compiler/lexer/lexer.h"
#include "../compiler/tree/decl.h"

#include "gtest/gtest.h"
#include <gtest/gtest.h>

namespace meddle {

namespace test {

class ParseStmtTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

#define IF_1 R"(test::() { if 1 { ret; }})"
TEST_F(ParseStmtTest, If_Only_Then_Compound) {
    File file = File("", "", "", IF_1);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getDecls().size(), 1);

    FunctionDecl *FN = dynamic_cast<FunctionDecl *>(unit->getDecls()[0]);
    EXPECT_NE(FN, nullptr);
    EXPECT_NE(FN->getBody(), nullptr);

    CompoundStmt *CS = dynamic_cast<CompoundStmt *>(FN->getBody());
    EXPECT_NE(CS, nullptr);
    EXPECT_EQ(CS->getStmts().size(), 1);

    IfStmt *IS = dynamic_cast<IfStmt *>(CS->getStmts()[0]);
    EXPECT_NE(IS, nullptr);
    EXPECT_NE(IS->getCond(), nullptr);
    EXPECT_NE(IS->getThen(), nullptr);
    EXPECT_EQ(IS->getElse(), nullptr);
    
    IntegerLiteral *I = dynamic_cast<IntegerLiteral *>(IS->getCond());
    EXPECT_NE(I, nullptr);
    EXPECT_EQ(I->getValue(), 1);

    CompoundStmt *CS2 = dynamic_cast<CompoundStmt *>(IS->getThen());
    EXPECT_NE(CS2, nullptr);
    EXPECT_EQ(CS2->getStmts().size(), 1);

    RetStmt *RS = dynamic_cast<RetStmt *>(CS2->getStmts()[0]);
    EXPECT_NE(RS, nullptr);
    EXPECT_EQ(RS->getExpr(), nullptr);

    delete unit;
}

#define IF_2 R"(test::() { if 1 ret; })"
TEST_F(ParseStmtTest, If_Only_Then) {
    File file = File("", "", "", IF_2);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getDecls().size(), 1);

    FunctionDecl *FN = dynamic_cast<FunctionDecl *>(unit->getDecls()[0]);
    EXPECT_NE(FN, nullptr);
    EXPECT_NE(FN->getBody(), nullptr);

    CompoundStmt *CS = dynamic_cast<CompoundStmt *>(FN->getBody());
    EXPECT_NE(CS, nullptr);
    EXPECT_EQ(CS->getStmts().size(), 1);

    IfStmt *IS = dynamic_cast<IfStmt *>(CS->getStmts()[0]);
    EXPECT_NE(IS, nullptr);
    EXPECT_NE(IS->getCond(), nullptr);
    EXPECT_NE(IS->getThen(), nullptr);
    EXPECT_EQ(IS->getElse(), nullptr);
    
    IntegerLiteral *I = dynamic_cast<IntegerLiteral *>(IS->getCond());
    EXPECT_NE(I, nullptr);
    EXPECT_EQ(I->getValue(), 1);

    RetStmt *RS = dynamic_cast<RetStmt *>(IS->getThen());
    EXPECT_NE(RS, nullptr);
    EXPECT_EQ(RS->getExpr(), nullptr);

    delete unit;
}

#define IF_3 R"(test::() { if 1 ret; else ret; })"
TEST_F(ParseStmtTest, If_Then_Else) {
    File file = File("", "", "", IF_3);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getDecls().size(), 1);

    FunctionDecl *FN = dynamic_cast<FunctionDecl *>(unit->getDecls()[0]);
    EXPECT_NE(FN, nullptr);
    EXPECT_NE(FN->getBody(), nullptr);

    CompoundStmt *CS = dynamic_cast<CompoundStmt *>(FN->getBody());
    EXPECT_NE(CS, nullptr);
    EXPECT_EQ(CS->getStmts().size(), 1);

    IfStmt *IS = dynamic_cast<IfStmt *>(CS->getStmts()[0]);
    EXPECT_NE(IS, nullptr);
    EXPECT_NE(IS->getCond(), nullptr);
    EXPECT_NE(IS->getThen(), nullptr);
    EXPECT_NE(IS->getElse(), nullptr);

    IntegerLiteral *I = dynamic_cast<IntegerLiteral *>(IS->getCond());
    EXPECT_NE(I, nullptr);
    EXPECT_EQ(I->getValue(), 1);

    RetStmt *RS = dynamic_cast<RetStmt *>(IS->getThen());
    EXPECT_NE(RS, nullptr);
    EXPECT_EQ(RS->getExpr(), nullptr);

    RetStmt *RS2 = dynamic_cast<RetStmt *>(IS->getElse());
    EXPECT_NE(RS2, nullptr);
    EXPECT_EQ(RS2->getExpr(), nullptr);

    delete unit;
}

#define IF_4 R"(test::() { if 1 { ret; } else { ret; } })"
TEST_F(ParseStmtTest, If_Then_Compound_Else_Compound) {
    File file = File("", "", "", IF_4);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getDecls().size(), 1);

    FunctionDecl *FN = dynamic_cast<FunctionDecl *>(unit->getDecls()[0]);
    EXPECT_NE(FN, nullptr);
    EXPECT_NE(FN->getBody(), nullptr);

    CompoundStmt *CS = dynamic_cast<CompoundStmt *>(FN->getBody());
    EXPECT_NE(CS, nullptr);
    EXPECT_EQ(CS->getStmts().size(), 1);

    IfStmt *IS = dynamic_cast<IfStmt *>(CS->getStmts()[0]);
    EXPECT_NE(IS, nullptr);
    EXPECT_NE(IS->getCond(), nullptr);
    EXPECT_NE(IS->getThen(), nullptr);
    EXPECT_NE(IS->getElse(), nullptr);

    IntegerLiteral *I = dynamic_cast<IntegerLiteral *>(IS->getCond());
    EXPECT_NE(I, nullptr);
    EXPECT_EQ(I->getValue(), 1);

    CompoundStmt *CS2 = dynamic_cast<CompoundStmt *>(IS->getThen());
    EXPECT_NE(CS2, nullptr);
    EXPECT_EQ(CS2->getStmts().size(), 1);

    RetStmt *RS = dynamic_cast<RetStmt *>(CS2->getStmts()[0]);
    EXPECT_NE(RS, nullptr);
    EXPECT_EQ(RS->getExpr(), nullptr);

    CompoundStmt *CS3 = dynamic_cast<CompoundStmt *>(IS->getElse());
    EXPECT_NE(CS3, nullptr);
    EXPECT_EQ(CS3->getStmts().size(), 1);

    RetStmt *RS2 = dynamic_cast<RetStmt *>(CS3->getStmts()[0]);
    EXPECT_NE(RS2, nullptr);
    EXPECT_EQ(RS2->getExpr(), nullptr);

    delete unit;
}

#define UNTIL_1 R"(test::() { until 1 { ret; } })"
TEST_F(ParseStmtTest, Until_Basic) {
    File file = File("", "", "", UNTIL_1);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getDecls().size(), 1);

    FunctionDecl *FN = dynamic_cast<FunctionDecl *>(unit->getDecls()[0]);
    EXPECT_NE(FN, nullptr);
    EXPECT_NE(FN->getBody(), nullptr);

    CompoundStmt *CS = dynamic_cast<CompoundStmt *>(FN->getBody());
    EXPECT_NE(CS, nullptr);
    EXPECT_EQ(CS->getStmts().size(), 1);

    UntilStmt *US = dynamic_cast<UntilStmt *>(CS->getStmts()[0]);
    EXPECT_NE(US, nullptr);
    EXPECT_NE(US->getCond(), nullptr);
    EXPECT_NE(US->getBody(), nullptr);

    IntegerLiteral *I = dynamic_cast<IntegerLiteral *>(US->getCond());
    EXPECT_NE(I, nullptr);
    EXPECT_EQ(I->getValue(), 1);

    CompoundStmt *CS2 = dynamic_cast<CompoundStmt *>(US->getBody());
    EXPECT_NE(CS2, nullptr);
    EXPECT_EQ(CS2->getStmts().size(), 1);

    RetStmt *RS = dynamic_cast<RetStmt *>(CS2->getStmts()[0]);
    EXPECT_NE(RS, nullptr);
    EXPECT_EQ(RS->getExpr(), nullptr);

    delete unit;
}

#define MATCH_1 R"(test::() { match 1 { 1 -> { ret; } } })"
TEST_F(ParseStmtTest, Match_One_Case) {
    File file = File("", "", "", MATCH_1);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getDecls().size(), 1);
    FunctionDecl *FN = dynamic_cast<FunctionDecl *>(unit->getDecls()[0]);
    EXPECT_NE(FN, nullptr);
    EXPECT_NE(FN->getBody(), nullptr);

    CompoundStmt *CS = dynamic_cast<CompoundStmt *>(FN->getBody());
    EXPECT_NE(CS, nullptr);
    EXPECT_EQ(CS->getStmts().size(), 1);

    MatchStmt *MS = dynamic_cast<MatchStmt *>(CS->getStmts()[0]);
    EXPECT_NE(MS, nullptr);
    EXPECT_NE(MS->getPattern(), nullptr);
    EXPECT_NE(MS->getCases().size(), 0);
    EXPECT_EQ(MS->getDefault(), nullptr);
    
    IntegerLiteral *I = dynamic_cast<IntegerLiteral *>(MS->getPattern());
    EXPECT_NE(I, nullptr);
    EXPECT_EQ(I->getValue(), 1);
    EXPECT_EQ(MS->getCases().size(), 1);

    CaseStmt *CS1 = dynamic_cast<CaseStmt *>(MS->getCases()[0]);
    EXPECT_NE(CS1, nullptr);
    EXPECT_NE(CS1->getPattern(), nullptr);
    EXPECT_NE(CS1->getBody(), nullptr);

    IntegerLiteral *I2 = dynamic_cast<IntegerLiteral *>(CS1->getPattern());
    EXPECT_NE(I2, nullptr);
    EXPECT_EQ(I2->getValue(), 1);

    CompoundStmt *CS2 = dynamic_cast<CompoundStmt *>(CS1->getBody());
    EXPECT_NE(CS2, nullptr);
    EXPECT_EQ(CS2->getStmts().size(), 1);

    RetStmt *RS = dynamic_cast<RetStmt *>(CS2->getStmts()[0]);
    EXPECT_NE(RS, nullptr);
    EXPECT_EQ(RS->getExpr(), nullptr);

    delete unit;
}

#define MATCH_2 R"(test::() { match 1 { 1 -> { ret; } 2 -> ret; } })"
TEST_F(ParseStmtTest, Match_Two_Cases) {
    File file = File("", "", "", MATCH_2);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getDecls().size(), 1);

    FunctionDecl *FN = dynamic_cast<FunctionDecl *>(unit->getDecls()[0]);
    EXPECT_NE(FN, nullptr);
    EXPECT_NE(FN->getBody(), nullptr);

    CompoundStmt *CS = dynamic_cast<CompoundStmt *>(FN->getBody());
    EXPECT_NE(CS, nullptr);
    EXPECT_EQ(CS->getStmts().size(), 1);

    MatchStmt *MS = dynamic_cast<MatchStmt *>(CS->getStmts()[0]);
    EXPECT_NE(MS, nullptr);
    EXPECT_NE(MS->getPattern(), nullptr);
    EXPECT_EQ(MS->getCases().size(), 2);
    EXPECT_EQ(MS->getDefault(), nullptr);

    IntegerLiteral *I = dynamic_cast<IntegerLiteral *>(MS->getPattern());
    EXPECT_NE(I, nullptr);
    EXPECT_EQ(I->getValue(), 1);

    CaseStmt *CS1 = dynamic_cast<CaseStmt *>(MS->getCases()[0]);
    EXPECT_NE(CS1, nullptr);
    EXPECT_NE(CS1->getPattern(), nullptr);
    EXPECT_NE(CS1->getBody(), nullptr);

    IntegerLiteral *I2 = dynamic_cast<IntegerLiteral *>(CS1->getPattern());
    EXPECT_NE(I2, nullptr);
    EXPECT_EQ(I2->getValue(), 1);

    CompoundStmt *CS2 = dynamic_cast<CompoundStmt *>(CS1->getBody());
    EXPECT_NE(CS2, nullptr);
    EXPECT_EQ(CS2->getStmts().size(), 1);

    RetStmt *RS = dynamic_cast<RetStmt *>(CS2->getStmts()[0]);
    EXPECT_NE(RS, nullptr);
    EXPECT_EQ(RS->getExpr(), nullptr);

    CaseStmt *CS3 = dynamic_cast<CaseStmt *>(MS->getCases()[1]);
    EXPECT_NE(CS3, nullptr);
    EXPECT_NE(CS3->getPattern(), nullptr);
    EXPECT_NE(CS3->getBody(), nullptr);

    IntegerLiteral *I3 = dynamic_cast<IntegerLiteral *>(CS3->getPattern());
    EXPECT_NE(I3, nullptr);
    EXPECT_EQ(I3->getValue(), 2);

    RetStmt *RS2 = dynamic_cast<RetStmt *>(CS3->getBody());
    EXPECT_NE(RS2, nullptr);
    EXPECT_EQ(RS2->getExpr(), nullptr);

    delete unit;
}

#define MATCH_3 R"(test::() { match 1 { 1 -> { ret; } _ -> ret; } })"
TEST_F(ParseStmtTest, Match_Case_Default) {
    File file = File("", "", "", MATCH_3);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getDecls().size(), 1);

    FunctionDecl *FN = dynamic_cast<FunctionDecl *>(unit->getDecls()[0]);
    EXPECT_NE(FN, nullptr);
    EXPECT_NE(FN->getBody(), nullptr);

    CompoundStmt *CS = dynamic_cast<CompoundStmt *>(FN->getBody());
    EXPECT_NE(CS, nullptr);
    EXPECT_EQ(CS->getStmts().size(), 1);

    MatchStmt *MS = dynamic_cast<MatchStmt *>(CS->getStmts()[0]);
    EXPECT_NE(MS, nullptr);
    EXPECT_NE(MS->getPattern(), nullptr);
    EXPECT_EQ(MS->getCases().size(), 1);
    EXPECT_NE(MS->getDefault(), nullptr);

    IntegerLiteral *I = dynamic_cast<IntegerLiteral *>(MS->getPattern());
    EXPECT_NE(I, nullptr);
    EXPECT_EQ(I->getValue(), 1);

    CaseStmt *CS1 = dynamic_cast<CaseStmt *>(MS->getCases()[0]);
    EXPECT_NE(CS1, nullptr);
    EXPECT_NE(CS1->getPattern(), nullptr);
    EXPECT_NE(CS1->getBody(), nullptr);

    IntegerLiteral *I2 = dynamic_cast<IntegerLiteral *>(CS1->getPattern());
    EXPECT_NE(I2, nullptr);
    EXPECT_EQ(I2->getValue(), 1);

    CompoundStmt *CS2 = dynamic_cast<CompoundStmt *>(CS1->getBody());
    EXPECT_NE(CS2, nullptr);
    EXPECT_EQ(CS2->getStmts().size(), 1);

    RetStmt *RS = dynamic_cast<RetStmt *>(CS2->getStmts()[0]);
    EXPECT_NE(RS, nullptr);
    EXPECT_EQ(RS->getExpr(), nullptr);

    RetStmt *RS2 = dynamic_cast<RetStmt *>(MS->getDefault());
    EXPECT_NE(RS2, nullptr);
    EXPECT_EQ(RS2->getExpr(), nullptr);

    delete unit;
}

#define MATCH_4 R"(test::() { match 1 { 1 -> { ret; } 2 -> ret; _ -> { ret; } } })"
TEST_F(ParseStmtTest, Match_Cases_Default) {
    File file = File("", "", "", MATCH_4);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getDecls().size(), 1);

    FunctionDecl *FN = dynamic_cast<FunctionDecl *>(unit->getDecls()[0]);
    EXPECT_NE(FN, nullptr);
    EXPECT_NE(FN->getBody(), nullptr);

    CompoundStmt *CS = dynamic_cast<CompoundStmt *>(FN->getBody());
    EXPECT_NE(CS, nullptr);
    EXPECT_EQ(CS->getStmts().size(), 1);

    MatchStmt *MS = dynamic_cast<MatchStmt *>(CS->getStmts()[0]);
    EXPECT_NE(MS, nullptr);
    EXPECT_NE(MS->getPattern(), nullptr);
    EXPECT_EQ(MS->getCases().size(), 2);
    EXPECT_NE(MS->getDefault(), nullptr);

    IntegerLiteral *I = dynamic_cast<IntegerLiteral *>(MS->getPattern());
    EXPECT_NE(I, nullptr);
    EXPECT_EQ(I->getValue(), 1);

    CaseStmt *CS1 = dynamic_cast<CaseStmt *>(MS->getCases()[0]);
    EXPECT_NE(CS1, nullptr);
    EXPECT_NE(CS1->getPattern(), nullptr);
    EXPECT_NE(CS1->getBody(), nullptr);

    IntegerLiteral *I2 = dynamic_cast<IntegerLiteral *>(CS1->getPattern());
    EXPECT_NE(I2, nullptr);
    EXPECT_EQ(I2->getValue(), 1);

    CompoundStmt *CS2 = dynamic_cast<CompoundStmt *>(CS1->getBody());
    EXPECT_NE(CS2, nullptr);
    EXPECT_EQ(CS2->getStmts().size(), 1);

    RetStmt *RS = dynamic_cast<RetStmt *>(CS2->getStmts()[0]);
    EXPECT_NE(RS, nullptr);
    EXPECT_EQ(RS->getExpr(), nullptr);

    CaseStmt *CS3 = dynamic_cast<CaseStmt *>(MS->getCases()[1]);
    EXPECT_NE(CS3, nullptr);
    EXPECT_NE(CS3->getPattern(), nullptr);
    EXPECT_NE(CS3->getBody(), nullptr);

    IntegerLiteral *I3 = dynamic_cast<IntegerLiteral *>(CS3->getPattern());
    EXPECT_NE(I3, nullptr);
    EXPECT_EQ(I3->getValue(), 2);

    RetStmt *RS2 = dynamic_cast<RetStmt *>(CS3->getBody());
    EXPECT_NE(RS2, nullptr);
    EXPECT_EQ(RS2->getExpr(), nullptr);

    CompoundStmt *CS4 = dynamic_cast<CompoundStmt *>(MS->getDefault());
    EXPECT_NE(CS4, nullptr);
    EXPECT_EQ(CS4->getStmts().size(), 1);

    RetStmt *RS3 = dynamic_cast<RetStmt *>(CS4->getStmts()[0]);
    EXPECT_NE(RS3, nullptr);
    EXPECT_EQ(RS3->getExpr(), nullptr);

    delete unit;
}

#define EXPR_1 R"(test::() { mut x: i64; x; })"
TEST_F(ParseStmtTest, Expr_Basic) {
    File file = File("", "", "", EXPR_1);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getDecls().size(), 1);

    FunctionDecl *FN = dynamic_cast<FunctionDecl *>(unit->getDecls()[0]);
    EXPECT_NE(FN, nullptr);
    EXPECT_NE(FN->getBody(), nullptr);

    CompoundStmt *CS = dynamic_cast<CompoundStmt *>(FN->getBody());
    EXPECT_NE(CS, nullptr);
    EXPECT_EQ(CS->getStmts().size(), 2);

    DeclStmt *DS = dynamic_cast<DeclStmt *>(CS->getStmts()[0]);
    EXPECT_NE(DS, nullptr);

    VarDecl *VD = dynamic_cast<VarDecl *>(DS->getDecl());
    EXPECT_NE(VD, nullptr);
    EXPECT_EQ(VD->getName(), "x");
    EXPECT_EQ(VD->getType()->getName(), "i64");

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);
    EXPECT_NE(ES->getExpr(), nullptr);

    RefExpr *RE = dynamic_cast<RefExpr *>(ES->getExpr());
    EXPECT_NE(RE, nullptr);

    delete unit;
}

#define BREAK_1 R"(test::() { break; })"
TEST_F(ParseStmtTest, Break_Basic) {
    File file = File("", "", "", BREAK_1);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getDecls().size(), 1);

    FunctionDecl *FN = dynamic_cast<FunctionDecl *>(unit->getDecls()[0]);
    EXPECT_NE(FN, nullptr);
    EXPECT_NE(FN->getBody(), nullptr);

    CompoundStmt *CS = dynamic_cast<CompoundStmt *>(FN->getBody());
    EXPECT_NE(CS, nullptr);
    EXPECT_EQ(CS->getStmts().size(), 1);

    BreakStmt *BS = dynamic_cast<BreakStmt *>(CS->getStmts()[0]);
    EXPECT_NE(BS, nullptr);

    delete unit;
}

#define CONTINUE_1 R"(test::() { continue; })"
TEST_F(ParseStmtTest, Continue_Basic) {
    File file = File("", "", "", CONTINUE_1);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getDecls().size(), 1);

    FunctionDecl *FN = dynamic_cast<FunctionDecl *>(unit->getDecls()[0]);
    EXPECT_NE(FN, nullptr);
    EXPECT_NE(FN->getBody(), nullptr);

    CompoundStmt *CS = dynamic_cast<CompoundStmt *>(FN->getBody());
    EXPECT_NE(CS, nullptr);
    EXPECT_EQ(CS->getStmts().size(), 1);

    ContinueStmt *BS = dynamic_cast<ContinueStmt *>(CS->getStmts()[0]);
    EXPECT_NE(BS, nullptr);

    delete unit;
}

} // namespace test

} // namespace meddle
