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

#define VAR_LOCAL_NESTED_ARRAY_TYPE R"(test::() { mut x: i64[2][3]; })"
TEST_F(ParseDeclTest, Var_Local_Nested_Array_Type) {
    File file = File("", "", "", VAR_LOCAL_NESTED_ARRAY_TYPE);
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
    EXPECT_EQ(VD->getType()->getName(), "i64[2][3]");
    EXPECT_EQ(VD->getInit(), nullptr);
    EXPECT_TRUE(VD->isMutable());
    EXPECT_FALSE(VD->isGlobal());

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

#define ENUM_BASIC R"(Colors :: i64 { Red = 0, Blue = 1, Green, Yellow = 4, Orange })"
TEST_F(ParseDeclTest, Enum_Basic) {
    File file = File("", "", "", ENUM_BASIC);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getDecls().size(), 1);

    EnumDecl *ED = dynamic_cast<EnumDecl *>(unit->getDecls()[0]);
    EXPECT_NE(ED, nullptr);
    EXPECT_EQ(ED->getName(), "Colors");
    EXPECT_EQ(ED->getDefinedType()->getName(), "Colors");
    EXPECT_EQ(ED->getVariants().size(), 5);

    EnumVariantDecl *EV1 = ED->getVariants()[0];
    EXPECT_EQ(EV1->getName(), "Red");
    EXPECT_EQ(EV1->getValue(), 0);

    EnumVariantDecl *EV2 = ED->getVariants()[1];
    EXPECT_EQ(EV2->getName(), "Blue");
    EXPECT_EQ(EV2->getValue(), 1);

    EnumVariantDecl *EV3 = ED->getVariants()[2];
    EXPECT_EQ(EV3->getName(), "Green");
    EXPECT_EQ(EV3->getValue(), 2);

    EnumVariantDecl *EV4 = ED->getVariants()[3];
    EXPECT_EQ(EV4->getName(), "Yellow");
    EXPECT_EQ(EV4->getValue(), 4);

    EnumVariantDecl *EV5 = ED->getVariants()[4];
    EXPECT_EQ(EV5->getName(), "Orange");
    EXPECT_EQ(EV5->getValue(), 5);

    delete unit;
}

#define STRUCT_BASIC R"(box :: { x: i64, y: i64, z: i64 = 42 })"
TEST_F(ParseDeclTest, Struct_Basic) {
    File file = File("", "", "", STRUCT_BASIC);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getDecls().size(), 1);

    StructDecl *SD = dynamic_cast<StructDecl *>(unit->getDecls()[0]);
    EXPECT_NE(SD, nullptr);
    EXPECT_EQ(SD->getName(), "box");
    EXPECT_EQ(SD->getFields().size(), 3);

    FieldDecl *FD1 = SD->getFields()[0];
    EXPECT_EQ(FD1->getName(), "x");
    EXPECT_EQ(FD1->getType()->getName(), "i64");
    EXPECT_EQ(FD1->getInit(), nullptr);
    EXPECT_EQ(FD1->getParent(), SD);

    FieldDecl *FD2 = SD->getFields()[1];
    EXPECT_EQ(FD2->getName(), "y");
    EXPECT_EQ(FD2->getType()->getName(), "i64");
    EXPECT_EQ(FD2->getInit(), nullptr);
    EXPECT_EQ(FD2->getParent(), SD);

    FieldDecl *FD3 = SD->getFields()[2];
    EXPECT_EQ(FD3->getName(), "z");
    EXPECT_EQ(FD3->getType()->getName(), "i64");
    EXPECT_NE(FD3->getInit(), nullptr);
    EXPECT_EQ(FD3->getParent(), SD);

    IntegerLiteral *I = dynamic_cast<IntegerLiteral *>(FD3->getInit());
    EXPECT_NE(I, nullptr);
    EXPECT_EQ(I->getValue(), 42);

    delete unit;
}

#define STRUCT_METHODS_BASIC R"(box :: { x: i64, y: i64, foo :: () { ret; }})"
TEST_F(ParseDeclTest, Struct_Methods_Basic) {
    File file = File("", "", "", STRUCT_METHODS_BASIC);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getDecls().size(), 1);

    StructDecl *SD = dynamic_cast<StructDecl *>(unit->getDecls()[0]);
    EXPECT_NE(SD, nullptr);
    EXPECT_EQ(SD->getName(), "box");
    EXPECT_EQ(SD->getFunctions().size(), 1);

    FunctionDecl *FD = SD->getFunctions()[0];
    EXPECT_EQ(FD->getName(), "foo");
    EXPECT_EQ(FD->getReturnType()->getName(), "void");
    EXPECT_EQ(FD->getParams().size(), 0);
    EXPECT_NE(FD->getBody(), nullptr);
    EXPECT_EQ(FD->getParent(), SD);
    EXPECT_EQ(FD->isMethod(), true);

    CompoundStmt *CS = dynamic_cast<CompoundStmt *>(FD->getBody());
    EXPECT_NE(CS, nullptr);
    EXPECT_EQ(CS->getStmts().size(), 1);

    RetStmt *RS = dynamic_cast<RetStmt *>(CS->getStmts()[0]);
    EXPECT_NE(RS, nullptr);
    EXPECT_EQ(RS->getExpr(), nullptr);

    delete unit;
}

