  .globl _start
_start:
    li x5, 100
    li x6, 3
    li x7, -5
    li x8, 25
    add x10, x5, x6
    sub x11, x5, x6
    mul x12, x5, x6
    sll x13, x6, x5
    slt x14, x7, x5
    sltu x15, x5, x7
    xor x16, x5, x6
    and x17, x5, x6
    srl x18, x5, x6
    sra x19, x7, x6
    or x20, x5, x6
    rem x21, x8, x6
    li a7, 93
    ecall
