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
int programCounters[2];
unsigned long core[CORESIZE];

// inline functions
extern inline unsigned long offset(int pc, int addr);

extern inline void set_owner(unsigned long *ploc, player_t player);

extern inline unsigned long clear_owner_flag(unsigned long loc);
extern inline unsigned long clear_owned_flag(unsigned long loc);

extern inline short to_storage(unsigned long operand);
extern inline short from_storage(unsigned long operand);
extern inline short decode(player_t player, unsigned long operand);

extern inline void increment_pc(player_t player);