#define STRUCT_ASSOCIATED_FUNCTION_BASIC R"(box :: { x: i64, y: i64, $associated foo :: () { ret; }})"
TEST_F(ParseDeclTest, Struct_Associated_Function_Basic) {
    File file = File("", "", "", STRUCT_ASSOCIATED_FUNCTION_BASIC);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getDecls().size(), 1);

    StructDecl *SD = dynamic_cast<StructDecl *>(unit->getDecls()[0]);
    EXPECT_NE(SD, nullptr);
    EXPECT_EQ(SD->getName(), "box");
    EXPECT_EQ(SD->getFunctions().size(), 1);

    FunctionDecl *FD = SD->getFunctions()[0];
    EXPECT_EQ(FD->getName(), "foo");
    EXPECT_EQ(FD->getReturnType()->getName(), "void");
    EXPECT_EQ(FD->getParams().size(), 0);
    EXPECT_NE(FD->getBody(), nullptr);
    EXPECT_EQ(FD->getParent(), SD);
    EXPECT_EQ(FD->isMethod(), false);

    CompoundStmt *CS = dynamic_cast<CompoundStmt *>(FD->getBody());
    EXPECT_NE(CS, nullptr);
    EXPECT_EQ(CS->getStmts().size(), 1);

    RetStmt *RS = dynamic_cast<RetStmt *>(CS->getStmts()[0]);
    EXPECT_NE(RS, nullptr);
    EXPECT_EQ(RS->getExpr(), nullptr);

    delete unit;
}

#define STRUCT_METHOD_FIELD_REF R"(box :: { x: i64, foo :: () i64 { ret x; } })"
TEST_F(ParseDeclTest, Struct_Method_Field_Ref) {
    File file = File("", "", "", STRUCT_METHOD_FIELD_REF);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getDecls().size(), 1);

    StructDecl *SD = dynamic_cast<StructDecl *>(unit->getDecls()[0]);
    EXPECT_NE(SD, nullptr);
    EXPECT_EQ(SD->getName(), "box");
    EXPECT_EQ(SD->getFunctions().size(), 1);
    EXPECT_EQ(SD->getFields().size(), 1);

    FieldDecl *FLD = SD->getFields()[0];
    EXPECT_EQ(FLD->getName(), "x");
    EXPECT_EQ(FLD->getType()->getName(), "i64");
    EXPECT_EQ(FLD->getInit(), nullptr);

    FunctionDecl *FD = SD->getFunctions()[0];
    EXPECT_EQ(FD->getName(), "foo");
    EXPECT_EQ(FD->getReturnType()->getName(), "i64");
    EXPECT_EQ(FD->getParams().size(), 0);
    EXPECT_NE(FD->getBody(), nullptr);
    EXPECT_EQ(FD->getParent(), SD);
    EXPECT_EQ(FD->isMethod(), true);

    CompoundStmt *CS = dynamic_cast<CompoundStmt *>(FD->getBody());
    EXPECT_NE(CS, nullptr);
    EXPECT_EQ(CS->getStmts().size(), 1);

    RetStmt *RS = dynamic_cast<RetStmt *>(CS->getStmts()[0]);
    EXPECT_NE(RS, nullptr);
    EXPECT_NE(RS->getExpr(), nullptr);

    RefExpr *VRE = dynamic_cast<RefExpr *>(RS->getExpr());
    EXPECT_NE(VRE, nullptr);
    EXPECT_EQ(VRE->getName(), "x");
    EXPECT_EQ(VRE->getRef(), FLD);

    delete unit;
}

