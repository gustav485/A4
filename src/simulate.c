#include "memory.h"
#include "simulate.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
    int size;             // Entries fra (256, 1K, 4K og 16K)
    uint8_t *table;       // Tabel
    uint64_t errors;      
}Predictor;

void handle_prediction(Predictor *p, uint32_t index, bool actual_taken) {
    uint8_t state = p->table[index];
    bool predicted_taken = (state >= 2);

    // Tæller fejl
    if (predicted_taken != actual_taken) {
        p->errors++;
    }

    // Opdater 2-bit counter (saturating math)
    if (actual_taken) {
        if (state < 3) p->table[index]++;
    } else {
        if (state > 0) p->table[index]--;
    }
}

struct Stat simulate(struct memory *mem, int start_addr, FILE *log_file, struct symbols* symbols) {
    int32_t regs[32] = {0};
    uint32_t program_counter = start_addr;
    long int instruction_count = 0;
    bool done = false;

    //Tæller
    uint64_t total_branches = 0;
    uint64_t mispredicts_nt = 0;
    uint64_t mispredicts_btfnt = 0;

    int sizes[] = {256, 1024, 4096, 16384};
    int num_sizes = 4;

    // Allocating memory
    Predictor bimodal_preds[4];
    Predictor gshare_preds[4];
    uint32_t gshare_ghrs[4] = {0};

    for (int i = 0; i < num_sizes; i++) {
        // Bimodal setup
        bimodal_preds[i].size = sizes[i];
        bimodal_preds[i].errors = 0;
        bimodal_preds[i].table = (uint8_t*)calloc(sizes[i], sizeof(uint8_t));
        // sætter som weakly not taken 
        for (int k = 0; k < sizes[i]; k++) {
            bimodal_preds[i].table[k] = 1;
        }

        // gShare setup
        gshare_preds[i].size = sizes[i];
        gshare_preds[i].errors = 0;
        gshare_preds[i].table = (uint8_t*)calloc(sizes[i], sizeof(uint8_t));
        for (int k = 0; k < sizes[i]; k++) {
            gshare_preds[i].table[k] = 1;
        }
    }

