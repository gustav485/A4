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

        case 2:
            putchar(regs[10]);

        case 3:
            break;
        case 93:
            break;
        
        default:
            printf("Error, ukendt systemkald: %u", systemkald);
            break;
    }

}
