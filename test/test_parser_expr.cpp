#include "../compiler/parser/parser.h"
#include "../compiler/lexer/lexer.h"
#include "../compiler/tree/decl.h"

#include "gtest/gtest.h"
#include <gtest/gtest.h>

namespace meddle {

namespace test {

class ParseExprTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

#define INT_B_I R"(test::() { mut x: i32 = 5; })"
TEST_F(ParseExprTest, IntLiteral_Basic_Init) {
    File file = File("", "", "", INT_B_I);
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
    EXPECT_EQ(VD->getType()->getName(), "i32");
    EXPECT_NE(VD->getInit(), nullptr);
    EXPECT_TRUE(VD->isMutable());
    EXPECT_FALSE(VD->isGlobal());

    IntegerLiteral *I = dynamic_cast<IntegerLiteral *>(VD->getInit());
    EXPECT_NE(I, nullptr);
    EXPECT_EQ(I->getValue(), 5);
    EXPECT_EQ(I->getType()->getName(), "i64");

    delete unit;
}

#define FP_B_I R"(test::() { mut x: f32 = 3.14; })"
TEST_F(ParseExprTest, FPLiteral_Basic_Init) {
    File file = File("", "", "", FP_B_I);
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

#define CHAR_B_I R"(test::() { mut x: char = 'a'; })"
TEST_F(ParseExprTest, CharLiteral_Basic_Init) {
    File file = File("", "", "", CHAR_B_I);
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

#define STRING_B_I R"(test::() { fix x: i32 = cast<i32> 5; })"
TEST_F(ParseExprTest, StringLiteral_Basic_Init) {
    File file = File("test.mdl", "/", "/test.mdl", STRING_B_I);
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
    EXPECT_EQ(VD->getType()->getName(), "i32");
    EXPECT_NE(VD->getInit(), nullptr);

    CastExpr *CE = dynamic_cast<CastExpr *>(VD->getInit());
    EXPECT_NE(CE, nullptr);
    EXPECT_EQ(CE->getType()->getName(), "i32");
    EXPECT_NE(CE->getExpr(), nullptr);

    IntegerLiteral *I = dynamic_cast<IntegerLiteral *>(CE->getExpr());
    EXPECT_NE(I, nullptr);
    EXPECT_EQ(I->getValue(), 5);
    EXPECT_EQ(I->getType()->getName(), "i64");

    delete unit;
}

#define CAST_B R"(test::() { fix x: char[7] = "hello\n"; })"
TEST_F(ParseExprTest, CastExpr_Basic) {
    File file = File("", "", "", CAST_B);
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
    EXPECT_EQ(VD->getType()->getName(), "char[7]");
    EXPECT_NE(VD->getInit(), nullptr);

    StringLiteral *SL = dynamic_cast<StringLiteral *>(VD->getInit());
    EXPECT_NE(SL, nullptr);
    EXPECT_EQ(SL->getValue(), "hello\n");
    EXPECT_EQ(SL->getType()->getName(), "char[7]");

    delete unit;
}

#define REF_BASIC R"(test::() { mut x: i64 = 0; ret x; })"
TEST_F(ParseExprTest, RefExpr_Basic) {
    File file = File("", "", "", REF_BASIC);
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

#define NIL_BASIC R"(test::() { fix x: i64* = nil; })"
TEST_F(ParseExprTest, Nil_Basic) {
    File file = File("", "", "", NIL_BASIC);
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
    EXPECT_EQ(VD->getType()->getName(), "i64*");
    EXPECT_NE(VD->getInit(), nullptr);
    EXPECT_FALSE(VD->isMutable());
    EXPECT_FALSE(VD->isGlobal());

    NilLiteral *N = dynamic_cast<NilLiteral *>(VD->getInit());
    EXPECT_NE(N, nullptr);
    EXPECT_EQ(N->getType()->getName(), "void*");

    delete unit;
}

#define BOOL_BASIC_TRUE R"(test::() { fix x: bool = true; })"
TEST_F(ParseExprTest, Bool_Basic_True) {
    File file = File("", "", "", BOOL_BASIC_TRUE);
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
    EXPECT_EQ(VD->getType()->getName(), "bool");
    EXPECT_NE(VD->getInit(), nullptr);
    EXPECT_FALSE(VD->isMutable());
    EXPECT_FALSE(VD->isGlobal());

    BoolLiteral *B = dynamic_cast<BoolLiteral *>(VD->getInit());
    EXPECT_NE(B, nullptr);
    EXPECT_EQ(B->getValue(), true);

    delete unit;
}

#define BOOL_BASIC_FALSE R"(test::() { fix x: bool = false; })"
TEST_F(ParseExprTest, Bool_Basic_False) {
    File file = File("", "", "", BOOL_BASIC_FALSE);
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
    EXPECT_EQ(VD->getType()->getName(), "bool");
    EXPECT_NE(VD->getInit(), nullptr);
    EXPECT_FALSE(VD->isMutable());
    EXPECT_FALSE(VD->isGlobal());

    BoolLiteral *B = dynamic_cast<BoolLiteral *>(VD->getInit());
    EXPECT_NE(B, nullptr);
    EXPECT_EQ(B->getValue(), false);

    delete unit;
}

#define PAREN_BASIC R"(test::() { fix x: bool = (true); })"
TEST_F(ParseExprTest, Paren_Basic) {
    File file = File("", "", "", PAREN_BASIC);
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
    EXPECT_EQ(VD->getType()->getName(), "bool");
    EXPECT_NE(VD->getInit(), nullptr);
    EXPECT_FALSE(VD->isMutable());
    EXPECT_FALSE(VD->isGlobal());

    ParenExpr *PE = dynamic_cast<ParenExpr *>(VD->getInit());
    EXPECT_NE(PE, nullptr);
    
    BoolLiteral *B = dynamic_cast<BoolLiteral *>(PE->getExpr());
    EXPECT_NE(B, nullptr);
    
    delete unit;
}

#define SIZEOF_BASIC R"(test::() { fix x: i32 = sizeof<i32>; })"
TEST_F(ParseExprTest, Sizeof_Basic) {
    File file = File("", "", "", SIZEOF_BASIC);
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
    EXPECT_EQ(VD->getType()->getName(), "i32");
    EXPECT_NE(VD->getInit(), nullptr);
    EXPECT_FALSE(VD->isMutable());
    EXPECT_FALSE(VD->isGlobal());

    SizeofExpr *SE = dynamic_cast<SizeofExpr *>(VD->getInit());
    EXPECT_NE(SE, nullptr);
    EXPECT_NE(SE->getTarget(), nullptr);
    EXPECT_EQ(SE->getTarget()->getName(), "i32");
    
    delete unit;
}

#define BINARY_ASSIGN R"(test::() { mut x = 0; x = 1; })"
TEST_F(ParseExprTest, Binary_Assign) {
    File file = File("", "", "", BINARY_ASSIGN);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::Assign);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define BINARY_ADD_ASSIGN R"(test::() { mut x = 0; x += 1; })"
TEST_F(ParseExprTest, Binary_Add_Assign) {
    File file = File("", "", "", BINARY_ADD_ASSIGN);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::Add_Assign);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define BINARY_SUB_ASSIGN R"(test::() { mut x = 0; x -= 1; })"
TEST_F(ParseExprTest, Binary_Sub_Assign) {
    File file = File("", "", "", BINARY_SUB_ASSIGN);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::Sub_Assign);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define BINARY_MUL_ASSIGN R"(test::() { mut x = 0; x *= 1; })"
TEST_F(ParseExprTest, Binary_Mul_Assign) {
    File file = File("", "", "", BINARY_MUL_ASSIGN);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::Mul_Assign);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define BINARY_DIV_ASSIGN R"(test::() { mut x = 0; x /= 1; })"
TEST_F(ParseExprTest, Binary_Div_Assign) {
    File file = File("", "", "", BINARY_DIV_ASSIGN);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::Div_Assign);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define BINARY_MOD_ASSIGN R"(test::() { mut x = 0; x %= 1; })"
TEST_F(ParseExprTest, Binary_Mod_Assign) {
    File file = File("", "", "", BINARY_MOD_ASSIGN);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::Mod_Assign);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define BINARY_AND_ASSIGN R"(test::() { mut x = 0; x &= 1; })"
TEST_F(ParseExprTest, Binary_And_Assign) {
    File file = File("", "", "", BINARY_AND_ASSIGN);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::And_Assign);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define BINARY_OR_ASSIGN R"(test::() { mut x = 0; x |= 1; })"
TEST_F(ParseExprTest, Binary_Or_Assign) {
    File file = File("", "", "", BINARY_OR_ASSIGN);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::Or_Assign);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define BINARY_XOR_ASSIGN R"(test::() { mut x = 0; x ^= 1; })"
TEST_F(ParseExprTest, Binary_Xor_Assign) {
    File file = File("", "", "", BINARY_XOR_ASSIGN);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::Xor_Assign);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define BINARY_LS_ASSIGN R"(test::() { mut x = 0; x <<= 1; })"
TEST_F(ParseExprTest, Binary_LS_Assign) {
    File file = File("", "", "", BINARY_LS_ASSIGN);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::LeftShift_Assign);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define BINARY_RS_ASSIGN R"(test::() { mut x = 0; x >>= 1; })"
TEST_F(ParseExprTest, Binary_RS_Assign) {
    File file = File("", "", "", BINARY_RS_ASSIGN);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::RightShift_Assign);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define BINARY_EQUALS R"(test::() { fix x: i32 = 0; x == 1; })"
TEST_F(ParseExprTest, Binary_Equals) {
    File file = File("", "", "", BINARY_EQUALS);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::Equals);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define BINARY_NEQUALS R"(test::() { fix x: i32 = 0; x != 1; })"
TEST_F(ParseExprTest, Binary_NEquals) {
    File file = File("", "", "", BINARY_NEQUALS);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::NEquals);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define BINARY_LESS R"(test::() { fix x: i32 = 0; x < 1; })"
TEST_F(ParseExprTest, Binary_LessThan) {
    File file = File("", "", "", BINARY_LESS);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::LessThan);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define BINARY_LESS_EQ R"(test::() { fix x: i32 = 0; x <= 1; })"
TEST_F(ParseExprTest, Binary_LessThanEquals) {
    File file = File("", "", "", BINARY_LESS_EQ);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::LessThanEquals);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define BINARY_GREATER R"(test::() { fix x: i32 = 0; x > 1; })"
TEST_F(ParseExprTest, Binary_GreaterThan) {
    File file = File("", "", "", BINARY_GREATER);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::GreaterThan);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define BINARY_GREATER_EQ R"(test::() { fix x: i32 = 0; x >= 1; })"
TEST_F(ParseExprTest, Binary_GreaterThanEquals) {
    File file = File("", "", "", BINARY_GREATER_EQ);
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
    
    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::GreaterThanEquals);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define BINARY_ADD R"(test::() { fix x: i32 = 0; x + 1; })"
TEST_F(ParseExprTest, Binary_Add) {
    File file = File("", "", "", BINARY_ADD);
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
    
    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::Add);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define BINARY_SUB R"(test::() { fix x: i32 = 0; x - 1; })"
TEST_F(ParseExprTest, Binary_Sub) {
    File file = File("", "", "", BINARY_SUB);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::Sub);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define BINARY_MUL R"(test::() { fix x: i32 = 0; x * 1; })"
TEST_F(ParseExprTest, Binary_Mul) {
    File file = File("", "", "", BINARY_MUL);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::Mul);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define BINARY_DIV R"(test::() { fix x: i32 = 0; x / 1; })"
TEST_F(ParseExprTest, Binary_Div) {
    File file = File("", "", "", BINARY_DIV);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::Div);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define BINARY_MOD R"(test::() { fix x: i32 = 0; x % 1; })"
TEST_F(ParseExprTest, Binary_Mod) {
    File file = File("", "", "", BINARY_MOD);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::Mod);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define BINARY_AND R"(test::() { fix x: i32 = 0; x & 1; })"
TEST_F(ParseExprTest, Binary_And) {
    File file = File("", "", "", BINARY_AND);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::Bitwise_And);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define BINARY_OR R"(test::() { fix x: i32 = 0; x | 1; })"
TEST_F(ParseExprTest, Binary_Or) {
    File file = File("", "", "", BINARY_OR);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::Bitwise_Or);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define BINARY_XOR R"(test::() { fix x: i32 = 0; x ^ 1; })"
TEST_F(ParseExprTest, Binary_Xor) {
    File file = File("", "", "", BINARY_XOR);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::Bitwise_Xor);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define BINARY_LS R"(test::() { fix x: i32 = 0; x << 1; })"
TEST_F(ParseExprTest, Binary_LS) {
    File file = File("", "", "", BINARY_LS);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::LeftShift);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define BINARY_RS R"(test::() { fix x: i32 = 0; x >> 1; })"
TEST_F(ParseExprTest, Binary_RS) {
    File file = File("", "", "", BINARY_RS);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::RightShift);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define BINARY_LOGIC_AND R"(test::() { fix x: i32 = 0; x && 1; })"
TEST_F(ParseExprTest, Binary_Logic_And) {
    File file = File("", "", "", BINARY_LOGIC_AND);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::Logic_And);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define BINARY_LOGIC_OR R"(test::() { fix x: i32 = 0; x || 1; })"
TEST_F(ParseExprTest, Binary_Logic_Or) {
    File file = File("", "", "", BINARY_LOGIC_OR);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::Logic_Or);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define BINARY_PRECEDENCE_BEDMAS1 R"(test::() { fix x: i32 = 0; x + 1 * 2; })"
TEST_F(ParseExprTest, Binary_Precedence_Bedmas_One) {
    File file = File("", "", "", BINARY_PRECEDENCE_BEDMAS1);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::Add);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);

    BinaryExpr *RHS = dynamic_cast<BinaryExpr *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);
    EXPECT_NE(RHS->getLHS(), nullptr);
    EXPECT_NE(RHS->getRHS(), nullptr);
    EXPECT_EQ(RHS->getKind(), BinaryExpr::Kind::Mul);

    IntegerLiteral *RHS_LHS = dynamic_cast<IntegerLiteral *>(RHS->getLHS());
    EXPECT_NE(RHS_LHS, nullptr);

    IntegerLiteral *RHS_RHS = dynamic_cast<IntegerLiteral *>(RHS->getRHS());
    EXPECT_NE(RHS_RHS, nullptr);

    delete unit;
}

#define BINARY_PRECEDENCE_BEDMAS2 R"(test::() { fix x: i32 = 0; x + 1 * 2 / 3; })"
TEST_F(ParseExprTest, Binary_Precedence_Bedmas_Two) {
    File file = File("", "", "", BINARY_PRECEDENCE_BEDMAS2);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *addExpr = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(addExpr, nullptr);
    EXPECT_EQ(addExpr->getKind(), BinaryExpr::Kind::Add);

    RefExpr *lhs = dynamic_cast<RefExpr *>(addExpr->getLHS());
    EXPECT_NE(lhs, nullptr);
    EXPECT_EQ(lhs->getName(), "x");

    BinaryExpr *divExpr = dynamic_cast<BinaryExpr *>(addExpr->getRHS());
    EXPECT_NE(divExpr, nullptr);
    EXPECT_EQ(divExpr->getKind(), BinaryExpr::Kind::Div);

    BinaryExpr *mulExpr = dynamic_cast<BinaryExpr *>(divExpr->getLHS());
    EXPECT_NE(mulExpr, nullptr);
    EXPECT_EQ(mulExpr->getKind(), BinaryExpr::Kind::Mul);

    IntegerLiteral *mulLHS = dynamic_cast<IntegerLiteral *>(mulExpr->getLHS());
    EXPECT_NE(mulLHS, nullptr);
    EXPECT_EQ(mulLHS->getValue(), 1);

    IntegerLiteral *mulRHS = dynamic_cast<IntegerLiteral *>(mulExpr->getRHS());
    EXPECT_NE(mulRHS, nullptr);
    EXPECT_EQ(mulRHS->getValue(), 2);

    IntegerLiteral *divRHS = dynamic_cast<IntegerLiteral *>(divExpr->getRHS());
    EXPECT_NE(divRHS, nullptr);
    EXPECT_EQ(divRHS->getValue(), 3);

    delete unit;
}

#define BINARY_PRECEDENCE_BEDMAS3 R"(test::() { fix x: i32 = 0; x + 1 * 2 / 3 % 4; })"
TEST_F(ParseExprTest, Binary_Precedence_Bedmas_Three) {
    File file = File("", "", "", BINARY_PRECEDENCE_BEDMAS3);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::Add);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);
    EXPECT_EQ(LHS->getName(), "x");