    while (!done) {
        uint32_t instruction = memory_rd_w(mem, program_counter);
        uint32_t opcode = instruction & 0x7F;
        uint32_t rd = (instruction >> 7) & 0x1F;
        uint32_t funct3 = (instruction >> 12) & 0x07;
        uint32_t rs1 = (instruction >> 15) & 0x1F;
        uint32_t rs2 = (instruction >> 20) & 0x1F;
        uint32_t funct7 = (instruction >> 25) & 0x7F;
        uint32_t systemkald;

        bool pc_updated = false;

        instruction_count++;

        switch(opcode) {
            case 0x73: {//ecall/ebreak
                systemkald = regs[17];
                struct Stat stat;
                stat.insns = instruction_count;

                // Tjek om afslut
                bool exiting = (systemkald == 0 || systemkald == 3 || systemkald == 93);
                if (exiting) {
                    printf("NT miss: %d, BTFNT miss: %d\n", mispredicts_nt, mispredicts_btfnt);
                    printf("Total branch: %d \n", total_branches);
                }

                    switch (systemkald) {
                        case 0: {
                            return stat;
                        }
                        case 1: {
                            regs[10] = getchar();
                            break;
                        }
                        case 2: {
                            putchar(regs[10]);
                            break;
                        }
                        case 3:
                        case 93:{
                            return stat;
                        }
                        default: {
                            printf("Error, ukendt systemkald: %u", systemkald);
                            return stat;
                        }
                    }
                break;
            }
            case 0x33: {//R-type (add, sub, and, or, slt, mul …)
                switch (funct3) {
                    case 0x0: {
                        if (funct7 == 0x0) { //add
                            if (rd != 0) {
                                regs[rd] = regs[rs1] + regs[rs2];
                            }
                        }
                        else if (funct7 == 0x1) { //mul
                            if (rd != 0) {
                                regs[rd] = regs[rs1] * regs[rs2];
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
                                break;
                            }
                            break;
                        }
                    }
                    case 0x3: {
                        if (funct7 == 0x0) { //sltu
                            if (rd != 0) {
                                regs[rd] = ((uint32_t)regs[rs1] < (uint32_t)regs[rs2]) ? 1 : 0;
                                break;
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
                    case 0x6: { //
                        if (funct7 == 0x0) { //or
                            if (rd != 0) {
                                regs[rd] = regs[rs1] | regs[rs2];
                            }
                            break;
                        }
                        else if (funct7 == 0x1) { //rem
                            if (rd != 0) {
                                regs[rd] = regs[rs1] % regs[rs2];
                            }
                            break;
                        }
                    }
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
                                regs[rd] = regs[rs1] >> shamt;
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
            }
            case 0x03: {//l-type lw, lh, lb, lhu, lbu                   //Vi sign extender de forskellige værdier. 
                int32_t imm = instruction >> 20;
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
            case 0x67: {//jalr
                int32_t imm = (int32_t)instruction >> 20;           // sign-extendet offset
                uint32_t target = regs[rs1] + imm;
                target &= ~1;                                       // clear bit 0 (RISC-V kræver det)
                if (rd != 0) {
                    regs[rd] = program_counter + 4;
                }
                program_counter = target;                       // -4 fordi vi tilføjer +4 til sidst
                regs[0] = 0;
                continue;
            }
            case 0x23: {//S-type (sw, sh, sb)
                //int32_t imm = ((int32_t)(instruction << 12) >> 20);
                int32_t imm11_5 = (instruction >> 25) & 0x7F;
                int32_t imm4_0  = (instruction >> 7) & 0x1F;
                int32_t imm = (imm11_5 << 5) | imm4_0;
                
                // Sign-extend fra 12-bit til 32-bit
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
            case 0x63: {//B-type (beq, bne, blt, bge, bltu, bgeu)
                int32_t immB = ((instruction >> 19) & 0x1000) |   // bit 12
                               ((instruction << 4)  & 0x0800) |   // bit 11
                               ((instruction >> 20) & 0x07E0) |   // bits 10:5
                               ((instruction >> 7)  & 0x001E);    // bits 4:1
                if (immB & 0x1000) immB |= 0xFFFFF000; // sign-extend bit 12
                //immB <<= 1; // fordi offset er i bytes, og bit 0 altid 0

                bool taken = false;

                switch (funct3) {
                    case 0x0: { //beq
                        taken = (regs[rs1] == regs[rs2]);
                        //printf("hello from beq\n");
                            //program_counter += immB;
                        break;
                    }
                    case 0x1: { //bne
                        taken = (regs[rs1] != regs[rs2]);
                        //printf("hello from bne\n");
                            //program_counter += immB;
                        break;
                    }
                    case 0x4: { //blt
                        taken = ((int32_t)regs[rs1] < (int32_t)regs[rs2]);
                        //printf("hello from blt\n");
                            //program_counter += immB;
                        break;
                    }
                    case 0x5: { //bge
                        taken = ((int32_t)regs[rs1] >= (int32_t)regs[rs2]);
                        //printf("hello from bge\n");
                            //program_counter += immB;
                        break;
                    }
                    case 0x6: { //bltu
                        taken = ((uint32_t)regs[rs1] < (uint32_t)regs[rs2]);
                        //printf("hello from bltu\n");
                            //program_counter += immB;
                        break;
                    }
                    case 0x7: { //bgeu
                        taken = ((uint32_t)regs[rs1] >= (uint32_t)regs[rs2]);
                        //printf("hello from bgeu\n");
                            //program_counter += immB;
                        break;
                    }
                }
                total_branches++;
                if (taken) {
                    mispredicts_nt++; //NT fajler når hop bliver taget
                }
                // BTFNT logic
                bool backward = (immB < 0);
                bool btfnt_prediction = backward; // Gæt taken hvis backward
                if (btfnt_prediction != taken) {
                    mispredicts_btfnt++;
                }

                // dynamic predictiors
                for (int i = 0; i < 4; i++) {
                    uint32_t pc_idx = program_counter >> 2; // Fjerner de to null-bits
                    int mask = sizes[i] - 1;

                    // Bimodal logic
                    uint32_t b_idx = pc_idx & mask;
                    handle_prediction(&bimodal_preds[i], b_idx, taken);

                    // gShare logic
                    uint32_t g_idx = (pc_idx ^ gshare_ghrs[i]) & mask;
                    handle_prediction(&gshare_preds[i], g_idx, taken);

                    // Opdater Global History Register for gShare
                    gshare_ghrs[i] = (gshare_ghrs[i] << 1) | (taken ? 1 : 0);
                }

                if (taken) {
                    program_counter += immB;
                    regs[0] = 0;
                    continue;
                }
                break;
            }
            case 0x6F: { //jal
        //        int32_t imm = 
        //                ((instruction >> 11) & 0x00100000) |   // bit 20 (sign)
        //                ((instruction >> 20) & 0x00000800) |   // bit 11
        //                ((instruction >>  0) & 0x000FF000) |   // bit 19:12
        //                ((instruction >> 21) & 0x000007FE);    // bit 10:1
        //                if (imm & 0x00100000) imm |= 0xFFE00000;  // sign-extend
                int32_t imm20    = (instruction >> 31) & 0x1;
                int32_t imm10_1  = (instruction >> 21) & 0x3FF;
                int32_t imm11    = (instruction >> 20) & 0x1;
                int32_t imm19_12 = (instruction >> 12) & 0xFF;

                int32_t imm = (imm20 << 20) | (imm19_12 << 12) | (imm11 << 11) | (imm10_1 << 1);

                if (imm & 0x100000) {
                    imm |= 0xFFE00000;
                }

                if (rd != 0) {
                    regs[rd] = program_counter + 4;   // ← returadresse!
                }

                program_counter += imm;  // ← HOP! (-4 fordi vi tilføjer 4 senere)
                regs[0] = 0;
                continue;
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
        program_counter += 4;
    }
}
