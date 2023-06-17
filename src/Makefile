CXX=g++
CXXFLAGS=-std=c++11 -Wall -Wextra -pedantic -O2
INCLUDE=-Iinclude

EXEC_PATH := $(addprefix ../,$(EXEC))

SRC := $(wildcard lib/**/*.cpp)
OBJS := $(SRC:%.cpp=%.o)
DEPS := $(SRC:%.cpp=%.d)

all: $(EXEC_PATH)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -MMD $< -o $@

$(EXEC_PATH): *.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $^ -o $@

.PHONY: clean
clean:
	$(RM) $(OBJS) $(DEPS) $(EXEC_PATH)

-include $(DEPS)
