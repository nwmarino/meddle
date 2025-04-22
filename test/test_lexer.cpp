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

    EXPECT_EQ(stream.getTokens().size(), 2);
    EXPECT_EQ(stream.get(0)->kind, TokenKind::Identifier);
    EXPECT_EQ(stream.get(0)->value, "test");
    EXPECT_EQ(stream.get(1)->kind, TokenKind::Eof);
}

TEST_F(LexerTest, LexCharLiteral) {
    Lexer lexer = Lexer(File("", "", "", "'a'"));
    TokenStream stream = lexer.unwrap();

    EXPECT_EQ(stream.getTokens().size(), 2);
    EXPECT_EQ(stream.get(0)->kind, TokenKind::Literal);
    EXPECT_EQ(stream.get(0)->literal, LiteralKind::Character);
    EXPECT_EQ(stream.get(0)->value, "a");
    EXPECT_EQ(stream.get(1)->kind, TokenKind::Eof);
}

TEST_F(LexerTest, LexCharLiteralWithEscape) {
    Lexer lexer = Lexer(File("", "", "", "'\n'"));
    TokenStream stream = lexer.unwrap();

    EXPECT_EQ(stream.getTokens().size(), 2);
    EXPECT_EQ(stream.get(0)->kind, TokenKind::Literal);
    EXPECT_EQ(stream.get(0)->literal, LiteralKind::Character);
    EXPECT_EQ(stream.get(0)->value, "\n");
    EXPECT_EQ(stream.get(1)->kind, TokenKind::Eof);
}

TEST_F(LexerTest, LexSingleQuote) {
    Lexer lexer = Lexer(File("", "", "", "'ab"));
    TokenStream stream = lexer.unwrap();

    EXPECT_EQ(stream.getTokens().size(), 3);
    EXPECT_EQ(stream.get(0)->kind, TokenKind::Quote);
    EXPECT_EQ(stream.get(1)->kind, TokenKind::Identifier);
    EXPECT_EQ(stream.get(1)->value, "ab");
    EXPECT_EQ(stream.get(2)->kind, TokenKind::Eof);
}

TEST_F(LexerTest, LexStringLiteral) {
    Lexer lexer = Lexer(File("", "", "", "\"test\""));
    TokenStream stream = lexer.unwrap();

    EXPECT_EQ(stream.getTokens().size(), 2);
    EXPECT_EQ(stream.get(0)->kind, TokenKind::Literal);
    EXPECT_EQ(stream.get(0)->literal, LiteralKind::String);
    EXPECT_EQ(stream.get(0)->value, "test");
    EXPECT_EQ(stream.get(1)->kind, TokenKind::Eof);
}

TEST_F(LexerTest, LexStringLiteralWithEscape) {
    Lexer lexer = Lexer(File("", "", "", "\"test\n\""));
    TokenStream stream = lexer.unwrap();

    EXPECT_EQ(stream.getTokens().size(), 2);
    EXPECT_EQ(stream.get(0)->kind, TokenKind::Literal);
    EXPECT_EQ(stream.get(0)->literal, LiteralKind::String);
    EXPECT_EQ(stream.get(0)->value, "test\n");
    EXPECT_EQ(stream.get(1)->kind, TokenKind::Eof);
}

TEST_F(LexerTest, LexIntLiteral) {
    Lexer lexer = Lexer(File("", "", "", "123"));
    TokenStream stream = lexer.unwrap();

    EXPECT_EQ(stream.getTokens().size(), 2);
    EXPECT_EQ(stream.get(0)->kind, TokenKind::Literal);
    EXPECT_EQ(stream.get(0)->literal, LiteralKind::Integer);
    EXPECT_EQ(stream.get(0)->value, "123");
    EXPECT_EQ(stream.get(1)->kind, TokenKind::Eof);
}

