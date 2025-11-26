#include <memory.h>
#include <simulate.h>
#include <stdio.h>
#include <stdbool.h>

struct Stat simulate(struct memory *mem, int start_addr, FILE *log_file, struct symbols* symbols){
    int32_t regs[32] = {0};
    u_int32_t program_counter = start_addr;
    long int instruction_count = 0;
    bool done = false;

    while (1){
        u_int32_t instruktion = memory_rd_w(mem, program_counter);
        u_int32_t opcode = instruktion & 0x7F;

        instruction_count++;
        switch(opcode){
            case 0x73:
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

            case 0x33: 
                decode_R();
                break;

            case 0x13:
            case 0x03: 
            case 0x67:
                decode_I();
                break;

            case 0x23:
                decode_s();
                break;

            case 0x63:
                decode_B();
                break;

            case 0x6F:
                decode_J();
                break;

            case 0x17:
            case 0x37:
                decode_U();
                break;

            default: 
                printf("Ukendt opcode: 0x%x\n", opcode);
                done = true;
                break;
            }

        program_counter += 4;
    }
}
