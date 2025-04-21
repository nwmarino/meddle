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
    EXPECT_EQ(0, 0);
}

TEST_F(LexerTest, LexCharLiteral) {
    
}

TEST_F(LexerTest, LexStringLiteral) {

}

TEST_F(LexerTest, LexIntLiteral) {

}

TEST_F(LexerTest, LexFPLiteral) {

}

} // namespace test

} // namespace meddle

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
