  .globl _start
.data
data_block:
.byte 0xCC, 0xFF, 0xAA, 0xBB
.half 0xDEAD, 0xBEEF
.word 0xCAFEF00D
.text
_start:
    la x5, data_block 
    
    lb x10, 1(x5)       
    
    lh x11, 2(x5)       
    
    lw x12, 8(x5)       
    
    lbu x13, 1(x5)      
    
    lhu x14, 2(x5)      
    
    li a7, 93
    ecall
    