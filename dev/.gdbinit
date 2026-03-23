set architecture mips
set endian big
target remote :3377
restore output.bin binary 0x80000000
set $pc = 0x80000000

define reload
  monitor system_reset
  restore output.bin binary 0x80000000
  set $pc = 0x80000000
end
