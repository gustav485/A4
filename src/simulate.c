#include <memory.h>
#include <simulate.h>
#include <stdio.h>
#include <stdbool.h>

struct Stat simulate(struct memory *mem, int start_addr, FILE *log_file, struct symbols* symbols){
    int32_t regs[32] = {0};
    int program_counter = start_addr;
    long int instruction_count = 0;
    bool done = false;

    if (log_file != NULL){
        printf("Hej");
    }

    u_int32_t systemkald = regs[17];
    switch (systemkald){
        case 1:
            regs[10] = getchar();
            break;
        case 2:
            putchar(regs[10]);
            break;
        case 3:
            break;
        case 93:
            break;
        
        default:
            printf("Error, ukendt systemkald: %u", systemkald);
            break;
    }
    switch (opcode) {
        case 0x33: 
            decode_R()
            break;
        case 0x13:
            decode_I()
            break;
        case 0x03: 
            decode_I()
            break;
        case 0x23:
            decode_s()
            break;
        case 0x63:
            decode_B()
            break;
        case 0x6F:
            decode_J()
            break;
        case 0x67:
            decode_I()
            break;
        case 0x17:
            decode_U()
            break;
        case 0x37:
            decode_U()
            break;
        default: 
            printf("Ukendt opcode: 0x%x\n", opcode);
            done = true;
        break;
    }

}