#define STRUCT_FIELD_ACCESS_BASIC R"(test::() { mut x: Struct; x.a = 42; })"
TEST_F(ParseDeclTest, Struct_Field_Access_Basic) {
    File file = File("", "", "", STRUCT_FIELD_ACCESS_BASIC);
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
    EXPECT_EQ(VD->getInit(), nullptr);

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);
    EXPECT_NE(ES->getExpr(), nullptr);

    BinaryExpr *BE = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BE, nullptr);
    EXPECT_NE(BE->getLHS(), nullptr);
    EXPECT_NE(BE->getRHS(), nullptr);

    AccessExpr *AE = dynamic_cast<AccessExpr *>(BE->getLHS());
    EXPECT_NE(AE, nullptr);
    EXPECT_NE(AE->getBase(), nullptr);
    EXPECT_EQ(AE->getName(), "a");

    RefExpr *VRE = dynamic_cast<RefExpr *>(AE->getBase());
    EXPECT_NE(VRE, nullptr);
    EXPECT_EQ(VRE->getName(), "x");
    EXPECT_EQ(VRE->getRef(), VD);

    delete unit;
}

#define USE_UNNAMED R"(use "test";)"
TEST_F(ParseDeclTest, Use_Anonymous) {
    File file = File("", "", "", USE_UNNAMED);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getUses().size(), 1);

    UseDecl *UD = dynamic_cast<UseDecl *>(unit->getUses()[0]);
    EXPECT_NE(UD, nullptr);
    EXPECT_EQ(UD->getPath(), "test");
    EXPECT_EQ(UD->getSymbols().size(), 0);

    delete unit;
}

#define USE_NAMED R"(use Name = "test";)"
TEST_F(ParseDeclTest, Use_Named) {
    File file = File("", "", "", USE_NAMED);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getUses().size(), 1);

    UseDecl *UD = dynamic_cast<UseDecl *>(unit->getUses()[0]);
    EXPECT_NE(UD, nullptr);
    EXPECT_EQ(UD->getPath(), "test");
    EXPECT_EQ(UD->getName(), "Name");
    EXPECT_EQ(UD->getSymbols().size(), 0);
    EXPECT_EQ(UD->isNamed(), true);

    delete unit;
}

#define USE_LIST R"(use { Foo, Bar } = "test";)"
TEST_F(ParseDeclTest, Use_List) {
    File file = File("", "", "", USE_LIST);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getUses().size(), 1);

    UseDecl *UD = dynamic_cast<UseDecl *>(unit->getUses()[0]);
    EXPECT_NE(UD, nullptr);
    EXPECT_EQ(UD->getPath(), "test");
    EXPECT_EQ(UD->getName(), "");
    EXPECT_EQ(UD->getSymbols().size(), 2);
    EXPECT_EQ(UD->isNamed(), false);
    EXPECT_EQ(UD->getSymbols()[0], "Foo");
    EXPECT_EQ(UD->getSymbols()[1], "Bar");

    delete unit;
}

#define TEMPLATE_FUNCTION R"(foo<T> :: (x: T) T { ret x; })"
TEST_F(ParseDeclTest, Template_Function) {
    File file = File("", "", "", TEMPLATE_FUNCTION);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getDecls().size(), 1);

    FunctionDecl *FD = dynamic_cast<FunctionDecl *>(unit->getDecls()[0]);
    EXPECT_NE(FD, nullptr);
    EXPECT_EQ(FD->getName(), "foo");
    EXPECT_EQ(FD->getReturnType()->getName(), "T");
    EXPECT_EQ(FD->getNumParams(), 1);
    EXPECT_EQ(FD->getNumTemplateParams(), 1);

    TemplateParamDecl *TPD = FD->getTemplateParam(0);
    EXPECT_EQ(TPD->getName(), "T");
    EXPECT_EQ(TPD->getDefinedType()->getName(), "T");

    ParamDecl *PD = FD->getParam(0);
    EXPECT_EQ(PD->getName(), "x");
    EXPECT_EQ(PD->getType()->getName(), "T");

    delete unit;
}

#define TEMPLATE_STRUCT R"(box<T> :: { x: T })"
TEST_F(ParseDeclTest, Template_Struct) {
    File file = File("", "", "", TEMPLATE_STRUCT);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getDecls().size(), 1);

    StructDecl *SD = dynamic_cast<StructDecl *>(unit->getDecls()[0]);
    EXPECT_NE(SD, nullptr);
    EXPECT_EQ(SD->getName(), "box");
    EXPECT_EQ(SD->getNumFields(), 1);
    EXPECT_EQ(SD->getNumTemplateParams(), 1);

    TemplateParamDecl *TPD = SD->getTemplateParam(0);
    EXPECT_EQ(TPD->getName(), "T");
    EXPECT_EQ(TPD->getDefinedType()->getName(), "T");

    FieldDecl *FD = SD->getFields()[0];
    EXPECT_EQ(FD->getName(), "x");
    EXPECT_EQ(FD->getType()->getName(), "T");

    delete unit;
}