    BinaryExpr *RHS = dynamic_cast<BinaryExpr *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);
    EXPECT_EQ(RHS->getKind(), BinaryExpr::Kind::Mod);

    BinaryExpr *MOD_LHS = dynamic_cast<BinaryExpr *>(RHS->getLHS());
    EXPECT_NE(MOD_LHS, nullptr);
    EXPECT_EQ(MOD_LHS->getKind(), BinaryExpr::Kind::Div);

    BinaryExpr *DIV_LHS = dynamic_cast<BinaryExpr *>(MOD_LHS->getLHS());
    EXPECT_NE(DIV_LHS, nullptr);
    EXPECT_EQ(DIV_LHS->getKind(), BinaryExpr::Kind::Mul);

    IntegerLiteral *MUL_LHS = dynamic_cast<IntegerLiteral *>(DIV_LHS->getLHS());
    EXPECT_NE(MUL_LHS, nullptr);
    EXPECT_EQ(MUL_LHS->getValue(), 1);

    IntegerLiteral *MUL_RHS = dynamic_cast<IntegerLiteral *>(DIV_LHS->getRHS());
    EXPECT_NE(MUL_RHS, nullptr);
    EXPECT_EQ(MUL_RHS->getValue(), 2);

    IntegerLiteral *DIV_RHS = dynamic_cast<IntegerLiteral *>(MOD_LHS->getRHS());
    EXPECT_NE(DIV_RHS, nullptr);
    EXPECT_EQ(DIV_RHS->getValue(), 3);

    IntegerLiteral *MOD_RHS = dynamic_cast<IntegerLiteral *>(RHS->getRHS());
    EXPECT_NE(MOD_RHS, nullptr);
    EXPECT_EQ(MOD_RHS->getValue(), 4);

    delete unit;
}

