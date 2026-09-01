CXX = g++
CXXFLAGS = -std=c++17 -g -Wall -Wextra -Wpedantic
TARGET = polynomial_tests
SOURCES = cpp_code/Polynomial.cpp cpp_code/Complex.cpp cpp_code/Rational.cpp tests/polynomial_invariants.cpp
HEADERS = cpp_code/Polynomial.h cpp_code/Complex.h cpp_code/Rational.h

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -Icpp_code $(SOURCES) -o $(TARGET)

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
