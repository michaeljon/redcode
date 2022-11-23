#include "redcode.h"

// reverse-mapped opcodes for the disassembler / lister
const char *OPCODES[] = {
    "DAT",
    "MOV",
    "ADD",
    "SUB",
    "JMP",
    "JMZ",
    "JMG",
    "DJZ",
    "CMP",
};

// reverse-mapped addressing modes for the disassembler / lister
const char *ADDRMODES[] = {
    "#", // IMMEDIATE
    "",  // DIRECT
    "@"  // INDIRECT
};

int program_size = 0;
unsigned long PROGRAM[CORESIZE];