#define BINARY_PRECEDENCE_BEDMAS4 R"(test::() { fix x: i32 = 0; x + 1 * 2 / 3 % 4 + 5; })"
TEST_F(ParseExprTest, Binary_Precedence_Bedmas_Four) {
    File file = File("", "", "", BINARY_PRECEDENCE_BEDMAS4);
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
    
    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::Add);
    EXPECT_NE(BS->getLHS(), nullptr);
    EXPECT_NE(BS->getRHS(), nullptr);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);
    EXPECT_EQ(LHS->getName(), "x");

    BinaryExpr *RHS = dynamic_cast<BinaryExpr *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);
    EXPECT_EQ(RHS->getKind(), BinaryExpr::Kind::Add);
    EXPECT_NE(RHS->getLHS(), nullptr);
    EXPECT_NE(RHS->getRHS(), nullptr);

    BinaryExpr *ADD_LHS = dynamic_cast<BinaryExpr *>(RHS->getLHS());
    EXPECT_NE(ADD_LHS, nullptr);
    EXPECT_EQ(ADD_LHS->getKind(), BinaryExpr::Kind::Mod);
    EXPECT_NE(ADD_LHS->getLHS(), nullptr);
    EXPECT_NE(ADD_LHS->getRHS(), nullptr);

    BinaryExpr *MOD_LHS = dynamic_cast<BinaryExpr *>(ADD_LHS->getLHS());
    EXPECT_NE(MOD_LHS, nullptr);
    EXPECT_EQ(MOD_LHS->getKind(), BinaryExpr::Kind::Div);
    EXPECT_NE(MOD_LHS->getLHS(), nullptr);
    EXPECT_NE(MOD_LHS->getRHS(), nullptr);

    BinaryExpr *DIV_LHS = dynamic_cast<BinaryExpr *>(MOD_LHS->getLHS());
    EXPECT_NE(DIV_LHS, nullptr);
    EXPECT_EQ(DIV_LHS->getKind(), BinaryExpr::Kind::Mul);
    EXPECT_NE(DIV_LHS->getLHS(), nullptr);
    EXPECT_NE(DIV_LHS->getRHS(), nullptr);

    IntegerLiteral *MUL_LHS = dynamic_cast<IntegerLiteral *>(DIV_LHS->getLHS());
    EXPECT_NE(MUL_LHS, nullptr);
    EXPECT_EQ(MUL_LHS->getValue(), 1);

    IntegerLiteral *MUL_RHS = dynamic_cast<IntegerLiteral *>(DIV_LHS->getRHS());
    EXPECT_NE(MUL_RHS, nullptr);
    EXPECT_EQ(MUL_RHS->getValue(), 2);

    IntegerLiteral *DIV_RHS = dynamic_cast<IntegerLiteral *>(MOD_LHS->getRHS());
    EXPECT_NE(DIV_RHS, nullptr);
    EXPECT_EQ(DIV_RHS->getValue(), 3);

    IntegerLiteral *MOD_RHS = dynamic_cast<IntegerLiteral *>(ADD_LHS->getRHS());
    EXPECT_NE(MOD_RHS, nullptr);
    EXPECT_EQ(MOD_RHS->getValue(), 4);

    IntegerLiteral *ADD_RHS = dynamic_cast<IntegerLiteral *>(RHS->getRHS());
    EXPECT_NE(ADD_RHS, nullptr);
    EXPECT_EQ(ADD_RHS->getValue(), 5);
    
    delete unit;
}

