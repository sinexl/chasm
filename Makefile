all: chasm as
chasm: chasm.cpp
	g++ -ggdb -Wall -Wextra chasm.cpp -o chasm
clean:
	rm chasm

as: temp
	mips-linux-gnu-as main.asm -o ./temp/gas.o
	mips-linux-gnu-objcopy -O binary ./temp/gas.o gas.bin
	# mv main.obj gas.bin

temp:
	mkdir -p temp/
