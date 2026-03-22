main:
    addi $s0, $zero, 0
    addi $s1, $zero, 0

.cond:
    slti $t0, $s0, 10
    beq  $t0, $zero, .exit

    add $s1, $s1, $s0

    addi $s0, $s0, 1
    beq $zero, $zero, .cond
.exit:
    jr $ra
