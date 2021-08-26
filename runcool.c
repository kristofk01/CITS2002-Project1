//  CITS2002 Project 1 2021
//  Name(s):             Daniel Ling   , Kristof Kovacs
//  Student number(s):   22896002 , 22869854

//  compile with:  cc -std=c11 -Wall -Werror -o runcool runcool.c

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//  THE STACK-BASED MACHINE HAS 2^16 (= 65,536) WORDS OF MAIN MEMORY
#define N_MAIN_MEMORY_WORDS (1<<16)

//  EACH WORD OF MEMORY CAN STORE A 16-bit UNSIGNED ADDRESS (0 to 65535)
#define AWORD               uint16_t
//  OR STORE A 16-bit SIGNED INTEGER (-32,768 to 32,767)
#define IWORD               int16_t

//  THE ARRAY OF 65,536 WORDS OF MAIN MEMORY
AWORD                       main_memory[N_MAIN_MEMORY_WORDS];

//  THE SMALL-BUT-FAST CACHE HAS 32 WORDS OF MEMORY
#define N_CACHE_WORDS       32


//  see:  https://teaching.csse.uwa.edu.au/units/CITS2002/projects/coolinstructions.php
enum INSTRUCTION {
    I_HALT       = 0,
    I_NOP,
    I_ADD,
    I_SUB,
    I_MULT,
    I_DIV,
    I_CALL,
    I_RETURN,
    I_JMP,
    I_JEQ,
    I_PRINTI,
    I_PRINTS,
    I_PUSHC,
    I_PUSHA,
    I_PUSHR,
    I_POPA,
    I_POPR
};

//  USE VALUES OF enum INSTRUCTION TO INDEX THE INSTRUCTION_name[] ARRAY
const char *INSTRUCTION_name[] = {
    "halt",
    "nop",
    "add",
    "sub",
    "mult",
    "div",
    "call",
    "return",
    "jmp",
    "jeq",
    "printi",
    "prints",
    "pushc",
    "pusha",
    "pushr",
    "popa",
    "popr"
};

//  ----  IT IS SAFE TO MODIFY ANYTHING BELOW THIS LINE  --------------


//  THE STATISTICS TO BE ACCUMULATED AND REPORTED
int n_main_memory_reads     = 0;
int n_main_memory_writes    = 0;
int n_cache_memory_hits     = 0;
int n_cache_memory_misses   = 0;

void report_statistics(void)
{
    printf("@number-of-main-memory-reads\t%i\n",    n_main_memory_reads);
    printf("@number-of-main-memory-writes\t%i\n",   n_main_memory_writes);
    printf("@number-of-cache-memory-hits\t%i\n",    n_cache_memory_hits);
    printf("@number-of-cache-memory-misses\t%i\n",  n_cache_memory_misses);
}

//  -------------------------------------------------------------------

//  EVEN THOUGH main_memory[] IS AN ARRAY OF WORDS, IT SHOULD NOT BE ACCESSED DIRECTLY.
//  INSTEAD, USE THESE FUNCTIONS read_memory() and write_memory()
//
//  THIS WILL MAKE THINGS EASIER WHEN WHEN EXTENDING THE CODE TO
//  SUPPORT CACHE MEMORY

AWORD read_memory(int address)
{
    return main_memory[address];
}

void write_memory(AWORD address, AWORD value)
{
    main_memory[address] = value;
}

//  -------------------------------------------------------------------

