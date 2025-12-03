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
        u_int32_t rd = (instruction >> 7) & 0x1F;
        u_int32_t funct3 = (instruction >> 12) & 0x07;
        u_int32_t rs1 = (instruction >> 15) & 0x1F;
        u_int32_t rs2 = (instruction >> 20) & 0x1F;
        u_int32_t funct7 = (instruction >> 25) & 0x7F;
        u_int32_t systemkald;

        instruction_count++;
        switch(opcode) {
            case 0x73: //ecall/ebreak
                systemkald = regs[17];
                struct Stat stat;
                stat.insns = instruction_count;
                    switch (systemkald){
                        case 0:                     
                            return stat;
                        case 1:
                            regs[10] = getchar();
                            break;
                        case 2:
                            putchar(regs[10]);
                            break;
                        case 3:
                        case 93:{
                            return stat;
                        }
                        default:
                            printf("Error, ukendt systemkald: %u", systemkald);
                            return stat;
                    }
                break;
            case 0x33: {//R-type (add, sub, and, or, slt, mul …)
                switch (funct3) {
                    case 0x0: {
                        if (funct7 == 0x0) { //add
                            regs[rd] = regs[rs1] + regs[rs2];
                        }
                        else if (funct7 == 0x1) { //mul
                            regs[rd] = regs[rs1] * regs[rs2];                            
                        }
                        else if (funct7 == 0x20) { //sub
                            regs[rd] = regs[rs1] - regs[rs2];
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
                            int64_t a = (int64_t)(int32_t)regs[rs1];
                            int64_t b = (int64_t)(uint32_t)regs[rs2];
                            int64_t result = a * b;
                            if (rd != 0) {
                                regs[rd] = (int32_t)(result >> 32);  // høje 32 bit
                            }
                            break;
                        }
                    }
                    case 0x3: {
                        if (funct7 == 0x0) { //sltu
                            if (rd != 0) {
                                regs[rd] = ((uint32_t)regs[rs1] < (uint32_t)regs[rs2]) ? 1 : 0;
                            }
                        }
                        else if (funct7 == 0x1){ //mulhu
                            uint64_t a = (uint64_t)(uint32_t)regs[rs1];
                            uint64_t b = (uint64_t)(uint32_t)regs[rs2];
                            uint64_t result = a * b;
                            if (rd != 0) {
                                regs[rd] = (uint32_t)(result >> 32);  // høje 32 bit
                            }
                        }
                    }
                    case 0x4: { 
                        if (funct7 == 0x0) { //xor
                            regs[rd] = regs[rs1] ^ regs[rs2];
                            break;
                        }
                        else if (funct7 == 0x1){ //div
                            if (regs[rs2] == 0) {
                                regs[rd] = -1;
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
                        else if (funct7 == 0x20){ //sra
                            regs[rd] = regs[rs1] >> regs[rs2];
                            break;
                        }
                        else if (funct7 == 0x1){ //divu
                            if (regs[rs2] == 0) {
                                regs[rd] = -1;
                            }
                            else {
                            regs[rd] = (uint32_t)regs[rs1] / (uint32_t)regs[rs2];
                            }
                            break;
                        }
                    }
                    case 0x6: { //
                        if (funct7 == 0x0) { //or
                            regs[rd] = regs[rs1] | regs[rs2];
                            break;
                        }
                        else if (funct7 == 0x1){ //rem
                            regs[rd] = regs[rs1] % regs[rs2];
                            break;
                        }
                    }
                    case 0x7: {
                        if (funct7 == 0x0) { //and
                            regs[rd] = regs[rs1] & regs[rs2];
                            break;
                        }
                        else if (funct7 == 0x1){ //remu
                            regs[rd] = (uint32_t)regs[rs1] % (uint32_t)regs[rs2];
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
            case 0x03: {//l-type lw, lh, lb, lhu, lbu                   //Vi sign extender de forskellige værdier. 
                int32_t imm = instruction >> 20;
                uint32_t address = regs[rs1] + imm;
                switch (funct3) {
                    case 0x0: { //lb
                        if (rd != 0){
                            regs[rd] = (int8_t)memory_rd_b(mem, address);
                        }
                        break;
                    }
                    case 0x1: { //lh
                        if (rd != 0){
                            regs[rd] = (int16_t)memory_rd_h(mem, address);
                        }
                        break;
                    }
                    case 0x2: { //lw
                        if (rd != 0){
                            regs[rd] = memory_rd_w(mem, address);
                        }
                        break;
                    }
                    case 0x4: { //lbu
                        if (rd != 0){
                            regs[rd] = memory_rd_b(mem, address) & 0xFF;
                        }
                        break;
                    }
                    case 0x5: { //lhu
                        if (rd != 0){
                            regs[rd] = memory_rd_h(mem, address) & 0xFFFF;
                        }
                        break;
                    }
                }
                break;
            }
            case 0x67: {//jalr
                int32_t imm = (int32_t)instruction >> 20;           // sign-extendet offset
                uint32_t target = regs[rs1] + imm;
                target &= ~1;                                       // clear bit 0 (RISC-V kræver det)
                if (rd != 0) {
                    regs[rd] = program_counter + 4;
                }
                program_counter = target - 4;                       // -4 fordi vi tilføjer +4 til sidst
                break;
            }
            case 0x23: {//S-type (sw, sh, sb)
                int32_t imm = ((int32_t)(instruction << 12) >> 20);
                uint32_t address = regs[rs1] + imm;
                switch (funct3) {
                    case 0x0: { //sb
                            memory_wr_b(mem, address, regs[rs2] & 0xFF);
                        break;
                    }
                    case 0x1: { //sh
                            memory_wr_h(mem, address, regs[rs2] & 0xFFFF);                
                        break;
                    }
                    case 0x2: { //sw
                            memory_wr_w(mem, address, regs[rs2]);                        
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

                switch (funct3) {
                    case 0x0: { //beq
                        if (regs[rs1] == regs[rs2])
                            program_counter += immB;
                        break;
                    }
                    case 0x1: { //bne
                        if (regs[rs1] != regs[rs2])
                            program_counter += immB;
                        break;
                    }
                    case 0x4: { //blt
                        if ((int32_t)regs[rs1] < (int32_t)regs[rs2])
                            program_counter += immB;
                        break;
                    }
                    case 0x5: { //bge
                        if ((int32_t)regs[rs1] >= (int32_t)regs[rs2])
                            program_counter += immB;
                        break;
                    }
                    case 0x6: { //bltu
                        if ((uint32_t)regs[rs1] < (uint32_t)regs[rs2])
                            program_counter += immB;
                        break;
                    }
                    case 0x7: { //bgeu
                        if ((uint32_t)regs[rs1] >= (uint32_t)regs[rs2])
                            program_counter += immB;
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
