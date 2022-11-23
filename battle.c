#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/errno.h>
#include <unistd.h>
#include <time.h>

#include "redcode.h"

const char *PLAYER1_COLOR = "\x1b[91;107m";
const char *PLAYER2_COLOR = "\x1b[94;107m";

const int row_length = 0x10;

int iterations = 0;

static char instruction_buffer[INSTSIZE];

typedef enum
{
    A = 0,
    B
} which;

char *disassemble_instruction(unsigned long instruction)
{
    int owner = (instruction & PLAYER_MASK) >> OFFSET_PLAYER;

    int opcode_val = (instruction & OPCODE_MASK) >> OFFSET_OPCODE;
    int addrModeA = (instruction & OP_A_ADDRMODE_MASK) >> OFFSET_A_ADDRMODE;
    int addrA = (instruction & OP_A_ADDR_MASK) >> OFFSET_A_ADDR;
    int addrModeB = (instruction & OP_B_ADDRMODE_MASK) >> OFFSET_B_ADDRMODE;
    int addrB = (instruction & OP_B_ADDR_MASK) >> OFFSET_B_ADDR;

    addrA = from_storage(addrA);
    addrB = from_storage(addrB);

    if (opcode_val == OPCODE_DAT)
    {
        snprintf(
            instruction_buffer,
            INSTSIZE,
            "%d: %s\t\t%s%d\n",
            owner,
            OPCODES[opcode_val],
            ADDRMODES[addrModeB], addrB);
    }
    else if (opcode_val == OPCODE_JMP)
    {
        snprintf(
            instruction_buffer,
            INSTSIZE,
            "%d: %s\t%s%d\n",
            owner,
            OPCODES[opcode_val],
            ADDRMODES[addrModeA], addrA);
    }
    else
    {
        snprintf(
            instruction_buffer,
            INSTSIZE,
            "%d: %s\t%s%d\t%s%d\n",
            owner,
            OPCODES[opcode_val],
            ADDRMODES[addrModeA], addrA,
            ADDRMODES[addrModeB], addrB);
    }

    return instruction_buffer;
}

int die(int player, int modeA, int addrA, int modeB, int addrB)
{
    printf("\x1b[%d;0H Player %d has lost by stepping on a NULL in iteration %d", 65 + player, player, iterations);
    printf("\x1b[67;0H\n");
    return 1;
}

int lea(int player, int mode, int value)
{
    int pc = programCounters[player];

    switch (mode)
    {
    case MODE_IMMEDIATE:
        return value;

    case MODE_DIRECT:
        return clear_owner_flag(offset(pc, value));

    case MODE_INDIRECT:
    {
        int addr = core[offset(pc, value)];
        int ownerless_addr = clear_owner_flag(addr);
        return offset(pc, ownerless_addr);
    }

    default:
        fprintf(stderr, "Invalid addressing mode %d\n", mode);
        exit(1);
    }
}

unsigned long operand(int player, int mode, int value)
{
    int pc = programCounters[player];

    switch (mode)
    {
    case MODE_IMMEDIATE:
        return value;

    case MODE_DIRECT:
        return clear_owner_flag(core[offset(pc, value)]);

    case MODE_INDIRECT:
    {
        int addr = core[offset(pc, value)];
        int ownerless_addr = clear_owner_flag(addr);
        return clear_owner_flag(core[offset(pc, ownerless_addr)]);
    }

    default:
        fprintf(stderr, "Invalid addressing mode %d\n", mode);
        exit(1);
    }
}

int dat(int player, int modeA, int addrA, int modeB, int addrB)
{
    printf("\x1b[%d;0H Player %d has lost by executing a DAT in iteration %d", 65 + player, player, iterations);
    printf("\x1b[67;0H\n");
    return 1;
}

int mov(int player, int modeA, int addrA, int modeB, int addrB)
{
    unsigned long valueA = operand(player, modeA, addrA);
    int address = lea(player, modeB, addrB);

    core[address] = valueA;
    set_owner(&core[address], player);
    increment_pc(player);

    return 0;
}

