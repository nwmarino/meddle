#include "../compiler/parser/parser.h"
#include "../compiler/lexer/lexer.h"
#include "../compiler/tree/decl.h"

#include "gtest/gtest.h"
#include <gtest/gtest.h>

namespace meddle {

namespace test {

class ParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        
    }

    void TearDown() override {

    }
};

TEST_F(ParserTest, ParseEmptyFunction) {
    File file = File("test.mdl", "/", "/test.mdl", "test::();");
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getDecls().size(), 1);

    FunctionDecl *FN = dynamic_cast<FunctionDecl *>(unit->getDecls()[0]);
    EXPECT_NE(FN, nullptr);

    EXPECT_EQ(FN->getName(), "test");
    EXPECT_EQ(FN->getReturnType()->getName(), "void");
    EXPECT_EQ(FN->getParams().size(), 0);
    EXPECT_EQ(FN->getBody(), nullptr);

    delete unit;
}

TEST_F(ParserTest, ParseFunctionReturnZero) {
    File file = File("test.mdl", "/", "/test.mdl", "test::(){ret 0;}");
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getDecls().size(), 1);

    FunctionDecl *FN = dynamic_cast<FunctionDecl *>(unit->getDecls()[0]);
    EXPECT_NE(FN, nullptr);
    EXPECT_EQ(FN->getName(), "test");
    EXPECT_EQ(FN->getReturnType()->getName(), "void");
    EXPECT_EQ(FN->getParams().size(), 0);
    EXPECT_NE(FN->getBody(), nullptr);

    CompoundStmt *CS = dynamic_cast<CompoundStmt *>(FN->getBody());
    EXPECT_NE(CS, nullptr);
    EXPECT_EQ(CS->getStmts().size(), 1);

    RetStmt *RS = dynamic_cast<RetStmt *>(CS->getStmts()[0]);
    EXPECT_NE(RS, nullptr);
    EXPECT_NE(RS->getExpr(), nullptr);

    IntegerLiteral *I = dynamic_cast<IntegerLiteral *>(RS->getExpr());
    EXPECT_NE(I, nullptr);
    EXPECT_EQ(I->getValue(), 0);

    delete unit;
}

TEST_F(ParserTest, ParseFunctionLocalMutVarNoInit) {
    File file = File("test.mdl", "/", "/test.mdl", "test::(){mut x: i64;}");
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

    DeclStmt *DS = dynamic_cast<DeclStmt *>(CS->getStmts()[0]);
    EXPECT_NE(DS, nullptr);
    
    VarDecl *VD = dynamic_cast<VarDecl *>(DS->getDecl());
    EXPECT_NE(VD, nullptr);
    EXPECT_EQ(VD->getName(), "x");
    EXPECT_EQ(VD->getType()->getName(), "i64");
    EXPECT_EQ(VD->getInit(), nullptr);
    EXPECT_TRUE(VD->isMutable());
    EXPECT_FALSE(VD->isGlobal());

    delete unit;
}

TEST_F(ParserTest, ParseFunctionLocalMutVarInit) {
    File file = File("test.mdl", "/", "/test.mdl", "test::(){mut x: i64 = 0;}");
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

    DeclStmt *DS = dynamic_cast<DeclStmt *>(CS->getStmts()[0]);
    EXPECT_NE(DS, nullptr);
    
    VarDecl *VD = dynamic_cast<VarDecl *>(DS->getDecl());
    EXPECT_NE(VD, nullptr);
    EXPECT_EQ(VD->getName(), "x");
    EXPECT_EQ(VD->getType()->getName(), "i64");
    EXPECT_NE(VD->getInit(), nullptr);
    EXPECT_TRUE(VD->isMutable());
    EXPECT_FALSE(VD->isGlobal());

    IntegerLiteral *I = dynamic_cast<IntegerLiteral *>(VD->getInit());
    EXPECT_NE(I, nullptr);
    EXPECT_EQ(I->getValue(), 0);

    delete unit;
}

TEST_F(ParserTest, ParseFunctionLocalImmutVarInit) {
    File file = File("test.mdl", "/", "/test.mdl", "test::(){fix x: i64 = 0;}");
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

    DeclStmt *DS = dynamic_cast<DeclStmt *>(CS->getStmts()[0]);
    EXPECT_NE(DS, nullptr);

    VarDecl *VD = dynamic_cast<VarDecl *>(DS->getDecl());
    EXPECT_NE(VD, nullptr);
    EXPECT_EQ(VD->getName(), "x");
    EXPECT_EQ(VD->getType()->getName(), "i64");
    EXPECT_NE(VD->getInit(), nullptr);
    EXPECT_FALSE(VD->isMutable());
    EXPECT_FALSE(VD->isGlobal());

    IntegerLiteral *I = dynamic_cast<IntegerLiteral *>(VD->getInit());
    EXPECT_NE(I, nullptr);
    EXPECT_EQ(I->getValue(), 0);

    delete unit;
}

