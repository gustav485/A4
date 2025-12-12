  .globl _start
.text
_start:
    li x5, 10
    li x6, 5
    li x7, 10
    li x8, -10
    
    li x10, 0 
    
    beq x5, x7, label_taken
    
    addi x10, x10, 1
    j continue_test
    
label_taken:
    addi x10, x10, 10
    
continue_test:
    bne x5, x7, label_not_taken
    
    addi x10, x10, 100
    j next_test
    
label_not_taken:
    addi x10, x10, 1
    
next_test:
    blt x8, x5, final_check 
    
    addi x10, x10, 1
    
final_check:
    bge x8, x5, final_end 
    
    addi x10, x10, 1000
    
final_end:
    li a7, 93
    ecall
    