#define BINARY_PRECEDENCE_ASSIGN R"(test::() { fix x: i32 = 0; x = 1 + 2; })"
TEST_F(ParseExprTest, Binary_Precedence_Assignment_Basic) {
    File file = File("", "", "", BINARY_PRECEDENCE_ASSIGN);
    Lexer lexer = Lexer(file);
    TokenStream stream = lexer.unwrap();
    Parser parser = Parser(file, stream);
    TranslationUnit *unit = parser.get();

    EXPECT_EQ(unit->getDecls().size(), 1);
    FunctionDecl *FN = dynamic_cast<FunctionDecl *>(unit->getDecls()[0]);
    EXPECT_NE(FN, nullptr);

    CompoundStmt *CS = dynamic_cast<CompoundStmt *>(FN->getBody());
    EXPECT_NE(CS, nullptr);
    EXPECT_EQ(CS->getStmts().size(), 2);

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BS = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BS, nullptr);
    EXPECT_EQ(BS->getKind(), BinaryExpr::Kind::Assign);

    RefExpr *LHS = dynamic_cast<RefExpr *>(BS->getLHS());
    EXPECT_NE(LHS, nullptr);
    EXPECT_EQ(LHS->getName(), "x");

    BinaryExpr *RHS = dynamic_cast<BinaryExpr *>(BS->getRHS());
    EXPECT_NE(RHS, nullptr);
    EXPECT_EQ(RHS->getKind(), BinaryExpr::Kind::Add);

    IntegerLiteral *RHS_LHS = dynamic_cast<IntegerLiteral *>(RHS->getLHS());
    EXPECT_NE(RHS_LHS, nullptr);
    EXPECT_EQ(RHS_LHS->getValue(), 1);

    IntegerLiteral *RHS_RHS = dynamic_cast<IntegerLiteral *>(RHS->getRHS());
    EXPECT_NE(RHS_RHS, nullptr);
    EXPECT_EQ(RHS_RHS->getValue(), 2);

    delete unit;
}