TEST_F(ParserTest, ParseLocalVarRef) {
    File file = File("test.mdl", "/", "/test.mdl", "test::(){mut x: i64 = 0; ret x;}");
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
    EXPECT_NE(VD->getInit(), nullptr);
    EXPECT_TRUE(VD->isMutable());
    EXPECT_FALSE(VD->isGlobal());

    IntegerLiteral *I = dynamic_cast<IntegerLiteral *>(VD->getInit());
    EXPECT_NE(I, nullptr);
    EXPECT_EQ(I->getValue(), 0);

    RetStmt *RS = dynamic_cast<RetStmt *>(CS->getStmts()[1]);
    EXPECT_NE(RS, nullptr);
    
    RefExpr *RE = dynamic_cast<RefExpr *>(RS->getExpr());
    EXPECT_NE(RE, nullptr);
    
    NamedDecl *ND = RE->getRef();
    EXPECT_EQ(ND, VD);

    delete unit;
}

TEST_F(ParserTest, ParseDoubleLocalVarInit) {
    File file = File("test.mdl", "/", "/test.mdl", "test::(){mut x: i64 = 0; fix y: i64 = 0;}");
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
    EXPECT_TRUE(VD->isMutable());

    DeclStmt *DS2 = dynamic_cast<DeclStmt *>(CS->getStmts()[1]);
    EXPECT_NE(DS2, nullptr);

    VarDecl *VD2 = dynamic_cast<VarDecl *>(DS2->getDecl());
    EXPECT_NE(VD2, nullptr);
    EXPECT_EQ(VD2->getName(), "y");
    EXPECT_EQ(VD2->getType()->getName(), "i64");
    EXPECT_FALSE(VD2->isMutable());

    delete unit;
}

TEST_F(ParserTest, ParseFPLocalVarInit) {
    File file = File("test.mdl", "/", "/test.mdl", "test::(){mut x: f32 = 3.14;}");
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

    DeclStmt *DS = dynamic_cast<DeclStmt *>(CS->getStmts()[0]);
    EXPECT_NE(DS, nullptr);

    VarDecl *VD = dynamic_cast<VarDecl *>(DS->getDecl());
    EXPECT_NE(VD, nullptr);
    EXPECT_EQ(VD->getName(), "x");
    EXPECT_EQ(VD->getType()->getName(), "f32");
    EXPECT_NE(VD->getInit(), nullptr);
    EXPECT_TRUE(VD->isMutable());
    EXPECT_FALSE(VD->isGlobal());

    FloatLiteral *F = dynamic_cast<FloatLiteral *>(VD->getInit());
    EXPECT_NE(F, nullptr);
    EXPECT_EQ(F->getValue(), 3.14);
    EXPECT_EQ(F->getType()->getName(), "f64");

    delete unit;
}

TEST_F(ParserTest, ParseCharLocalVarInit) {
    File file = File("test.mdl", "/", "/test.mdl", "test::(){mut x: char = 'a';}");
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

    DeclStmt *DS = dynamic_cast<DeclStmt *>(CS->getStmts()[0]);
    EXPECT_NE(DS, nullptr);

    VarDecl *VD = dynamic_cast<VarDecl *>(DS->getDecl());
    EXPECT_NE(VD, nullptr);
    EXPECT_EQ(VD->getName(), "x");
    EXPECT_EQ(VD->getType()->getName(), "char");
    EXPECT_NE(VD->getInit(), nullptr);
    EXPECT_TRUE(VD->isMutable());
    EXPECT_FALSE(VD->isGlobal());

    CharLiteral *C = dynamic_cast<CharLiteral *>(VD->getInit());
    EXPECT_NE(C, nullptr);
    EXPECT_EQ(C->getValue(), 'a');
    EXPECT_EQ(C->getType()->getName(), "char");

    delete unit;
}

TEST_F(ParserTest, ParseIfStatementOnlyThenCompound) {
    File file = File("test.mdl", "/", "/test.mdl", "test::(){if 1 {ret;}}");
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

TEST_F(ParserTest, ParseIfStatementOnlyThenNoCompound) {
    File file = File("test.mdl", "/", "/test.mdl", "test::(){if 1 ret;}");
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

TEST_F(ParserTest, ParseIfStatementElseNoCompound) {
    File file = File("test.mdl", "/", "/test.mdl", "test::(){if 1 ret; else ret;}");
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

TEST_F(ParserTest, ParseIfStatementElseCompound) {
    File file = File("test.mdl", "/", "/test.mdl", "test::(){if 1 { ret; } else { ret; } }");
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

TEST_F(ParserTest, ParseUntilStatement) {
    File file = File("test.mdl", "/", "/test.mdl", "test::(){until 1 { ret; }}");
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

TEST_F(ParserTest, ParseMatchStatementOneCase) {
    File file = File("test.mdl", "/", "/test.mdl", "test::(){match 1 { 1 -> { ret; } }}");
}

TEST_F(ParserTest, ParseMatchStatementTwoCases) {

}

TEST_F(ParserTest, ParseMatchStatementWithDefault) {

}

TEST_F(ParserTest, ParseMatchStatementManyWithDefault) {

}

} // namespace test

} // namespace meddle
