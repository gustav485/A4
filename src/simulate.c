#include <memory.h>
#include "simulate.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

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
                        case 93:{
                            struct Stat stat;
                            stat.insns = instruction_count;
                            return stat;
                        }
                        default:
                            printf("Error, ukendt systemkald: %u", systemkald);
                            break;
                    }
                break;
            case 0x33: {//R-type (add, sub, and, or, slt, mul …)
                switch (funct3) {
                    case 0x0: {
                        if (funct7 == 0x0) { //add
                            regs[rd] = regs[rs1] + regs[rs2];
                            instruction_count++;
                        }
                        else if (funct7 == 0x1) { //mul
                            regs[rd] = regs[rs1] * regs[rs2];
                            instruction_count++;
                        }
                        else if (funct7 == 0x32) { //sub
                            regs[rd] = regs[rs1] - regs[rs2];
                            instruction_count++;
                        }
                        break;
                    }
                    case 0x1:{ //sll
                        if (funct7 == 0x0){ //sll
                            regs[rd] = regs[rs1] << regs[rs2];
                            break;
                        }
                        else if (funct7 == 0x1) { //mulh
                            regs[rd] = (u_int32_t) regs[rs1] * (u_int32_t) regs[rs2];
                            break;
                        }
                    }
                    case 0x2: { 
                        if (funct7 == 0x0){ //slt
                            regs[rd] = (regs[rs1] < regs[rs2]);
                            break;
                        }
                        else if (funct7 == 0x1){ //mulhsu
                            u_int32_t long long regs[rd] = long long regs[rs1] * u_int32_t long long regs[rs2];
                            break;
                        }
                    }
                    case 0x3: {
                        if (funct7 == 0x0) { //sltu
                            u_int32_t regs[rd] = (u_int32_t regs[rs1] < regs[rs2]);
                            break;
                        }
                        else if (funct7 == 0x1){ //mulhu
                            u_int32_t long long regs[rd] = u_int32_t long long regs[rs1] * u_int32_t long long regs[rs2];
                            break;
                        }
                    }
                    case 0x4: { 
                        if (funct7 == 0x0) { //xor
                            regs[rd] = regs[rs1] ^ regs[rs2];
                            break;
                        }
                        else if (funct7 == 0x1){ //div
                            if (rs2 == 0) {
                                return -1;
                            }
                            else {
                                regs[rd] = regs[rs1] / regs[rs2];
                            }
                            break;
                        }
                    }
                    case 0x5: {
                        if (funct7 == 0x0){ //srl
                            regs[rd] = regs[rs1] >> regs[rs2];
                            break;
                        }
                        else if (funct7 == 0x32){ //sra
                            regs[rd] = regs[rs1] >>> regs[rs2];
                            break;
                        }
                        else if (funct7 == 0x1){ //divu
                            if (regs[rs2] == 0) {
                                return -1;
                            }
                            else {
                            u_int32_t regs[rd] = (u_int32_t regs[rs1] / u_int32_t regs[rs2]);
                            }
                            break;
                        }
                    }
                    case 0x6: { //
                        if (funct7 == 0x0) { //or
                            regs[rd] = regs[rs1] || regs[rs2];
                            break;
                        }
                        else if (funct7 == 0x1){ //rem
                            regs[rd] = regs[rs1] % regs[rs2];
                            break;
                        }
                    }
                    case 0x7: {
                        if (funct7 == 0x0) { //and
                            regs[rd] = regs[rs1] && regs[rs2];
                            break;
                        }
                        else if (funct7 == 0x1){ //remu
                            u_int32_t regs[rd] = u_int32_t regs[rs1] % regs[rs2];
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
                        if (rd != 0)
                            regs[rd] = regs[rs1] + immI;
                        break;
                    }
                    case 0x1: { //slli
                        if (rd != 0)
                            regs[rd] = regs[rs1] << shamt;
                        break;
                    }
                    case 0x2: { //slti
                        if (rd != 0)
                            regs[rd] =  (regs[rs1] < immI) ? 1 : 0;                    
                        break;
                    }
                    case 0x3: { //sltiu
                        if (rd != 0)
                            regs[rd] =  ((uint32_t)regs[rs1] < (uint32_t)immI) ? 1 : 0;  
                        break;
                    }
                    case 0x4: { //xori
                        if (rd != 0)
                            regs[rd] = regs[rs1] ^ immI;
                        break;
                    }
                    case 0x5:{
                        if (funct7 == 0x0){ //srli
                            if (rd != 0)
                                regs[rd] = ((uint32_t)regs[rs1]) >> shamt;
                        }
                        else if (funct7 == 0x20){ //srai
                            if (rd != 0)
                                regs[rd] = regs[rs1] >> shamt;
                        }
                        break;
                    }
                    case 0x6: { //ori
                        if (rd != 0)
                            regs[rd] = regs[rs1] | immI;
                        break;
                    }
                    case 0x7: { //andi
                        if (rd != 0)
                            regs[rd] = regs[rs1] & immI;
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
                break;
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
                int32_t imm = 
                        ((instruction >> 11) & 0x00100000) |   // bit 20 (sign)
                        ((instruction >> 20) & 0x00000800) |   // bit 11
                        ((instruction >>  0) & 0x000FF000) |   // bit 19:12
                        ((instruction >> 21) & 0x000007FE);    // bit 10:1

                if (imm & 0x00100000) imm |= 0xFFE00000;  // sign-extend

                if (rd != 0) {
                    regs[rd] = program_counter + 4;   // ← returadresse!
                }

                program_counter = program_counter + imm - 4;  // ← HOP! (-4 fordi vi tilføjer 4 senere)

                break;
            }
            case 0x17: { //auipc
                int32_t imm = (int32_t)(instruction & 0xFFFFF000); // allerede shiftet
                if (rd != 0){
                    regs[rd] = program_counter + imm;
                }
                break;
            }
            case 0x37: {//lui
                int32_t imm = (int32_t)(instruction & 0xFFFFF000);
                if (rd != 0){
                    regs[rd] = imm;
                }
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
