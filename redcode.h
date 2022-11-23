#ifndef _REDCODE_H
#define _REDCODE_H

// size of memory we support
#define CORESIZE 1024
// #define OFFSET(p, a) ((((p + a) % CORESIZE) + CORESIZE) % CORESIZE)

// length of a line in the source code
#define INSTSIZE 80

// instructions values for the assembler
typedef enum
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
} opcode_t;

// address mode values for the assembler
typedef enum
{
    MODE_IMMEDIATE = 0,
    MODE_DIRECT = 1,
    MODE_INDIRECT = 2
} addrmode_t;

typedef enum
{
    PLAYER1 = 0,
    PLAYER2 = 1
} player_t;

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
extern int programCounters[];

extern unsigned long core[];

// these macros assume we have 10 bits of storage
// where the high order bit is the sign bit, meaning
// really, we only have 9 bits to play with

#define MAX_VALUE 0x1ff
#define MIN_VALUE -MAX_VALUE

// define how faster the app should run
#define INST_PER_SECOND 32

/**
 * calculates an offset into our core given an address
 */
inline unsigned long offset(int pc, int addr)
{
    return ((((pc + addr) % CORESIZE) + CORESIZE) % CORESIZE);
}

/**
 * sets the owner and owned flag on a cell
 */
inline void set_owner(unsigned long *ploc, player_t player)
{
    *ploc = (unsigned long)((*ploc | (player << OFFSET_PLAYER)) & 0xffffffff);
    *ploc = (unsigned long)((*ploc | (1 << OFFSET_OWNER)) & 0xffffffff);
}

/**
 * clears the owner flag from a cell's value and returns it
 */
inline unsigned long clear_owner_flag(unsigned long loc)
{
    return loc & PLAYER_UNMASK;
}

/**
 * clears the owne flag from a cell's value and returns it
 */
inline unsigned long clear_owned_flag(unsigned long loc)
{
    return loc & TAKEN_UNMASK;
}

/**
 * handles value conversion
 */
inline short to_storage(unsigned long operand)
{
    return ((operand >= 0) ? operand : (~operand + 1) | 0x200);
}

inline short from_storage(unsigned long operand)
{
    return ((operand & 0x200) == 0 ? operand : ~((operand & 0x1ff) - 1));
}

inline short decode(player_t player, unsigned long operand)
{
    return (from_storage(
        (player == PLAYER1) ? (operand & OP_A_ADDR_MASK) >> OFFSET_A_ADDR : (operand & OP_B_ADDR_MASK) >> OFFSET_B_ADDR));
}

inline void increment_pc(player_t player)
{
    (programCounters[player] = (programCounters[player] + 1) % CORESIZE);
}

#endif