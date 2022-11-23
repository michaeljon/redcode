#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/errno.h>
#include <unistd.h>

#include "redcode.h"

char *trimString(char *str)
{
    char *end;

    while (isspace((unsigned char)*str))
        str++;

    if (*str == 0)
        return str;

    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;

    end[1] = '\0';

    return str;
}

int parse_opcode(char *inst)
{
    for (int i = 0; i < OPCODE_COUNT; i++)
    {
        if (strncasecmp(OPCODES[i], inst, 3) == 0)
            return i;
    }

    return OPCODE_ERR;
}

int parse_address_mode(char *arg)
{
    if (*arg == '@')
        return MODE_INDIRECT;
    if (*arg == '#')
        return MODE_IMMEDIATE;

    return MODE_DIRECT;
}

int parse_address_value(char *arg)
{
    short val = atoi(arg) & 0xffff;

    if (val > MAX_VALUE || val < MIN_VALUE)
    {
        printf("value %d out of range [%d, %d]\n", val, MAX_VALUE, MIN_VALUE);
        exit(2);
    }

    return val >= 0 ? val : (~val + 1) | 0x200;
}

unsigned long parse_arg(char *arg, int base)
{
    unsigned long argument = 0;

    int addressMode = parse_address_mode(arg);
    if (addressMode != MODE_DIRECT)
    {
        arg++;
    }

    argument |= addressMode << (base + OFFSET_BASE_COMMON);

    int argVal = parse_address_value(arg);

    argument |= argVal << base;

    return argument;
}

unsigned long assemble_instruction(char *inst, int line)
{
    unsigned long word = 0;

    char *operation = strsep(&inst, " \t");
    int op = parse_opcode(operation);
    if (op == OPCODE_ERR)
    {
        fprintf(stderr, "Unknown opcode %s at line %d\n", operation, line);
        return -1;
    }

    word |= (op << OFFSET_OPCODE);

    // could use some better error handling in here
    if (op == OPCODE_DAT)
    {
        char *argB = strsep(&inst, " \t");
        word |= parse_arg(argB, OFFSET_BASE_B);
    }
    else if (op == OPCODE_JMP)
    {
        char *argA = strsep(&inst, " \t");
        word |= parse_arg(argA, OFFSET_BASE_A);
    }
    else
    {
        char *argA = strsep(&inst, " \t");
        word |= parse_arg(argA, OFFSET_BASE_A);

        char *argB = strsep(&inst, " \t");
        word |= parse_arg(argB, OFFSET_BASE_B);
    }

    return word;
}

int assemble(FILE *fpin)
{
    int pc = 0;
    char buffer[INSTSIZE];
    int err;
    int line = 0;

    while (fgets(buffer, INSTSIZE, fpin) != NULL)
    {
        if ((err = ferror(fpin)))
        {
            fprintf(stderr, "Error %d reading file\n", err);
            return -1;
        }

        line++;
        char *inst = trimString(buffer);
        if (strlen(inst) == 0)
            continue;

        // we're extending the langugage and adding comments
        if (*inst == ';')
            continue;

        // parse the instruction
        unsigned long word = assemble_instruction(inst, line);

        if (word == -1)
        {
            return -1;
        }

        core[pc++] = word;

        if (pc > CORESIZE)
        {
            fprintf(stderr, "Out of memory\n");
            return 1;
        }
    }

    return pc;
}

void output(FILE *fp)
{
    fwrite((void *)&program_size, sizeof(program_size), 1, fp);
    fwrite((void *)core, sizeof(core[0]), program_size, fp);
}

void usage(char *program)
{
    printf("usage: %s -i program.r -o program.a\n", program);
    exit(1);
}

int main(int argc, char **argv)
{
    int ch;
    char *program = argv[0];

    FILE *fpin;
    FILE *fpout;

    while ((ch = getopt(argc, argv, "i:o:")) != -1)
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

        case 'o':
            if ((fpout = fopen(optarg, "wb")) < 0)
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

    if ((program_size = assemble(fpin)) > 0)
    {
        output(fpout);
    }

    if (fpin)
    {
        fclose(fpin);
    }

    if (fpout)
    {
        fclose(fpout);
    }
}