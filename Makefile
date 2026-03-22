CXX := g++
CXXFLAGS := -ggdb -Wall -Wextra -Wswitch-enum

SRC := $(wildcard src/*.cpp)
HEADERS := $(wildcard src/*.hpp)

OBJS := $(SRC:src/%.cpp=temp/%.o)

all: chasm as

chasm: $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

temp/%.o: src/%.cpp $(HEADERS) | temp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f chasm gas.bin
	rm -rf temp

as: temp
	mips-linux-gnu-as ./dev/main.asm -o temp/gas.o
	mips-linux-gnu-objcopy -O binary -j .text temp/gas.o ./dev/gas.bin

temp:
	mkdir -p temp

.PHONY: all clean as