//  EXECUTE THE INSTRUCTIONS IN main_memory[]
int execute_stackmachine(void)
{
//  THE 3 ON-CPU CONTROL REGISTERS:
    int PC      = 0;                    // 1st instruction is at address=0
    int SP      = N_MAIN_MEMORY_WORDS;  // initialised to top-of-stack
    int FP      = 0;                    // frame pointer

//  REMOVE THE FOLLOWING LINE ONCE YOU ACTUALLY NEED TO USE FP
    FP = FP;

    while(true) {

//  FETCH THE NEXT INSTRUCTION TO BE EXECUTED
        IWORD instruction   = read_memory(PC);
        ++PC;

//      printf("%s\n", INSTRUCTION_name[instruction]);

        if(instruction == I_HALT) {
            break;
        }

        //No operation: PC advanced to the next instruction.
       if(instruction == I_NOP) {
           ;
       }

//Add: Two integers on TOS popped and added. Result is left on the TOS.
       if(instruction == I_ADD) {
           int value1 = read_memory(SP);
           write_memory(SP++, 0);

           int value2 = read_memory(SP);
           write_memory(SP, value1 + value2);
       }

//Subtract: Two integers on TOS popped, second subtracted from first. Result is left on the TOS.
       if(instruction == I_SUB) {
           int value1 = read_memory(SP);
           write_memory(SP++, 0);

           int value2 = read_memory(SP);
           write_memory(SP, value1 - value2);
       }

//Multiply: Two integers on TOS popped and multiplied. Result is left on TOS.
       if(instruction == I_MULT) {
           int value1 = read_memory(SP);
           write_memory(SP++, 0);

           int value2 = read_memory(SP);
           write_memory(SP, value1 * value2);
       }

//Div: Two integers on TOS popped, second divided from first. Result is left on the TOS.
       if(instruction == I_DIV) {
           int value1 = read_memory(SP);
           write_memory(SP++, 0);

           int value2 = read_memory(SP);
           write_memory(SP, value1 / value2);
       }

//Function call: WIP

//Function return: WIP

//Unconditional jump: Flow of execution jumps to the next specified address.
        if(instruction == I_JMP) {
            PC = read_memory(PC);
        }

//Conditional jump:  Value at TOS popped. Iff the value is zero, flow of execution jumps to the next specified address.
        if(instruction == I_JEQ) {
            int condition = read_memory(SP);
            write_memory(SP, 0);

            if(condition == 0) PC = read_memory(PC);
        }

//Print integer: Value at TOS popped and printed to stdout.
        if(instruction == I_PRINTI) {
            printf("%u \n", SP);
            write_memory(SP, 0);
        }

//Print String: Print the next NULL-byte terminated character string. WIP


// Push Constant: This should push an integer constant onto the stack.
        if(instruction == I_PUSHC) {
            write_memory(SP--, PC++);
        }

// Push Absolute: Push the integer in the address, which is specified in the word immediately after the push declaration..
        if(instruction == I_PUSHA) {
            write_memory(SP--, read_memory(PC++));
        }

// Push Relative: Push the integer in the next word, which specifies an address that is the frame pointer + offset.
        if(instruction == I_PUSHR) {
            write_memory(SP--, read_memory(FP + read_memory(PC++)));
        }

// Pop Absolute: Pop the integer in the address, which is specified in the word immediately after the pop declaration.
        if(instruction  == I_POPA) {
            write_memory(0, read_memory(PC++));
        }

// Pop Relative: Pop the integer in the next word, which specifies an address that is the frame pointer + offset.
        if(instruction == I_POPR) {
            write_memory(0, FP + read_memory(PC++));
        }

    }

//  THE RESULT OF EXECUTING THE INSTRUCTIONS IS FOUND ON THE TOP-OF-STACK
    return read_memory(SP);
}

//  -------------------------------------------------------------------

//  READ THE PROVIDED coolexe FILE INTO main_memory[]
void read_coolexe_file(char filename[])
{
    memset(main_memory, 0, sizeof main_memory);   //  clear all memory

//  READ CONTENTS OF coolexe FILE
    FILE *file = fopen(filename, "rb");
    if(file)
    {
        fread(main_memory, sizeof(AWORD), sizeof(main_memory), file);
        fclose(file);
    }
    else
    {
        fprintf(stderr, "ERROR: Cannot open file %s.\n", filename);
    }
}

//  -------------------------------------------------------------------

int main(int argc, char *argv[])
{
//  CHECK THE NUMBER OF ARGUMENTS
    if(argc != 2) {
        fprintf(stderr, "Usage: %s program.coolexe\n", argv[0]);
        exit(EXIT_FAILURE);
    }

//  READ THE PROVIDED coolexe FILE INTO THE EMULATED MEMORY
    read_coolexe_file(argv[1]);

//  EXECUTE THE INSTRUCTIONS FOUND IN main_memory[]
    int result = execute_stackmachine();

    report_statistics();

    return result;          // or  exit(result);
}
