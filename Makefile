CXX := g++
CXXFLAGS := -ggdb -Wall -Wextra -Wswitch-enum

SRC := $(wildcard *.cpp)
HEADERS := $(wildcard *.hpp)

all: chasm

chasm: $(SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRC) -o $@

clean:
	rm -f chasm gas.bin
	rm -rf temp

as: temp
	mips-linux-gnu-as main.asm -o temp/gas.o
	mips-linux-gnu-objcopy -O binary temp/gas.o gas.bin

temp:
	mkdir -p temp
