  .globl _start
.text
_start:
    lui x10, 0xABCD0
    
    auipc x11, 0x10000 
    
    jal ra, subrutine_a 

    addi x12, x12, 100
    
    li x5, 0x20000
    jalr x0, x5, 0
    
    li a7, 93
    ecall
    
subrutine_a:
    addi x12, x12, 5
    
    jalr x0, ra, 0