#define UNARY_PREFIX_NEG R"(test::() { mut x: i64 = -5;})"
TEST_F(ParseExprTest, Unary_Prefix_Neg) {
    File file = File("", "", "", UNARY_PREFIX_NEG);
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
    EXPECT_NE(VD->getInit(), nullptr);

    UnaryExpr *US = dynamic_cast<UnaryExpr *>(VD->getInit());
    EXPECT_NE(US, nullptr);
    EXPECT_EQ(US->getKind(), UnaryExpr::Kind::Negate);
    EXPECT_EQ(US->isPostfix(), false);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(US->getExpr());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define UNARY_PREFIX_BITWISE_NOT R"(test::() { mut x: i64 = ~5; })"
TEST_F(ParseExprTest, Unary_Prefix_Bitwise_Not) {
    File file = File("", "", "", UNARY_PREFIX_BITWISE_NOT);
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
    EXPECT_NE(VD->getInit(), nullptr);

    UnaryExpr *US = dynamic_cast<UnaryExpr *>(VD->getInit());
    EXPECT_NE(US, nullptr);
    EXPECT_EQ(US->getKind(), UnaryExpr::Kind::Bitwise_Not);
    EXPECT_EQ(US->isPostfix(), false);

    IntegerLiteral *RHS = dynamic_cast<IntegerLiteral *>(US->getExpr());
    EXPECT_NE(RHS, nullptr);

    delete unit;
}

#define UNARY_PREFIX_LOGICAL_NOT R"(test::() { mut x: bool = true; mut y: bool = !x; })"
TEST_F(ParseExprTest, Unary_Prefix_Logical_Not) {
    File file = File("", "", "", UNARY_PREFIX_LOGICAL_NOT);
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
    EXPECT_NE(VD->getInit(), nullptr);

    DeclStmt *DS2 = dynamic_cast<DeclStmt *>(CS->getStmts()[1]);
    EXPECT_NE(DS2, nullptr);

    VarDecl *VD2 = dynamic_cast<VarDecl *>(DS2->getDecl());
    EXPECT_NE(VD2, nullptr);
    EXPECT_EQ(VD2->getName(), "y");
    EXPECT_NE(VD2->getInit(), nullptr);

    UnaryExpr *UN = dynamic_cast<UnaryExpr *>(VD2->getInit());
    EXPECT_NE(UN, nullptr);
    EXPECT_EQ(UN->getKind(), UnaryExpr::Kind::Logic_Not);
    EXPECT_EQ(UN->isPostfix(), false);

    delete unit;
}

#define UNARY_PREFIX_ADDRESS_OF R"(test::() { mut x: i64 = 5; mut y: i64* = &x; })"
TEST_F(ParseExprTest, Unary_Prefix_Address_Of) {
    File file = File("", "", "", UNARY_PREFIX_ADDRESS_OF);
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
    EXPECT_NE(VD->getInit(), nullptr);

    DeclStmt *DS2 = dynamic_cast<DeclStmt *>(CS->getStmts()[1]);
    EXPECT_NE(DS2, nullptr);

    VarDecl *VD2 = dynamic_cast<VarDecl *>(DS2->getDecl());
    EXPECT_NE(VD2, nullptr);
    EXPECT_EQ(VD2->getName(), "y");
    EXPECT_NE(VD2->getInit(), nullptr);

    UnaryExpr *UN = dynamic_cast<UnaryExpr *>(VD2->getInit());
    EXPECT_NE(UN, nullptr);
    EXPECT_EQ(UN->getKind(), UnaryExpr::Kind::Address_Of);
    EXPECT_EQ(UN->isPostfix(), false);

    delete unit;
}

#define UNARY_PREFIX_DEREFERENCE_LVALUE R"(test::() { mut x: i64* = nil; *x = 5; })"
TEST_F(ParseExprTest, Unary_Prefix_Dereference_LValue) {
    File file = File("", "", "", UNARY_PREFIX_DEREFERENCE_LVALUE);
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
    EXPECT_NE(VD->getInit(), nullptr);

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BE = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BE, nullptr);
    EXPECT_NE(BE->getLHS(), nullptr);
    EXPECT_NE(BE->getRHS(), nullptr);

    UnaryExpr *UN = dynamic_cast<UnaryExpr *>(BE->getLHS());
    EXPECT_NE(UN, nullptr);
    EXPECT_EQ(UN->getKind(), UnaryExpr::Kind::Dereference);
    EXPECT_EQ(UN->isPostfix(), false);

    delete unit;
}

