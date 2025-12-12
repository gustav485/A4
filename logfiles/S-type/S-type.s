  .globl _start
.data
mem_target:
.zero 16
.text
_start:
    la x5, mem_target 
    li x6, 0x12345678
    li x7, 0x42
    li x8, 0xBBCC
    
    sb x7, 0(x5)        
    
    sh x8, 4(x5)        
    
    sw x6, 8(x5)        
    
    li a7, 93
    ecall
    