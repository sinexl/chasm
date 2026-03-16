set architecture mips
target remote :3377
restore output.bin binary 0x80000000
set $pc = 0x80000000