#define UNARY_PREFIX_DEREFERENCE_RVALUE R"(test::() { mut x: i64* = nil; mut y: i64 = *x; })"
TEST_F(ParseExprTest, Unary_Prefix_Dereference_RValue) {
    File file = File("", "", "", UNARY_PREFIX_DEREFERENCE_RVALUE);
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
    EXPECT_NE(VD->getInit(), nullptr);

    DeclStmt *DS2 = dynamic_cast<DeclStmt *>(CS->getStmts()[1]);
    EXPECT_NE(DS2, nullptr);

    VarDecl *VD2 = dynamic_cast<VarDecl *>(DS2->getDecl());
    EXPECT_NE(VD2, nullptr);
    EXPECT_EQ(VD2->getName(), "y");
    EXPECT_NE(VD2->getInit(), nullptr);

    UnaryExpr *UN = dynamic_cast<UnaryExpr *>(VD2->getInit());
    EXPECT_NE(UN, nullptr);
    EXPECT_EQ(UN->getKind(), UnaryExpr::Kind::Dereference);
    EXPECT_EQ(UN->isPostfix(), false);

    delete unit;
}

#define UNARY_PREFIX_INCREMENT R"(test::() { mut x: i64 = 0; ++x; })"
TEST_F(ParseExprTest, Unary_Prefix_Increment) {
    File file = File("", "", "", UNARY_PREFIX_INCREMENT);
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
    EXPECT_NE(VD->getInit(), nullptr);

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    UnaryExpr *UN = dynamic_cast<UnaryExpr *>(ES->getExpr());
    EXPECT_NE(UN, nullptr);
    EXPECT_EQ(UN->getKind(), UnaryExpr::Kind::Increment);
    EXPECT_EQ(UN->isPostfix(), false);

    delete unit;
}

