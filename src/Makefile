CXX=g++
CXXFLAGS=-Wall -Wextra -pedantic
TARGET=munchkin

SOURCES=main.cpp bitboard.cpp munchkin.cpp position.cpp bittricks.cpp
OBJECTS =$(SOURCES:.cpp=.o)
DEPEND=$(OBJECTS:.o=.h)


all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.c $(DEPEND)
	g++ -c -o $@ $<

clean:
	rm -rf $(OBJECTS) $(TARGET)
