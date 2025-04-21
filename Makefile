CXX := clang
CXXFLAGS := -Wall -Wextra -std=c++20 -g -O0 -stdlib=libstdc++
CXXTESTFLAGS := -Wall -Wextra -std=c++20 -stdlib=libstdc++ -O0
LDFLAGS := -lstdc++

MAIN := compiler/meddle.cpp
SRC := $(filter-out $(MAIN), $(wildcard compiler/*.cpp))
TEST_SRC := $(wildcard test/*.cpp)

TARGET := meddle
TEST_TARGET := test_meddle

GTEST_DIR := /usr/include/gtest
GTEST_LIB := -L/usr/lib64 -lgtest -pthread

all: $(TARGET)

$(TARGET): $(SRC) $(MAIN)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(MAIN) $(LDFLAGS)

test: $(TEST_TARGET)

$(TEST_TARGET): $(SRC) $(TEST_SRC)
	$(CXX) $(CXXTESTFLAGS) -o $(TEST_TARGET) $(SRC) $(TEST_SRC) $(LDFLAGS) -I$(GTEST_DIR)/include $(GTEST_LIB)

clean:
	rm -f $(TARGET) $(TEST_TARGET)

.PHONY: all clean test