#define TEMPLATE_STRUCT_WITH_METHOD R"(box<T> :: { x: T, foo :: () T { ret x; } })"
TEST_F(ParseDeclTest, Template_Struct_With_Method) {
    File file = File("", "", "", TEMPLATE_STRUCT_WITH_METHOD);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getDecls().size(), 1);

    StructDecl *SD = dynamic_cast<StructDecl *>(unit->getDecls()[0]);
    EXPECT_NE(SD, nullptr);
    EXPECT_EQ(SD->getName(), "box");
    EXPECT_EQ(SD->getNumFields(), 1);
    EXPECT_EQ(SD->getNumFunctions(), 1);
    EXPECT_EQ(SD->getNumTemplateParams(), 1);

    TemplateParamDecl *TPD = SD->getTemplateParam(0);
    EXPECT_EQ(TPD->getName(), "T");
    EXPECT_EQ(TPD->getDefinedType()->getName(), "T");

    FieldDecl *FLD = SD->getField(0);
    EXPECT_EQ(FLD->getName(), "x");
    EXPECT_EQ(FLD->getType()->getName(), "T");

    FunctionDecl *FD = SD->getFunction(0);
    EXPECT_EQ(FD->getName(), "foo");
    EXPECT_EQ(FD->getReturnType()->getName(), "T");
    EXPECT_EQ(FD->getNumParams(), 0);

    delete unit;
}

#define TEMPLATE_STRUCT_WITH_TEMPLATE_METHOD R"(box<T> :: { x: T, foo<U> :: () U { ret x; } })"
TEST_F(ParseDeclTest, Template_Struct_With_Template_Method) {
    File file = File("", "", "", TEMPLATE_STRUCT_WITH_TEMPLATE_METHOD);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getDecls().size(), 1);

    StructDecl *SD = dynamic_cast<StructDecl *>(unit->getDecls()[0]);
    EXPECT_NE(SD, nullptr);
    EXPECT_EQ(SD->getName(), "box");
    EXPECT_EQ(SD->getNumTemplateParams(), 1);
    EXPECT_EQ(SD->getNumFields(), 1);
    EXPECT_EQ(SD->getNumFunctions(), 1);

    TemplateParamDecl *TPD = SD->getTemplateParam(0);
    EXPECT_EQ(TPD->getName(), "T");
    EXPECT_EQ(TPD->getDefinedType()->getName(), "T");

    FieldDecl *FD = SD->getField(0);
    EXPECT_EQ(FD->getName(), "x");
    EXPECT_EQ(FD->getType()->getName(), "T");

    FunctionDecl *FD2 = SD->getFunction(0);
    EXPECT_EQ(FD2->getName(), "foo");
    EXPECT_EQ(FD2->getReturnType()->getName(), "U");
    EXPECT_EQ(FD2->getParams().size(), 0);

    delete unit;
}

#define TEMPLATE_STRUCT_WITH_TEMPLATE_METHOD2 R"(box<T> :: { x: T, foo<U> :: () T { ret x; } })"
TEST_F(ParseDeclTest, Template_Struct_With_Template_Method2) {
    File file = File("", "", "", TEMPLATE_STRUCT_WITH_TEMPLATE_METHOD2);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getDecls().size(), 1);

    StructDecl *SD = dynamic_cast<StructDecl *>(unit->getDecls()[0]);
    EXPECT_NE(SD, nullptr);
    EXPECT_EQ(SD->getName(), "box");
    EXPECT_EQ(SD->getNumTemplateParams(), 1);
    EXPECT_EQ(SD->getNumFields(), 1);
    EXPECT_EQ(SD->getNumFunctions(), 1);

    TemplateParamDecl *TPD = SD->getTemplateParam(0);
    EXPECT_EQ(TPD->getName(), "T");
    EXPECT_EQ(TPD->getDefinedType()->getName(), "T");

    FieldDecl *FD = SD->getField(0);
    EXPECT_EQ(FD->getName(), "x");
    EXPECT_EQ(FD->getType()->getName(), "T");

    FunctionDecl *FD2 = SD->getFunction(0);
    EXPECT_EQ(FD2->getName(), "foo");
    EXPECT_EQ(FD2->getReturnType()->getName(), "T");
    EXPECT_EQ(FD2->getNumParams(), 0);
    EXPECT_EQ(FD2->getNumTemplateParams(), 1);
    
    TemplateParamDecl *TPD2 = FD2->getTemplateParam(0);
    EXPECT_EQ(TPD2->getName(), "U");
    EXPECT_EQ(TPD2->getDefinedType()->getName(), "U");

    delete unit;
}

} // namespace test

} // namespace meddle
