#include <memory.h>
#include <simulate.h>
#include <stdio.h>

struct Stat simulate(struct memory *mem, int start_addr, FILE *log_file, struct symbols* symbols){
    int32_t regs[32] = {0};
    int program_counter = start_addr;
    long int instruction_count = 0;
    if (log_file != NULL){
        printf("Hej");

    }
}
