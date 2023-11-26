CXX=g++
CXXFLAGS=-Wall -Wextra -pedantic -O2 -ffast-math
TARGET=munchkin

SOURCES=main.cpp bitboard.cpp munchkin.cpp position.cpp bittricks.cpp movegen.cpp moveupd.cpp misc.cpp testing.cpp search.cpp evaluation.cpp
OBJECTS =$(SOURCES:.cpp=.o)
DEPEND=$(OBJECTS:.o=.h)


all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.c $(DEPEND)
	g++ -c -o $@ $<

clean:
	rm -rf $(OBJECTS) $(TARGET)
