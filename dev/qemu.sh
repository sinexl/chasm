# https://www.qemu.org/docs/master/system/target-mips.html#mips-system-emulator
qemu-system-mips \
  -M malta \
  -S -gdb tcp::3377 \
  -cpu 24Kf \
  -device loader,file=output.bin,addr=0x80000000  \
  -nographic 
 # -d in_asm,cpu \