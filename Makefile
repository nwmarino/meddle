CXX := clang++
CXXFLAGS := -std=c++20 -g -O0 -stdlib=libstdc++ -Icompiler -I$(BOOST_DIR) -I$(GTEST_DIR)
LDFLAGS := -lstdc++ -lm

MAIN := compiler/meddle.cpp
MAIN_OBJ := $(MAIN:.cpp=.o)
SRC := $(filter-out $(MAIN), $(shell find compiler -name "*.cpp"))
OBJ := $(SRC:.cpp=.o)

TEST_SRC := $(wildcard test/*.cpp)
TEST_OBJ := $(TEST_SRC:.cpp=.o)

TARGET := meddle
TEST_TARGET := test_meddle

GTEST_LIB := -L/usr/lib64 -lgtest -pthread
BOOST_LIB := -L/usr/lib64 -lboost_filesystem

all: $(TARGET)

$(TARGET): $(OBJ) $(MAIN_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(BOOST_LIB)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TEST_TARGET)

$(TEST_TARGET): $(OBJ) $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(BOOST_LIB) $(GTEST_LIB)

clean:
	rm -f $(OBJ) $(TEST_OBJ) $(TARGET) $(TEST_TARGET)
