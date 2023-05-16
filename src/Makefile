CXX=g++
CXXFLAGS=-std=c++20 -Wall -Wextra -pedantic -O2
INCLUDE=-Iinclude

EXEC := main

SRC := $(wildcard lib/**/*.cpp)
OBJS := $(SRC:%.cpp=%.o)
DEPS := $(SRC:%.cpp=%.d)

all: $(EXEC)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -MMD $< -o $@

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $^ -o $@

.PHONY: clean
clean:
	$(RM) $(OBJS) $(DEPS) $(EXEC)

-include $(DEPS)