#define UNARY_PREFIX_DECREMENT R"(test::() { mut x: i64 = 0; --x; })"
TEST_F(ParseExprTest, Unary_Prefix_Decrement) {
    File file = File("", "", "", UNARY_PREFIX_DECREMENT);
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
    EXPECT_NE(VD->getInit(), nullptr);

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    UnaryExpr *UN = dynamic_cast<UnaryExpr *>(ES->getExpr());
    EXPECT_NE(UN, nullptr);
    EXPECT_EQ(UN->getKind(), UnaryExpr::Kind::Decrement);
    EXPECT_EQ(UN->isPostfix(), false);

    delete unit;
}

#define UNARY_POSTFIX_INCREMENT R"(test::() { mut x: i64 = 0; x++; })"
TEST_F(ParseExprTest, Unary_Postfix_Increment) {
    File file = File("", "", "", UNARY_POSTFIX_INCREMENT);
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
    EXPECT_NE(VD->getInit(), nullptr);

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    UnaryExpr *UN = dynamic_cast<UnaryExpr *>(ES->getExpr());
    EXPECT_NE(UN, nullptr);
    EXPECT_EQ(UN->getKind(), UnaryExpr::Kind::Increment);
    EXPECT_EQ(UN->isPostfix(), true);

    delete unit;
}

