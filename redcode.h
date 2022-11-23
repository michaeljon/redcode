#ifndef _REDCODE_H
#define _REDCODE_H

// size of memory we support
#define CORESIZE 1024
#define OFFSET(p, a) ((((p + a) % CORESIZE) + CORESIZE) % CORESIZE)

// length of a line in the source code
#define INSTSIZE 80

// instructions values for the assembler
enum OPCODES
{
    OPCODE_ERR = -1,

    OPCODE_DAT = 0,
    OPCODE_MOV = 1,
    OPCODE_ADD = 2,
    OPCODE_SUB = 3,
    OPCODE_JMP = 4,
    OPCODE_JMZ = 5,
    OPCODE_JMG = 6,
    OPCODE_DJZ = 7,
    OPCODE_CMP = 8,
};

// address mode values for the assembler
enum ADDRMODES
{
    MODE_IMMEDIATE = 0,
    MODE_DIRECT = 1,
    MODE_INDIRECT = 2
};

// used by the disassembler for extracting the instrution components
enum MASKS
{
    PLAYER_MASK = 0x80000000,
    PLAYER_UNMASK = 0x7fffffff,

    TAKEN_MASK = 0x40000000,
    TAKEN_UNMASK = 0xbfffffff,

    OPCODE_MASK = 0x0f000000,
    OP_A_ADDRMODE_MASK = 0x00C00000,
    OP_A_ADDR_MASK = 0x003ff000,
    OP_B_ADDRMODE_MASK = 0x00000c00,
    OP_B_ADDR_MASK = 0x000003ff
};

// places in an instruction word where the various components fit
enum OFFSETS
{
    OFFSET_PLAYER = 31,
    OFFSET_OWNER = 30,

    OFFSET_OPCODE = 24,
    OFFSET_A_ADDRMODE = 22,
    OFFSET_A_ADDR = 12,
    OFFSET_B_ADDRMODE = 10,
    OFFSET_B_ADDR = 0,

    OFFSET_BASE_COMMON = 10,
    OFFSET_BASE_A = 12,
    OFFSET_BASE_B = 0,
};

// number of opcodes and address modes, plus places to find them for the disassembler / loader
#define OPCODE_COUNT 10
extern const char *OPCODES[];

#define ADDRMODE_COUNT 3
extern const char *ADDRMODES[];

extern int program_size;
extern unsigned long PROGRAM[];

// set and clear "owner" of a cell, for display purposes
#define REMOVE_OWNER(c) (c & PLAYER_UNMASK)
#define SET_OWNER(c, p) (c = (unsigned long)((c | (p << OFFSET_PLAYER)) & 0xffffffff))

#define CLEAR_OWNED(c) (c & TAKEN_UNMASK)
#define SET_OWNED(c) (c = (unsigned long)((c | (1 << OFFSET_OWNER)) & 0xffffffff))

// move the player p's program counter
#define STEP(p) (programCounters[p] = (programCounters[p] + 1) % CORESIZE)

// these macros assume we have 10 bits of storage
// where the high order bit is the sign bit, meaning
// really, we only have 9 bits to play with

#define MAX_VALUE 0x1ff
#define MIN_VALUE -MAX_VALUE

#define TOSTORAGE(_x) ((_x >= 0) ? _x : (~_x + 1) | 0x200)
#define FROMSTORAGE(_x) ((_x & 0x200) == 0 ? _x : ~((_x & 0x1ff) - 1))
#define DECODE(_s, _x) (FROMSTORAGE((_s == 0) ? (_x & OP_A_ADDR_MASK) >> OFFSET_A_ADDR : (_x & OP_B_ADDR_MASK) >> OFFSET_B_ADDR))

// define how faster the app should run
#define INST_PER_SECOND 32

#endif