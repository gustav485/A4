#include <memory.h>
#include <simulate.h>
#include <stdio.h>
#include <stdbool.h>

struct Stat simulate(struct memory *mem, int start_addr, FILE *log_file, struct symbols* symbols) {
    int32_t regs[32] = {0};
    u_int32_t program_counter = start_addr;
    long int instruction_count = 0;
    bool done = false;

    while (!done) {
        u_int32_t instruction = memory_rd_w(mem, program_counter);
        u_int32_t opcode = instruction & 0x7F;
        u_int32_t rd     = (instruction >> 7) & 0x1F;
        u_int32_t funct3 = (instruction >> 12) & 0x07;
        u_int32_t rs1    = (instruction >> 15) & 0x1F;
        u_int32_t rs2    = (instruction >> 20) & 0x1F;
        u_int32_t funct7 = (instruction >> 25) & 0x7F;

        instruction_count++;
        switch(opcode) {
            case 0x73: //ecall/ebreak
                u_int32_t systemkald = regs[17];
                    switch (systemkald){
                        case 1:
                            regs[10] = getchar();
                            break;
                        case 2:
                            putchar(regs[10]);
                            break;
                        case 3:
                            done = true;
                            break;
                        case 93:
                            done = true;
                            break;
                        default:
                            printf("Error, ukendt systemkald: %u", systemkald);
                            break;
                    }
                break;
            case 0x33: {//R-type (add, sub, and, or, slt, mul …)
                switch (funct3) {
                    case 0x0: { //add, mul og sub
                        if (funct7 == 0x0) { //add

                        }
                        else if (funct7 == 0x1) { //mul

                        }
                        else if (funct7 == 0x20) { //sub

                        }
                        break;
                    }
                    case 0x7: { //and og funct7 = 0000000

                        break;
                    }
                    case 0x6: { //or og funct7 = 0000000

                        break;
                    }
                }
                
            }
            case 0x13: {//I-type (addi, xori, ori, slti, slli, srli, srai …)
                switch (funct3) {
                    case 0x0: { //addi
                        
                        break;
                    }
                    case 0x2: { //slti
                    
                        break;
                    }
                    case 0x4: { //xori

                        break;
                    }
                    case 0x6: { //ori

                        break;
                    }
                    case 0x7: {

                        break;
                    }
                }
            }
            case 0x03: {//l-type lw, lh, lb, lhu, lbu
                switch (funct3) {
                    case 0x0: { //lb

                        break;
                    }
                    case 0x1: { //lh

                        break;
                    }
                    case 0x2: { //lw

                        break;
                    }
                    case 0x4: { //lbu

                        break;
                    }
                    case 0x5: { //lhu

                        break;
                    }
                }
            }
            case 0x67: {//jalr
                decode_I();
                break;
            }
            case 0x23: {//S-type (sw, sh, sb)
                switch (funct3) {
                    case 0x0: { //sb
                        break;
                    }
                    case 0x1: { //sh

                        break;
                    }
                    case 0x2: { //sh

                        break;
                    }
                }
                break;
            }
            case 0x63: {//B-type (beq, bne, blt, bge, bltu, bgeu)
                switch (funct3) {
                    case 0x0: { //beq

                        break;
                    }
                    case 0x1: { //bne

                        break;
                    }
                    case 0x4: { //blt
                        
                        break;
                    }
                    case 0x5: { //bge

                        break;
                    }
                    case 0x6: { //bltu

                        break;
                    }
                    case 0x7: { //bgeu

                        break;
                    }
                }
                break;
            }
            case 0x6F: { //jal
                decode_J();

                break;
            }
            case 0x17: { //auipc
                int32_t imm = (int32_t)(instruction & 0xFFFFF000); // allerede shiftet
                regs[rd] = program_counter + imm20;
                break;
            }
            case 0x37: {//lui
                int32_t imm = (int32_t)(instruction & 0xFFFFF000);
                regs[rd] = imm;
                break;
            }
            default: {
                printf("Ukendt opcode: 0x%x\n", opcode);
                done = true;
            break;
            }
        }
        regs[0] = 0;
        program_counter += 4;
    }
}
