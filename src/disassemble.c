#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "memory.h"        // nødvendigt for memory_rd_w
#include "disassemble.h"   // brug anførselstegn, ikke < >

// Rigtig funktion – matcher præcis disassemble.h
void disassemble(uint32_t addr, uint32_t instruction, char* result, size_t buf_size, struct symbols* symbols)
{
    // Midlertidig stub – bare så det linker og kører
    // Du skal senere lave en rigtig disassembler, men det her er nok til A4-delen
    (void)addr;         // undgår unused warning
    (void)symbols;      // undgår unused warning
    snprintf(result, buf_size, "%08x    <stub>", instruction);
}