TEST_F(LexerTest, LexIntLiteralEndsWithDot) {
    Lexer lexer = Lexer(File("", "", "", "123."));
    TokenStream stream = lexer.unwrap();

    EXPECT_EQ(stream.getTokens().size(), 3);
    EXPECT_EQ(stream.get(0)->kind, TokenKind::Literal);
    EXPECT_EQ(stream.get(0)->literal, LiteralKind::Integer);
    EXPECT_EQ(stream.get(0)->value, "123");
    EXPECT_EQ(stream.get(1)->kind, TokenKind::Dot);
    EXPECT_EQ(stream.get(2)->kind, TokenKind::Eof);
}

TEST_F(LexerTest, LexFPLiteral) {
    Lexer lexer = Lexer(File("", "", "", "123.456"));
    TokenStream stream = lexer.unwrap();

    EXPECT_EQ(stream.getTokens().size(), 2);
    EXPECT_EQ(stream.get(0)->kind, TokenKind::Literal);
    EXPECT_EQ(stream.get(0)->literal, LiteralKind::Float);
    EXPECT_EQ(stream.get(0)->value, "123.456");
    EXPECT_EQ(stream.get(1)->kind, TokenKind::Eof);
}

TEST_F(LexerTest, LexNumericMultipleDots) {
    Lexer lexer = Lexer(File("", "", "", "123.456.789"));
    TokenStream stream = lexer.unwrap();

    EXPECT_EQ(stream.getTokens().size(), 4);
    EXPECT_EQ(stream.get(0)->kind, TokenKind::Literal);
    EXPECT_EQ(stream.get(0)->literal, LiteralKind::Float);
    EXPECT_EQ(stream.get(0)->value, "123.456");
    EXPECT_EQ(stream.get(1)->kind, TokenKind::Dot);
    EXPECT_EQ(stream.get(2)->kind, TokenKind::Literal);
    EXPECT_EQ(stream.get(2)->literal, LiteralKind::Integer);
    EXPECT_EQ(stream.get(2)->value, "789");
    EXPECT_EQ(stream.get(3)->kind, TokenKind::Eof);
}

TEST_F(LexerTest, LexEmptyFunctionDecl) {
    Lexer lexer = Lexer(File("", "", "", "test::();"));
    TokenStream stream = lexer.unwrap();

    EXPECT_EQ(stream.getTokens().size(), 6);
    EXPECT_EQ(stream.get(0)->kind, TokenKind::Identifier);
    EXPECT_EQ(stream.get(0)->value, "test");
    EXPECT_EQ(stream.get(1)->kind, TokenKind::Path);
    EXPECT_EQ(stream.get(2)->kind, TokenKind::SetParen);
    EXPECT_EQ(stream.get(3)->kind, TokenKind::EndParen);
    EXPECT_EQ(stream.get(4)->kind, TokenKind::Semi);
    EXPECT_EQ(stream.get(5)->kind, TokenKind::Eof);
}

TEST_F(LexerTest, LexFunctionReturnZero) {
    Lexer lexer = Lexer(File("", "", "", "test::(){ret 0;}"));
    TokenStream stream = lexer.unwrap();

    EXPECT_EQ(stream.getTokens().size(), 10);
    EXPECT_EQ(stream.get(0)->kind, TokenKind::Identifier);
    EXPECT_EQ(stream.get(0)->value, "test");
    EXPECT_EQ(stream.get(1)->kind, TokenKind::Path);
    EXPECT_EQ(stream.get(2)->kind, TokenKind::SetParen);
    EXPECT_EQ(stream.get(3)->kind, TokenKind::EndParen);
    EXPECT_EQ(stream.get(4)->kind, TokenKind::SetBrace);
    EXPECT_EQ(stream.get(5)->kind, TokenKind::Identifier);
    EXPECT_EQ(stream.get(5)->value, "ret");
    EXPECT_EQ(stream.get(6)->kind, TokenKind::Literal);
    EXPECT_EQ(stream.get(6)->literal, LiteralKind::Integer);
    EXPECT_EQ(stream.get(6)->value, "0");
    EXPECT_EQ(stream.get(7)->kind, TokenKind::Semi);
    EXPECT_EQ(stream.get(8)->kind, TokenKind::EndBrace);
    EXPECT_EQ(stream.get(9)->kind, TokenKind::Eof);
}