int add(int player, int modeA, int addrA, int modeB, int addrB)
{
    unsigned long valueA = operand(player, modeA, addrA);
    unsigned long valueB = operand(player, modeB, addrB);
    int address = lea(player, modeB, addrB);

    int valA = modeA == MODE_IMMEDIATE ? valueA : decode(PLAYER1, valueA);
    int valB = modeB == MODE_IMMEDIATE ? valueB : decode(PLAYER2, valueB);

    core[address] = valA + valB;
    set_owner(&core[address], player);

    increment_pc(player);

    return 0;
}

int sub(int player, int modeA, int addrA, int modeB, int addrB)
{
    unsigned long valueA = operand(player, modeA, addrA);
    unsigned long valueB = operand(player, modeB, addrB);
    int address = lea(player, modeB, addrB);

    int valA = modeA == MODE_IMMEDIATE ? valueA : decode(PLAYER1, valueA);
    int valB = modeB == MODE_IMMEDIATE ? valueB : decode(PLAYER2, valueB);

    core[address] = valB - valA;
    set_owner(&core[address], player);

    increment_pc(player);

    return 0;
}

int jmp(int player, int modeA, int addrA, int modeB, int addrB)
{
    int address = lea(player, modeA, addrA);

    programCounters[player] = address;

    return 0;
}

int jmz(int player, int modeA, int addrA, int modeB, int addrB)
{
    unsigned long valueA = operand(player, modeA, addrA);
    unsigned long valueB = operand(player, modeB, addrB);

    int valA = modeA == MODE_IMMEDIATE ? valueA : decode(PLAYER1, valueA);
    int valB = modeB == MODE_IMMEDIATE ? valueB : decode(PLAYER2, valueB);

    increment_pc(player);

    if (valB == 0)
        programCounters[player] = valA;

    return 0;
}

int jmg(int player, int modeA, int addrA, int modeB, int addrB)
{
    unsigned long valueA = operand(player, modeA, addrA);
    unsigned long valueB = operand(player, modeB, addrB);

    int valA = modeA == MODE_IMMEDIATE ? valueA : decode(PLAYER1, valueA);
    int valB = modeB == MODE_IMMEDIATE ? valueB : decode(PLAYER2, valueB);

    increment_pc(player);

    if (valB > 0)
        programCounters[player] = valA;

    return 0;
}

int djz(int player, int modeA, int addrA, int modeB, int addrB)
{
    unsigned long valueA = operand(player, modeA, addrA);
    int address = lea(player, modeB, addrB);

    int valA = modeA == MODE_IMMEDIATE ? valueA : decode(PLAYER1, valueA);

    increment_pc(player);

    core[address]--;
    if (core[address] == 0)
        programCounters[player] = valA;

    set_owner(&core[address], player);

    return 0;
}

int cmp(int player, int modeA, int addrA, int modeB, int addrB)
{
    unsigned long valueA = operand(player, modeA, addrA);
    unsigned long valueB = operand(player, modeB, addrB);

    int valA = modeA == MODE_IMMEDIATE ? valueA : decode(PLAYER1, valueA);
    int valB = modeB == MODE_IMMEDIATE ? valueB : decode(PLAYER2, valueB);

    increment_pc(player);

    // move one more
    if (valA != valB)
        increment_pc(player);

    return 0;
}

int (*handlers[])(int player, int modeA, int addrA, int modeB, int addrB) = {
    dat,
    mov,
    add,
    sub,
    jmp,
    jmz,
    jmg,
    djz,
    cmp};

void printstats()
{
    printf("%sPlayer 1 PC 0x%03x\x1b[0m\n", PLAYER1_COLOR, programCounters[0]);
    printf("%sPlayer 2 PC 0x%03x\x1b[0m\n", PLAYER2_COLOR, programCounters[1]);
}

void showcore()
{
    // clear screen
    printf("\x1b[3J");

    // set cursor to 0,0
    printf("\x1b[0;0H");

    for (int r = 0; r < (1024 / row_length); r++)
    {
        printf("%03x: ", r * row_length);
        for (int c = 0; c < row_length; c++)
        {
            unsigned long instruction = core[r * row_length + c];
            int owned = instruction & TAKEN_MASK;

            if (owned)
            {
                int owner = (instruction & PLAYER_MASK) >> OFFSET_PLAYER;
                printf("%s", owner == 0 ? PLAYER1_COLOR : PLAYER2_COLOR);
                printf("%08.8lx", (unsigned long)(instruction)&0xffffffff);
                printf("\x1b[0m ");
            }
            else
            {
                printf("........ ");
            }
        }
        printf("\n");
    }
}

