#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "memory.h"        
#include "disassemble.h"   

void disassemble(uint32_t addr, uint32_t instruction, char* result, size_t buf_size, struct symbols* symbols)
{
    static const char* regname[32] = {
        "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
        "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
    };

    uint32_t opcode = instruction & 0x7F;
    uint32_t rd  = (instruction >> 7) & 0x1F;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;
    uint32_t funct3 = (instruction >> 12) & 7;
    uint32_t funct7 = (instruction >> 25) & 0x7F;

    switch(opcode) {
            case 0x73: {
                snprintf(result, buf_size, "ecall");
                break;
            }
            case 0x33: {//R-type (add, sub, and, or, slt, mul …)
                switch (funct3) {
                    case 0x0: {
                        if (funct7 == 0x0) { //add
                            snprintf(result, buf_size, "add %s %s %s", regname[rd], regname[rs1], regname[rs2]);
                        }
                        else if (funct7 == 0x1) { //mul
                            snprintf(result, buf_size, "mul %s %s %s", regname[rd], regname[rs1], regname[rs2]);                            
                        }
                        else if (funct7 == 0x20) { //sub
                            snprintf(result, buf_size, "sub %s %s %s", regname[rd], regname[rs1], regname[rs2]);
                        }
                        break;
                    }
                    case 0x1:{ //sll
                        if (funct7 == 0x0){ //sll
                            snprintf(result, buf_size, "sll %s %s %s", regname[rd], regname[rs1], regname[rs2]);
                            break;
                        }
                        else if (funct7 == 0x1) { //mulh
                            snprintf(result, buf_size, "mulh %s %s %s", regname[rd], regname[rs1], regname[rs2]);
                            break;
                        }
                    }
                    case 0x2: { 
                        if (funct7 == 0x0){ //slt
                            snprintf(result, buf_size, "slt %s %s %s", regname[rd], regname[rs1], regname[rs2]);
                            break;
                        }
                        else if (funct7 == 0x1){ //mulhsu
                            snprintf(result, buf_size, "mulhsu %s %s %s", regname[rd], regname[rs1], regname[rs2]); 
                            }
                            break;
                        }
                    case 0x3: {
                        if (funct7 == 0x0) { //sltu
                            snprintf(result, buf_size, "sltu %s %s %s", regname[rd], regname[rs1], regname[rs2]);
                        }
                        else if (funct7 == 0x1){ //mulhu
                            snprintf(result, buf_size, "mulhu %s %s %s", regname[rd], regname[rs1], regname[rs2]);
                        }
                    }
                    case 0x4: { 
                        if (funct7 == 0x0) { //xor
                            snprintf(result, buf_size, "xor %s %s %s", regname[rd], regname[rs1], regname[rs2]);
                            break;
                        }
                        else if (funct7 == 0x1){ //div
                            snprintf(result, buf_size, "div %s %s %s", regname[rd], regname[rs1], regname[rs2]);
                            break;
                        }
                    }
                    case 0x5: {
                        if (funct7 == 0x0){ //srl
                            snprintf(result, buf_size, "srl %s %s %s", regname[rd], regname[rs1], regname[rs2]);
                            break;
                        }
                        else if (funct7 == 0x20){ //sra
                            snprintf(result, buf_size, "sra %s %s %s", regname[rd], regname[rs1], regname[rs2]);
                            break;
                        }
                        else if (funct7 == 0x1){ //divu
                            snprintf(result, buf_size, "divu %s %s %s", regname[rd], regname[rs1], regname[rs2]);
                            break;
                        }
                    }
                    case 0x6: { //
                        if (funct7 == 0x0) { //or
                            snprintf(result, buf_size, "or %s %s %s", regname[rd], regname[rs1], regname[rs2]);
                            break;
                        }
                        else if (funct7 == 0x1){ //rem
                            snprintf(result, buf_size, "rem %s %s %s", regname[rd], regname[rs1], regname[rs2]);
                            break;
                        }
                    }
                    case 0x7: {
                        if (funct7 == 0x0) { //and
                            snprintf(result, buf_size, "and %s %s %s", regname[rd], regname[rs1], regname[rs2]);
                            break;
                        }
                        else if (funct7 == 0x1){ //remu
                            snprintf(result, buf_size, "remu %s %s %s", regname[rd], regname[rs1], regname[rs2]);
                            break;
                        }
                    }
                }
                break;
            }
            case 0x13: {//I-type (addi, xori, ori, slti, slli, srli, srai …)
                int32_t immI = (int32_t)instruction >> 20; //de første 12 bits skal stores i imm[11:0]
                uint32_t shamt = immI & 0x1F;
                switch (funct3) {
                    case 0x0: { //addi
                        snprintf(result, buf_size, "addi %s %s %d", regname[rd], regname[rs1], immI);
                        break;

                    }
                    case 0x1: { //slli
                        snprintf(result, buf_size, "slli %s %s %d", regname[rd], regname[rs1], shamt);
                        break;
                    }
                    case 0x2: { //slti
                        snprintf(result, buf_size, "slti %s %s %d", regname[rd], regname[rs1], immI);               
                        break;
                    }
                    case 0x3: { //sltiu
                        snprintf(result, buf_size, "sltiu %s %s %d", regname[rd], regname[rs1], immI);
                        break;
                    }
                    case 0x4: { //xori
                        snprintf(result, buf_size, "xori %s %s %d", regname[rd], regname[rs1], immI);
                        break;
                    }
                    case 0x5:{
                        if (funct7 == 0x0){ //srli
                            snprintf(result, buf_size, "srli %s %s %d", regname[rd], regname[rs1], shamt);
                        }
                        else if (funct7 == 0x20){ //srai
                            snprintf(result, buf_size, "srai %s %s %d", regname[rd], regname[rs1], shamt);                        
                        }
                        break;
                    }
                    case 0x6: { //ori
                        snprintf(result, buf_size, "ori %s %s %d", regname[rd], regname[rs1], immI);
                        break;
                    }
                    case 0x7: { //andi
                        snprintf(result, buf_size, "andi %s %s %d", regname[rd], regname[rs1], immI);
                        break;
                    }
                }
                break;
            }
            case 0x03: {//l-type lw, lh, lb, lhu, lbu
                int32_t imm = instruction >> 20;
                switch (funct3) {
                    case 0x0: { //lb
                        snprintf(result, buf_size, "lb %s %d(%s)", regname[rd], imm, regname[rs1]);
                        break;
                    }
                    case 0x1: { //lh
                        snprintf(result, buf_size, "lh %s %d(%s)", regname[rd], imm, regname[rs1]);
                        break;
                    }
                    case 0x2: { //lw
                        snprintf(result, buf_size, "lw %s %d(%s)", regname[rd], imm, regname[rs1]);
                        break;
                    }
                    case 0x4: { //lbu
                        snprintf(result, buf_size, "lbu %s %d(%s)", regname[rd], imm, regname[rs1]);
                        break;
                    }
                    case 0x5: { //lhu
                        snprintf(result, buf_size, "lhu %s %d(%s)", regname[rd], imm, regname[rs1]);
                        break;
                    }
                }
                break;
            }
            case 0x67: {//jalr
                int32_t imm = (int32_t)instruction >> 20;           // sign-extendet offset
                snprintf(result, buf_size, "jalr %s %d(%s)", regname[rd], imm, regname[rs1]);
                break;
            }
            case 0x23: {//S-type (sw, sh, sb)
                int32_t imm = ((instruction >> 7) & 0x1F) | (((int32_t)instruction << 12) >> 20);
                if (imm & 0x800) imm |= 0xFFFFF000;  // sign-extension of bit 11
                switch (funct3) {
                    case 0x0: { //sb
                        snprintf(result, buf_size, "sb %s %d(%s)", regname[rs2], imm, regname[rs1]);
                        break;
                    }
                    case 0x1: { //sh
                        snprintf(result, buf_size, "sh %s %d(%s)", regname[rs2], imm, regname[rs1]);
                        break;
                    }
                    case 0x2: { //sw
                        snprintf(result, buf_size, "sw %s %d(%s)", regname[rs2], imm, regname[rs1]);
                        break;
                    }
                }
                break;
            }
            case 0x63: {//B-type (beq, bne, blt, bge, bltu, bgeu)
                int32_t imm12 = (instruction >> 31) & 0x1;
                int32_t imm10_5 = (instruction >> 25) & 0x3F;
                int32_t imm4_1 = (instruction >> 8) & 0xF;
                int32_t imm11 = (instruction >> 7) & 0x1;

                int32_t immB = 0;
                immB |= (imm12 << 12);
                immB |= (imm10_5 << 5);
                immB |= (imm4_1 << 1);
                immB |= (imm11 << 11);
                if (immB & 0x1000) immB |= 0xFFFFF000;  // sign-extension of bit 12

                uint32_t target = addr + immB; 

                switch (funct3) {
                    case 0x0: { //beq
                        snprintf(result, buf_size, "beq %s %s %d", regname[rs1], regname[rs2], target);
                        break;
                    }
                    case 0x1: { //bne
                        snprintf(result, buf_size, "bne %s %s %d", regname[rs1], regname[rs2], target);
                        break;
                    }
                    case 0x4: { //blt
                        snprintf(result, buf_size, "blt %s %s %d", regname[rs1], regname[rs2], target);
                        break;
                    }
                    case 0x5: { //bge
                        snprintf(result, buf_size, "bge %s %s %d", regname[rs1], regname[rs2], target);
                        break;
                    }
                    case 0x6: { //bltu
                        snprintf(result, buf_size, "bltu %s %s %d", regname[rs1], regname[rs2], target);
                        break;
                    }
                    case 0x7: { //bgeu
                        snprintf(result, buf_size, "bgeu %s %s %d", regname[rs1], regname[rs2], target);
                        break;
                    }
                }
                break;
            }
            case 0x6F: { //jal
                int32_t imm = 
                    ((instruction & 0x80000000) >> 11) |  // imm[20] -> bit 20
                    ((instruction & 0x7FE00000) >> 20) |  // imm[10:1]
                    ((instruction & 0x00100000) >> 9)  |  // imm[11]
                    ((instruction & 0x000FF000));         // imm[19:12]

                if (imm & 0x100000) imm |= 0xFFF00000;  // sign-extension
                uint32_t target = addr + imm;

                if (rd == 0){
                    snprintf(result, buf_size, "j 0x%x", target);
                }
                else{
                    snprintf(result, buf_size, "jal %s 0x%x", regname[rd], target);
                }
                break;
            }
            case 0x17: { //auipc
                snprintf(result, buf_size, "auipc %s 0x%x", regname[rd], instruction & 0xFFFFF000);
                break;
            }
            case 0x37: {//lui
                snprintf(result, buf_size, "lui %s 0x%x", regname[rd], instruction & 0xFFFFF000);
                break;
            }
            default: {
                snprintf(result, buf_size, "what the helly: 0x%08x 0x%x", instruction);
                break;
            }
        }
}