TEST_F(LexerTest, LexLocalVariableNoInit) {
    Lexer lexer = Lexer(File("", "", "", "test::(){mut x: i64;}"));
    TokenStream stream = lexer.unwrap();

    EXPECT_EQ(stream.getTokens().size(), 12);
    EXPECT_EQ(stream.get(0)->kind, TokenKind::Identifier);
    EXPECT_EQ(stream.get(0)->value, "test");
    EXPECT_EQ(stream.get(1)->kind, TokenKind::Path);
    EXPECT_EQ(stream.get(2)->kind, TokenKind::SetParen);
    EXPECT_EQ(stream.get(3)->kind, TokenKind::EndParen);
    EXPECT_EQ(stream.get(4)->kind, TokenKind::SetBrace);
    EXPECT_EQ(stream.get(5)->kind, TokenKind::Identifier);
    EXPECT_EQ(stream.get(5)->value, "mut");
    EXPECT_EQ(stream.get(6)->kind, TokenKind::Identifier);
    EXPECT_EQ(stream.get(6)->value, "x");
    EXPECT_EQ(stream.get(7)->kind, TokenKind::Colon);
    EXPECT_EQ(stream.get(8)->kind, TokenKind::Identifier);
    EXPECT_EQ(stream.get(8)->value, "i64");
    EXPECT_EQ(stream.get(9)->kind, TokenKind::Semi);
    EXPECT_EQ(stream.get(10)->kind, TokenKind::EndBrace);
    EXPECT_EQ(stream.get(11)->kind, TokenKind::Eof);
}

TEST_F(LexerTest, LexLocalVariableInit) {
    Lexer lexer = Lexer(File("", "", "", "test::(){mut x: i64 = 0;}"));
    TokenStream stream = lexer.unwrap();

    EXPECT_EQ(stream.getTokens().size(), 14);
    EXPECT_EQ(stream.get(0)->kind, TokenKind::Identifier);
    EXPECT_EQ(stream.get(0)->value, "test");
    EXPECT_EQ(stream.get(1)->kind, TokenKind::Path);
    EXPECT_EQ(stream.get(2)->kind, TokenKind::SetParen);
    EXPECT_EQ(stream.get(3)->kind, TokenKind::EndParen);
    EXPECT_EQ(stream.get(4)->kind, TokenKind::SetBrace);
    EXPECT_EQ(stream.get(5)->kind, TokenKind::Identifier);
    EXPECT_EQ(stream.get(5)->value, "mut");
    EXPECT_EQ(stream.get(6)->kind, TokenKind::Identifier);
    EXPECT_EQ(stream.get(6)->value, "x");
    EXPECT_EQ(stream.get(7)->kind, TokenKind::Colon);
    EXPECT_EQ(stream.get(8)->kind, TokenKind::Identifier);
    EXPECT_EQ(stream.get(8)->value, "i64");
    EXPECT_EQ(stream.get(9)->kind, TokenKind::Equals);
    EXPECT_EQ(stream.get(10)->kind, TokenKind::Literal);
    EXPECT_EQ(stream.get(10)->literal, LiteralKind::Integer);
    EXPECT_EQ(stream.get(10)->value, "0");
    EXPECT_EQ(stream.get(11)->kind, TokenKind::Semi);
    EXPECT_EQ(stream.get(12)->kind, TokenKind::EndBrace);
    EXPECT_EQ(stream.get(13)->kind, TokenKind::Eof);
}

} // namespace test

} // namespace meddle

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