#define UNARY_POSTFIX_DECREMENT R"(test::() { mut x: i64 = 0; x--; })"
TEST_F(ParseExprTest, Unary_Postfix_Decrement) {
    File file = File("", "", "", UNARY_POSTFIX_DECREMENT);
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
    EXPECT_NE(VD->getInit(), nullptr);

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    UnaryExpr *UN = dynamic_cast<UnaryExpr *>(ES->getExpr());
    EXPECT_NE(UN, nullptr);
    EXPECT_EQ(UN->getKind(), UnaryExpr::Kind::Decrement);
    EXPECT_EQ(UN->isPostfix(), true);

    delete unit;
}

#define UNARY_PREFIX_DEREF_POSTFIX_INC R"(test::() { mut x: i64* = nil; *x++ = 5; })"
TEST_F(ParseExprTest, Unary_Prefix_Deref_Postfix_Inc) {
    File file = File("", "", "", UNARY_PREFIX_DEREF_POSTFIX_INC);
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

    ExprStmt *ES = dynamic_cast<ExprStmt *>(CS->getStmts()[1]);
    EXPECT_NE(ES, nullptr);

    BinaryExpr *BE = dynamic_cast<BinaryExpr *>(ES->getExpr());
    EXPECT_NE(BE, nullptr);
    EXPECT_NE(BE->getLHS(), nullptr);
    EXPECT_NE(BE->getRHS(), nullptr);

    UnaryExpr *US = dynamic_cast<UnaryExpr *>(BE->getLHS());
    EXPECT_NE(US, nullptr);
    EXPECT_EQ(US->getKind(), UnaryExpr::Kind::Dereference);
    EXPECT_EQ(US->isPostfix(), false);

    UnaryExpr *US2 = dynamic_cast<UnaryExpr *>(US->getExpr());
    EXPECT_NE(US2, nullptr);
    EXPECT_EQ(US2->getKind(), UnaryExpr::Kind::Increment);
    EXPECT_EQ(US2->isPostfix(), true);

    RefExpr *RE = dynamic_cast<RefExpr *>(US2->getExpr());
    EXPECT_NE(RE, nullptr);

    delete unit;
}

#define ARRAY_BASIC R"(test::() { mut x: i64[3] = [ 1, 2, 3 ]; })"
TEST_F(ParseExprTest, Array_Basic) {
    File file = File("", "", "", ARRAY_BASIC);
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
    EXPECT_NE(VD->getInit(), nullptr);

    ArrayExpr *AL = dynamic_cast<ArrayExpr *>(VD->getInit());
    EXPECT_NE(AL, nullptr);
    EXPECT_EQ(AL->getElements().size(), 3);

    IntegerLiteral *IL1 = dynamic_cast<IntegerLiteral *>(AL->getElements()[0]);
    EXPECT_NE(IL1, nullptr);
    EXPECT_EQ(IL1->getValue(), 1);

    IntegerLiteral *IL2 = dynamic_cast<IntegerLiteral *>(AL->getElements()[1]);
    EXPECT_NE(IL2, nullptr);
    EXPECT_EQ(IL2->getValue(), 2);

    IntegerLiteral *IL3 = dynamic_cast<IntegerLiteral *>(AL->getElements()[2]);
    EXPECT_NE(IL3, nullptr);
    EXPECT_EQ(IL3->getValue(), 3);

    delete unit;
}

} // namespace test

} // namespace meddle
