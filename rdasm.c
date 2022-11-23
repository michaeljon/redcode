#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/errno.h>
#include <unistd.h>

#include "redcode.h"

void disassemble_instruction(unsigned long instruction, int offset)
{
    int opcode_val = (instruction & OPCODE_MASK) >> OFFSET_OPCODE;
    int addrModeA = (instruction & OP_A_ADDRMODE_MASK) >> OFFSET_A_ADDRMODE;
    int addrA = (instruction & OP_A_ADDR_MASK) >> OFFSET_A_ADDR;
    int addrModeB = (instruction & OP_B_ADDRMODE_MASK) >> OFFSET_B_ADDRMODE;
    int addrB = (instruction & OP_B_ADDR_MASK) >> OFFSET_B_ADDR;

    addrA = FROMSTORAGE(addrA);
    addrB = FROMSTORAGE(addrB);

    if (opcode_val == OPCODE_DAT)
    {
        printf("%04d: %s\t\t%s%d\n",
               offset,
               OPCODES[opcode_val],
               ADDRMODES[addrModeB], addrB);
    }
    else if (opcode_val == OPCODE_JMP)
    {
        printf("%04d: %s\t%s%d\n",
               offset,
               OPCODES[opcode_val],
               ADDRMODES[addrModeA], addrA);
    }
    else
    {
        printf("%04d: %s\t%s%d\t%s%d\n",
               offset,
               OPCODES[opcode_val],
               ADDRMODES[addrModeA], addrA,
               ADDRMODES[addrModeB], addrB);
    }
}

void disassemble()
{
    for (int pc = 0; pc < program_size; pc++)
    {
        disassemble_instruction(PROGRAM[pc], pc);
    }
}

void usage(char *program)
{
    printf("usage: %s -i program.o\n", program);
    exit(1);
}

int main(int argc, char **argv)
{
    int ch;
    char *program = argv[0];

    FILE *fpin;

    while ((ch = getopt(argc, argv, "i:")) != -1)
    {
        switch (ch)
        {
        case 'i':
            if ((fpin = fopen(optarg, "r")) < 0)
            {
                (void)fprintf(stderr, "%s: %s: %s\n", program, optarg, strerror(errno));
                exit(1);
            }
            break;

        case '?':
        default:
            usage(program);
        }
    }

    int read = fread(&program_size, sizeof(program_size), 1, fpin);
    if (read > 0)
    {
        read = fread((void *)PROGRAM, sizeof(unsigned long), program_size, fpin);
        if (read == program_size)
        {
            disassemble();
        }
        else
        {
            fprintf(stderr, "Failed to read program from stream, read %d, expected %d\n", read, program_size);
        }
    }
    else
    {
        fprintf(stderr, "Failed to read program size from stream\n");
    }

    if (fpin)
    {
        fclose(fpin);
    }
}