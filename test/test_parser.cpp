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

TEST_F(ParserTest, ParseFunctionLocalVarNoInit) {
    File file = File("test.mdl", "/", "/test.mdl", "test::(){mut x: i64;}");
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

TEST_F(ParserTest, ParseFunctionLocalVarInit) {
    File file = File("test.mdl", "/", "/test.mdl", "test::(){mut x: i64 = 0;}");
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

TEST_F(ParserTest, ParseLocalVarRef) {
    File file = File("test.mdl", "/", "/test.mdl", "test::(){mut x: i64 = 0; ret x;}");
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

} // namespace test

} // namespace meddle
