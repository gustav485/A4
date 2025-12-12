#include "memory.h"
#include "simulate.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "disassemble.h"


struct Stat simulate(struct memory *mem, int start_addr, FILE *log_file, struct symbols* symbols, int predictor_type) {
    int32_t regs[32] = {0};
    uint32_t program_counter = start_addr;
    long int instruction_count = 0;
    bool done = false;
    long int branch_count = 0;
    long int mispredict_count = 0;
    uint32_t last_pc = start_addr - 4;  

    while (!done) {
        uint32_t instruction = memory_rd_w(mem, program_counter);
        uint32_t opcode = instruction & 0x7F;
        uint32_t rd = (instruction >> 7) & 0x1F;
        uint32_t funct3 = (instruction >> 12) & 0x07;
        uint32_t rs1 = (instruction >> 15) & 0x1F;
        uint32_t rs2 = (instruction >> 20) & 0x1F;
        uint32_t funct7 = (instruction >> 25) & 0x7F;
        uint32_t systemkald;
        //printf("insn %ld: pc=0x%08x instr=0x%08x opcode=0x%x\n",instruction_count, program_counter, instruction, opcode);

        instruction_count++;
        bool taken = false;

        if (log_file) {
            char buf[128];
            disassemble(program_counter, instruction, buf, sizeof(buf), symbols);

            fprintf(log_file, "%6ld %c %08x : %08x   %-30s",
                    instruction_count,
                    (program_counter == last_pc + 4 ? ' ' : ' '),
                    program_counter,
                    instruction,
                    buf);

            fprintf(log_file, "\n");
        }

        switch(opcode) {
            case 0x73: { // ecall / ebreak
                if (instruction == 0x00000073) { // ecall
                    uint32_t call = regs[17]; // a7

                    if (call == 1) { // getchar
                        int c = getchar();
                        regs[10] = (c == EOF) ? -1 : c;
                    }
                    else if (call == 2) { // putchar
                        putchar(regs[10]);
                        fflush(stdout);
                    }
                    else if (call == 3 || call == 93) { // exit
                        fflush(stdout);
                        struct Stat stat = {
                            .insns          = instruction_count,
                            .branches       = branch_count,
                            .mispredictions = mispredict_count
                        };
                        return stat;
                    }
                }
                break;
            }
            case 0x33: {//R-type (add, sub, and, or, slt, mul …)
                switch (funct3) {
                    case 0x0: {
                        if (funct7 == 0x00) { //add
                            if (rd != 0) {
                                regs[rd] = regs[rs1] + regs[rs2];
                            }
                        }
                        else if (funct7 == 0x1) { //mul
                            if (rd != 0) {
                                regs[rd] = (int32_t)((int64_t)regs[rs1] * regs[rs2]);
                            }
                        }
                        else if (funct7 == 0x20) { //sub
                            if (rd != 0) {
                                regs[rd] = regs[rs1] - regs[rs2];
                            }
                        }
                        break;
                    }
                    case 0x1:{ //sll
                        if (funct7 == 0x0){ //sll
                            if (rd != 0) {
                                regs[rd] = regs[rs1] << regs[rs2];
                            }
                            break;
                        }
                        else if (funct7 == 0x1) { //mulh
                            if (rd != 0) {
                                regs[rd] = (uint32_t) regs[rs1] * (uint32_t) regs[rs2];
                            }
                            break;
                        }
                    }
                    case 0x2: { 
                        if (funct7 == 0x0) { //slt
                            if (rd !=0) {
                                regs[rd] = (regs[rs1] < regs[rs2]);
                            }
                            break;
                        }
                        else if (funct7 == 0x1) { //mulhsu
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
                        else if (funct7 == 0x1) { //mulhu
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
                            if (rd != 0) {
                                regs[rd] = regs[rs1] ^ regs[rs2];
                            }
                            break;
                        }
                        else if (funct7 == 0x1) { //div
                            if (rd != 0) {
                                if (regs[rs2] == 0) {
                                    regs[rd] = -1;
                                }
                                else {
                                    regs[rd] = regs[rs1] / regs[rs2];
                                }
                            }
                            break;
                        }
                    }
                    case 0x5: {
                        if (funct7 == 0x0) { //srl
                            if (rd != 0) {
                                regs[rd] = regs[rs1] >> regs[rs2];
                            }
                            break;
                        }
                        else if (funct7 == 0x20) { //sra
                            if (rd != 0) {
                                regs[rd] = regs[rs1] >> regs[rs2];
                            }
                            break;
                        }
                        else if (funct7 == 0x1) { //divu
                            if (rd != 0) {
                                if (regs[rs2] == 0) {
                                    regs[rd] = -1;
                                }
                                else {
                                    regs[rd] = (uint32_t)regs[rs1] / (uint32_t)regs[rs2];
                                }
                            }
                            break;
                        }
                    }
                    case 0x6: // or / rem
                            if (funct7 == 0x00) { // or
                                if (rd) regs[rd] = regs[rs1] | regs[rs2];
                            } else if (funct7 == 0x01) { // rem 
                                if (rd) {
                                    if (regs[rs2] == 0) {
                                        regs[rd] = regs[rs1];                
                                    } else {
                                        regs[rd] = (int32_t)regs[rs1] % (int32_t)regs[rs2];
                                    }
                                }
                            }
                            break;
                    case 0x7: {
                        if (funct7 == 0x0) { //and
                            if (rd != 0) {
                                regs[rd] = regs[rs1] & regs[rs2];
                            }
                            break;
                        }
                        else if (funct7 == 0x1) { //remu
                            if (rd != 0) {
                                regs[rd] = (uint32_t)regs[rs1] % (uint32_t)regs[rs2];
                            }
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
                        if (rd != 0) {
                            regs[rd] = regs[rs1] + immI;
                        }
                        break;
                    }
                    case 0x1: { //slli
                        if (rd != 0) {
                            regs[rd] = regs[rs1] << shamt;
                        }
                        break;
                    }
                    case 0x2: { //slti
                        if (rd != 0) {
                            regs[rd] = (regs[rs1] < immI) ? 1 : 0;
                        }
                        break;
                    }
                    case 0x3: { //sltiu
                        if (rd != 0) {
                            regs[rd] = ((uint32_t)regs[rs1] < (uint32_t)immI) ? 1 : 0;
                        }
                        break;
                    }
                    case 0x4: { //xori
                        if (rd != 0) {
                            regs[rd] = regs[rs1] ^ immI;
                        }
                        break;
                    }
                    case 0x5:{
                        if (funct7 == 0x0){ //srli
                            if (rd != 0) {
                                regs[rd] = ((uint32_t)regs[rs1]) >> shamt;
                            }
                        }
                        else if (funct7 == 0x20){ //srai
                            if (rd != 0) {
                                regs[rd] = (int32_t)regs[rs1] >> shamt;
                            }
                        }
                        break;
                    }
                    case 0x6: { //ori
                        if (rd != 0) {
                            regs[rd] = regs[rs1] | immI;
                        }
                        break;
                    }
                    case 0x7: { //andi
                        if (rd != 0) {
                            regs[rd] = regs[rs1] & immI;
                        }
                        break;
                    }
                }
                break;
            }
            case 0x03: {//l-type lw, lh, lb, lhu, lbu                   //Vi sign extender de forskellige værdier. 
                int32_t imm = ((int32_t)instruction) >> 20;
                uint32_t address = regs[rs1] + imm;
                switch (funct3) {
                    case 0x0: { //lb
                        if (rd != 0) {
                            regs[rd] = (int8_t)memory_rd_b(mem, address);
                        }
                        break;
                    }
                    case 0x1: { //lh
                        if (rd != 0) {
                            regs[rd] = (int16_t)memory_rd_h(mem, address);
                        }
                        break;
                    }
                    case 0x2: { //lw
                        if (rd != 0) {
                            regs[rd] = memory_rd_w(mem, address);
                        }
                        break;
                    }
                    case 0x4: { //lbu
                        if (rd != 0) {
                            regs[rd] = memory_rd_b(mem, address) & 0xFF;
                        }
                        break;
                    }
                    case 0x5: { //lhu
                        if (rd != 0) {
                            regs[rd] = memory_rd_h(mem, address) & 0xFFFF;
                        }
                        break;
                    }
                }
                break;
            }
            case 0x67: { // jalr
                int32_t imm = (int32_t)instruction >> 20;
                uint32_t target = (regs[rs1] + imm);  

                if (rd != 0) {
                    regs[rd] = program_counter + 4;  
                }
                program_counter = target;            
                continue;                            
            }
            case 0x23: {//S-type (sw, sh, sb)
                int32_t imm = ((int32_t)(instruction >> 25) << 5) | ((instruction >> 7) & 0x1F);
                if (imm & 0x800) imm |= 0xFFFFF000; 
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
            case 0x63: { // branches
                int32_t imm = 
                    ((instruction >> 19) & 0x1000) |   // imm[12]
                    ((instruction << 4)  & 0x800)  |   // imm[11]
                    ((instruction >> 20) & 0x7E0)  |   // imm[10:5]
                    ((instruction >> 7)  & 0x1E);      // imm[4:1]

                if (imm & 0x1000) imm |= 0xFFFFF000;  // sign-extend
                
                uint32_t target = program_counter + imm;
                
                switch (funct3) {
                    case 0x0: { //beq
                        taken = (regs[rs1] == regs[rs2]);
                        break;
                    }
                    case 0x1: { //bne
                        taken = (regs[rs1] != regs[rs2]);
                        break;
                    }
                    case 0x4: { //blt
                        taken = ((int32_t)regs[rs1] < (int32_t)regs[rs2]);
                        break;
                    }
                    case 0x5: { //bge
                        taken = ((int32_t)regs[rs1] >= (int32_t)regs[rs2]);
                        break;
                    }
                    case 0x6: { //bltu
                        taken = ((uint32_t)regs[rs1] < (uint32_t)regs[rs2]);
                        break;
                    }
                    case 0x7: { //bgeu
                        taken = ((uint32_t)regs[rs1] >= (uint32_t)regs[rs2]);
                        break;
                    }
                }
                branch_count++; 
                bool predicted_taken = false;

                if (predictor_type == 1) {
                        // NT: Always Not Taken
                        predicted_taken = false;
                    }
                    else if (predictor_type == 2) {
                        // BTFNT: Backward Taken, Forward Not Taken
                        predicted_taken = (imm < 0);
                    }
                    else if (predictor_type >= 3) {
                        // Bimodal
                        static uint8_t pht[4096] = {0};  
                        int index = (program_counter >> 2) & 0xFFF;  

                        uint8_t counter = pht[index];
                        predicted_taken = (counter >= 2);  

                        if (taken) {
                            if (counter < 3) pht[index]++;
                        } else {
                            if (counter > 0) pht[index]--;
                        }
                    }

                    if (predicted_taken != taken) {
                        mispredict_count++;
                    }

                    if (taken) {
                        program_counter = target;
                        continue;
                    }
                    break;
                }
    
            case 0x6F: { // jal
                int32_t imm = 
                    ((instruction & 0x80000000) >> 11) |  // bit 31 → imm[20]
                    ((instruction & 0x7FE00000) >> 20) |  // bit 30:21 → imm[10:1]
                    ((instruction & 0x00100000) >> 9)  |  // bit 20 → imm[11]
                    ((instruction & 0x000FF000));         // bit 19:12 → imm[19:12]

                if (imm & 0x100000) imm |= 0xFFF00000;  // sign-extend

                if (rd != 0) {
                    regs[rd] = program_counter + 4;
                }
                program_counter += imm;
                continue;  // spring over +4
            }
            case 0x17: { //auipc
                int32_t imm = (int32_t)(instruction & 0xFFFFF000); // allerede shiftet
                if (rd != 0) {
                    regs[rd] = program_counter + imm;
                }
                break;
            }
            case 0x37: {//lui
                int32_t imm = (int32_t)(instruction & 0xFFFFF000);
                if (rd != 0) {
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

        // if (instruction_count % 100 == 0) {
        //     printf("Instr %ld: a0 = %d\n", instruction_count, regs[10]);
        // }

        program_counter += 4;
    }

    struct Stat stat = {
        .insns = instruction_count,
        .branches = branch_count,
        .mispredictions = mispredict_count
    };
    return stat;
}
