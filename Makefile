CXX = g++
CXXFLAGS = -std=c++17

TARGET = SimCity

SRCS = main.cpp commercial.cpp config.cpp growth.cpp industrial.cpp region.cpp residential.cpp goods.cpp

OBJS = $(SRCS:.cpp=.o)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	del /Q *.o $(TARGET).exe 2>nul

# Measure line/branch/function coverage (see tests/coverage/run_coverage.sh).
.PHONY: coverage

coverage:
	tests/coverage/run_coverage.sh
