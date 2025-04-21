#include "../compiler/lexer/lexer.h"

#include <gtest/gtest.h>

namespace meddle {

namespace test {

class LexerTest : public ::testing::Test {
protected:
    void SetUp() override {
        
    }

    void TearDown() override {

    }
};

TEST_F(LexerTest, LexIdentifer) {
    Lexer lexer = Lexer(File("", "", "", "test"));
    TokenStream stream = lexer.unwrap();

    EXPECT_EQ(stream.getTokens().size(), 1);
    EXPECT_EQ(stream.get(0)->kind, TokenKind::Identifier);
    EXPECT_EQ(stream.get(0)->value, "test");
}

TEST_F(LexerTest, LexCharLiteral) {
    Lexer lexer = Lexer(File("", "", "", "'a'"));
    TokenStream stream = lexer.unwrap();

    EXPECT_EQ(stream.getTokens().size(), 1);
    EXPECT_EQ(stream.get(0)->kind, TokenKind::Literal);
    EXPECT_EQ(stream.get(0)->literal, LiteralKind::Character);
    EXPECT_EQ(stream.get(0)->value, "a");
}

TEST_F(LexerTest, LexCharLiteralWithEscape) {
    Lexer lexer = Lexer(File("", "", "", "'\n'"));
    TokenStream stream = lexer.unwrap();

    EXPECT_EQ(stream.getTokens().size(), 1);
    EXPECT_EQ(stream.get(0)->kind, TokenKind::Literal);
    EXPECT_EQ(stream.get(0)->literal, LiteralKind::Character);
    EXPECT_EQ(stream.get(0)->value, "\n");
}

TEST_F(LexerTest, LexSingleQuote) {
    Lexer lexer = Lexer(File("", "", "", "'ab"));
    TokenStream stream = lexer.unwrap();

    EXPECT_EQ(stream.getTokens().size(), 2);
    EXPECT_EQ(stream.get(0)->kind, TokenKind::Quote);
    EXPECT_EQ(stream.get(1)->kind, TokenKind::Identifier);
    EXPECT_EQ(stream.get(1)->value, "ab");
}

TEST_F(LexerTest, LexStringLiteral) {
    Lexer lexer = Lexer(File("", "", "", "\"test\""));
    TokenStream stream = lexer.unwrap();

    EXPECT_EQ(stream.getTokens().size(), 1);
    EXPECT_EQ(stream.get(0)->kind, TokenKind::Literal);
    EXPECT_EQ(stream.get(0)->literal, LiteralKind::String);
    EXPECT_EQ(stream.get(0)->value, "test");
}

TEST_F(LexerTest, LexStringLiteralWithEscape) {
    Lexer lexer = Lexer(File("", "", "", "\"test\n\""));
    TokenStream stream = lexer.unwrap();

    EXPECT_EQ(stream.getTokens().size(), 1);
    EXPECT_EQ(stream.get(0)->kind, TokenKind::Literal);
    EXPECT_EQ(stream.get(0)->literal, LiteralKind::String);
    EXPECT_EQ(stream.get(0)->value, "test\n");
}

TEST_F(LexerTest, LexIntLiteral) {
    Lexer lexer = Lexer(File("", "", "", "123"));
    TokenStream stream = lexer.unwrap();

    EXPECT_EQ(stream.getTokens().size(), 1);
    EXPECT_EQ(stream.get(0)->kind, TokenKind::Literal);
    EXPECT_EQ(stream.get(0)->literal, LiteralKind::Integer);
    EXPECT_EQ(stream.get(0)->value, "123");
}

TEST_F(LexerTest, LexIntLiteralEndsWithDot) {
    Lexer lexer = Lexer(File("", "", "", "123."));
    TokenStream stream = lexer.unwrap();

    EXPECT_EQ(stream.getTokens().size(), 2);
    EXPECT_EQ(stream.get(0)->kind, TokenKind::Literal);
    EXPECT_EQ(stream.get(0)->literal, LiteralKind::Integer);
    EXPECT_EQ(stream.get(0)->value, "123");

    EXPECT_EQ(stream.get(1)->kind, TokenKind::Dot);
}

TEST_F(LexerTest, LexFPLiteral) {
    Lexer lexer = Lexer(File("", "", "", "123.456"));
    TokenStream stream = lexer.unwrap();

    EXPECT_EQ(stream.getTokens().size(), 1);
    EXPECT_EQ(stream.get(0)->kind, TokenKind::Literal);
    EXPECT_EQ(stream.get(0)->literal, LiteralKind::Float);
    EXPECT_EQ(stream.get(0)->value, "123.456");
}

TEST_F(LexerTest, LexNumericMultipleDots) {
    Lexer lexer = Lexer(File("", "", "", "123.456.789"));
    TokenStream stream = lexer.unwrap();

    EXPECT_EQ(stream.getTokens().size(), 3);
    EXPECT_EQ(stream.get(0)->kind, TokenKind::Literal);
    EXPECT_EQ(stream.get(0)->literal, LiteralKind::Float);
    EXPECT_EQ(stream.get(0)->value, "123.456");

    EXPECT_EQ(stream.get(1)->kind, TokenKind::Dot);

    EXPECT_EQ(stream.get(2)->kind, TokenKind::Literal);
    EXPECT_EQ(stream.get(2)->literal, LiteralKind::Integer);
    EXPECT_EQ(stream.get(2)->value, "789");
}

} // namespace test

} // namespace meddle

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