int execute(int player, unsigned long instruction)
{
    int opcode_val = (instruction & OPCODE_MASK) >> OFFSET_OPCODE;
    int addrModeA = (instruction & OP_A_ADDRMODE_MASK) >> OFFSET_A_ADDRMODE;
    int addrA = (instruction & OP_A_ADDR_MASK) >> OFFSET_A_ADDR;
    int addrModeB = (instruction & OP_B_ADDRMODE_MASK) >> OFFSET_B_ADDRMODE;
    int addrB = (instruction & OP_B_ADDR_MASK) >> OFFSET_B_ADDR;

    addrA = from_storage(addrA);
    addrB = from_storage(addrB);

    if (instruction == 0)
    {
        return die(player, addrModeA, addrA, addrModeB, addrB);
    }

    return handlers[opcode_val](player, addrModeA, addrA, addrModeB, addrB);
}

void runsimulation()
{
    while (1)
    {
        iterations++;

        for (int player = 0; player <= 1; player++)
        {
            unsigned long instruction = core[programCounters[player]];
            disassemble_instruction(instruction);

            showcore();
            int result = execute(player, instruction & PLAYER_UNMASK);

            if (result != 0)
            {
                exit(0);
                return;
            }

            printstats();

            // 1/2 second wait between steps
            usleep(1 * (1000 / INST_PER_SECOND) * 1000);
        }
    }
}

int load(FILE *fp, unsigned long *buffer)
{
    int player_program_size;

    int read = fread(&player_program_size, sizeof(player_program_size), 1, fp);
    if (read > 0)
    {
        read = fread((void *)buffer, sizeof(unsigned long), player_program_size, fp);
        if (read != player_program_size)
        {
            fprintf(stderr, "Failed to read program from stream, read %d, expected %d\n", read, player_program_size);
            return -1;
        }
    }
    else
    {
        fprintf(stderr, "Failed to read program size from stream\n");
        return -1;
    }

    return player_program_size;
}

void place(unsigned long *program, int size, int location, unsigned int player)
{
    // place the program in memory
    for (int ip = 0; ip < size; ip++)
    {
        core[(ip + location) % CORESIZE] = program[ip];
        set_owner(&core[(ip + location) % CORESIZE], player);
    }

    // set the pc for the player
    for (int ip = 0; ip < size; ip++)
    {
        // get the opcode from the loaded program
        int opcode_val = (program[ip] & OPCODE_MASK) >> OFFSET_OPCODE;
        if (opcode_val != OPCODE_DAT)
        {
            // if it's not an OPCODE_DAT then we can start there,
            // set the program counter to the location _in memory_
            programCounters[player] = (ip + location) % CORESIZE;
            return;
        }
    }
}

void usage(char *program)
{
    printf("usage: %s -1 player1.o -2 player2.o\n", program);
    exit(1);
}

int main(int argc, char **argv)
{
    int ch;
    char *program = argv[0];

    int seed = -1;
    FILE *fp1;
    FILE *fp2;

    while ((ch = getopt(argc, argv, "1:2:s:")) != -1)
    {
        switch (ch)
        {
        case '1':
            if ((fp1 = fopen(optarg, "rb")) < 0)
            {
                (void)fprintf(stderr, "%s: %s: %s\n", program, optarg, strerror(errno));
                exit(1);
            }
            break;

        case '2':
            if ((fp2 = fopen(optarg, "rb")) < 0)
            {
                (void)fprintf(stderr, "%s: %s: %s\n", program, optarg, strerror(errno));
                exit(1);
            }
            break;

        case 's':
            seed = atoi(optarg);
            break;

        case '?':
        default:
            usage(program);
        }
    }

    unsigned long pgm[CORESIZE];
    int length = load(fp1, pgm);
    if (length < 0)
        goto terminate;
    place(pgm, length, 0, 0U);

    if (seed != -1)
        srand(seed);
    else
        srand(time(NULL));

    length = load(fp2, pgm);
    if (length < 0)
        goto terminate;
    place(pgm, length, (rand() % CORESIZE), 1U);

    // starting positions
    showcore();
    runsimulation();

    printf("Simulation complete after %d iterations", iterations);

terminate:
    if (fp1)
    {
        fclose(fp1);
    }

    if (fp2)
    {
        fclose(fp2);
    }
}