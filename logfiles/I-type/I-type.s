  .globl _start
.text
_start:
    li x5, 200
    li x6, -100
    
    addi x10, x5, 42
    
    slli x11, x5, 3
    
    slti x12, x6, 0
    sltiu x13, x5, 100
    
    srli x15, x5, 4
    srai x16, x6, 2
    
    ori x17, x5, 0x10
    andi x18, x5, 0xFF
    
    li a7, 93
    ecall
