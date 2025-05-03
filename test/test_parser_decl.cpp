#include "../compiler/parser/parser.h"
#include "../compiler/lexer/lexer.h"
#include "../compiler/tree/decl.h"

#include "gtest/gtest.h"
#include <gtest/gtest.h>

namespace meddle {

namespace test {

class ParseDeclTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

#define FUNCTION_1 R"(test::();)"
TEST_F(ParseDeclTest, Function_Empty) {
    File file = File("", "", "", FUNCTION_1);
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

#define FUNCTION_2 R"(test::() { ret 0; })"
TEST_F(ParseDeclTest, Function_Single_Statement) {
    File file = File("", "", "", FUNCTION_2);
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

#define FUNCTION_PARAMS_BASIC R"(test::(x: i64) { ret; })"
TEST_F(ParseDeclTest, Function_Params_Basic) {
    File file = File("", "", "", FUNCTION_PARAMS_BASIC);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getDecls().size(), 1);

    FunctionDecl *FN = dynamic_cast<FunctionDecl *>(unit->getDecls()[0]);
    EXPECT_NE(FN, nullptr);
    EXPECT_EQ(FN->getName(), "test");
    EXPECT_EQ(FN->getReturnType()->getName(), "void");
    EXPECT_EQ(FN->getParams().size(), 1);

    ParamDecl *PD = FN->getParams()[0];
    EXPECT_EQ(PD->getName(), "x");
    EXPECT_EQ(PD->getType()->getName(), "i64");

    delete unit;
}

#define FUNCTION_PARAMS_MANY R"(test::(x: i64, y: f32, z: f64) { ret; })"
TEST_F(ParseDeclTest, Function_Params_Many) {
    File file = File("", "", "", FUNCTION_PARAMS_MANY);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getDecls().size(), 1);

    FunctionDecl *FN = dynamic_cast<FunctionDecl *>(unit->getDecls()[0]);
    EXPECT_NE(FN, nullptr);
    EXPECT_EQ(FN->getName(), "test");
    EXPECT_EQ(FN->getReturnType()->getName(), "void");
    EXPECT_EQ(FN->getParams().size(), 3);

    ParamDecl *PD1 = FN->getParams()[0];
    EXPECT_EQ(PD1->getName(), "x");
    EXPECT_EQ(PD1->getType()->getName(), "i64");

    ParamDecl *PD2 = FN->getParams()[1];
    EXPECT_EQ(PD2->getName(), "y");
    EXPECT_EQ(PD2->getType()->getName(), "f32");

    ParamDecl *PD3 = FN->getParams()[2];
    EXPECT_EQ(PD3->getName(), "z");
    EXPECT_EQ(PD3->getType()->getName(), "f64");

    delete unit;
}

#define VAR_LOCAL_1 R"(test::(){ mut x: i64; })"
TEST_F(ParseDeclTest, Var_Local_Mut_Empty) {
    File file = File("", "", "", VAR_LOCAL_1);
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

#define VAR_LOCAL_2 R"(test::(){ mut x: i64 = 0; })"
TEST_F(ParseDeclTest, Var_Local_Mut_Init) {
    File file = File("", "", "", VAR_LOCAL_2);
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

#define VAR_LOCAL_3 R"(test::(){ fix x: i64 = 0; })"
TEST_F(ParseDeclTest, Var_Local_Fix_Init) {
    File file = File("", "", "", VAR_LOCAL_3);
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

#define VAR_LOCAL_4 R"(test::() { fix x = 0; })"
TEST_F(ParseDeclTest, Var_Local_No_Type) {
    File file = File("", "", "", VAR_LOCAL_4);
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
    EXPECT_EQ(VD->getType(), nullptr);
    EXPECT_NE(VD->getInit(), nullptr);
    EXPECT_FALSE(VD->isMutable());
    EXPECT_FALSE(VD->isGlobal());

    IntegerLiteral *I = dynamic_cast<IntegerLiteral *>(VD->getInit());
    EXPECT_NE(I, nullptr);
    EXPECT_EQ(I->getValue(), 0);

    delete unit;
}

#define VAR_GLOBAL_1 R"(global :: fix i64 = 0;)"
TEST_F(ParseDeclTest, Var_Global_Fix_Init) {
    File file = File("", "", "", VAR_GLOBAL_1);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getDecls().size(), 1);

    VarDecl *VD = dynamic_cast<VarDecl *>(unit->getDecls()[0]);
    EXPECT_NE(VD, nullptr);
    EXPECT_EQ(VD->getName(), "global");
    EXPECT_EQ(VD->getType()->getName(), "i64");
    EXPECT_NE(VD->getInit(), nullptr);
    EXPECT_FALSE(VD->isMutable());
    EXPECT_TRUE(VD->isGlobal());

    IntegerLiteral *I = dynamic_cast<IntegerLiteral *>(VD->getInit());
    EXPECT_NE(I, nullptr);
    EXPECT_EQ(I->getValue(), 0);

    delete unit;
}

#define VAR_GLOBAL_2 R"(global :: mut i64 = 0;)"
TEST_F(ParseDeclTest, Var_Global_Mut_Init) {
    File file = File("", "", "", VAR_GLOBAL_2);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getDecls().size(), 1);

    VarDecl *VD = dynamic_cast<VarDecl *>(unit->getDecls()[0]);
    EXPECT_NE(VD, nullptr);
    EXPECT_EQ(VD->getName(), "global");
    EXPECT_EQ(VD->getType()->getName(), "i64");
    EXPECT_NE(VD->getInit(), nullptr);
    EXPECT_TRUE(VD->isMutable());
    EXPECT_TRUE(VD->isGlobal());

    IntegerLiteral *I = dynamic_cast<IntegerLiteral *>(VD->getInit());
    EXPECT_NE(I, nullptr);
    EXPECT_EQ(I->getValue(), 0);

    delete unit;
}

} // namespace test

} // namespace meddle
