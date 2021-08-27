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
    ++n_main_memory_reads;
    return main_memory[address];
}

void write_memory(AWORD address, AWORD value)
{
    ++n_main_memory_writes;
    main_memory[address] = value;
}

//  -------------------------------------------------------------------

// NOTE: Debug utility which prints out the bottom n elements 
// of main_memory.
void DEBUG_print_memory(int n)
{
    for(int i = 0; i < n; ++i)
    {
        printf("%i, ", main_memory[i]);
    }
    printf("\n");
}

// Prints n elements of the stack
void DEBUG_print_tos(int n)
{
    int k = N_MAIN_MEMORY_WORDS;
    printf("\nTOS: (address: value)\n");
    for(int i = k; i > (k - n); --i)
    {
        printf("%i: %i, ", i, main_memory[i]);
    }
    printf("\n\n");
}

//  -------------------------------------------------------------------

//  EXECUTE THE INSTRUCTIONS IN main_memory[]
int execute_stackmachine(void)
{
//  THE 3 ON-CPU CONTROL REGISTERS:
    int PC      = 0;                    // 1st instruction is at address=0
    int SP      = N_MAIN_MEMORY_WORDS;  // initialised to top-of-stack
    int FP      = 0;                    // frame pointer

// Since we can't declare new variables inside each case in the switch statement,
// is this the best solution?
    IWORD value1, value2;
    AWORD address;
    AWORD returnVal;
    IWORD value;

    DEBUG_print_memory(15);
    while(true) {

//  FETCH THE NEXT INSTRUCTION TO BE EXECUTED
        IWORD instruction   = read_memory(PC);
        ++PC;

        printf("\n>> %s\n", INSTRUCTION_name[instruction]);
//        printf("SP: %i\nPC: %i\nFP: %i\n", SP, PC, FP);
//        DEBUG_print_tos(4);

        if(instruction == I_HALT) {
            break;
        }

        switch(instruction)
        {
// No operation: PC advanced to the next instruction.
            case I_NOP:
                continue;

// Add: Two integers on TOS popped and added. Result is left on the TOS.
            case I_ADD:
                value1 = read_memory(SP++);
                value2 = read_memory(SP);
                write_memory(SP, value1 + value2);
                printf("Arithmetic I_ADD: %i + %i\n", value1, value2);
                break;

// Subtract: Two integers on TOS popped, second subtracted from first. Result is left on the TOS.
            case I_SUB:
                value1 = read_memory(SP++);
                value2 = read_memory(SP);
                write_memory(SP, value1 - value2);
                printf("Arithmetic I_SUB: %i - %i\n", value1, value2);
                break;

//Multiply: Two integers on TOS popped and multiplied. Result is left on TOS.
            case I_MULT:
                value1 = read_memory(SP++);
                value2 = read_memory(SP);
                write_memory(SP, value1 * value2);
                printf("Arithmetic I_MULT: %i * %i\n", value1, value2);
                break;

//Div: Two integers on TOS popped, second divided from first. Result is left on the TOS.
            case I_DIV:
                value1 = read_memory(SP++);
                value2 = read_memory(SP);
                write_memory(SP, value1 / value2);
                printf("Arithmetic I_DIV: %i / %i\n", value1, value2);
                break;

// Call: WIP
            case I_CALL:
                address = read_memory(PC++);
                FP = PC;
                PC = address;
                printf("calling function at address: %i\n", address);
                break;

// Return: WIP
            case I_RETURN:
                returnVal = read_memory(SP);
                printf("return from function with value: %i\n", returnVal);

                PC = FP;
                // return.cool says on the coolc website: 
                //      "return value (on TOS) to be copied to FP+1"
                // but i feel like i'm doing something wrong here:
                write_memory(SP, returnVal);   // write returnVal to TOS
                write_memory(FP+1, returnVal); // write returnVal to FP+1
                break;

// Unconditional jump: Flow of execution jumps to the next specified address.
            case I_JMP:
                PC = read_memory(PC);
                break;

// Conditional jump:  Value at TOS popped. Iff the value is zero, flow of execution jumps to the next specified address.
            case I_JEQ:
                value = read_memory(SP);
                write_memory(SP, 0);

                if(value == 0)
                    PC = read_memory(PC);
                break;

// Print integer: Value at TOS popped and printed to stdout.
            case I_PRINTI:
                printf("%u \n", SP);
                write_memory(SP, 0);
                break;

// Print String: Print the next NULL-byte terminated character string. WIP
            case I_PRINTS:
                break;

// Push Constant: This should push an integer constant onto the stack.
            case I_PUSHC:
                value = read_memory(PC++);
                write_memory(--SP, value);
                printf("pushing onto stack a value of: %i\n", value);
                break;

// Push Absolute: Push the integer in the address, which is specified in the word immediately after the push declaration..
            case I_PUSHA:
                // TODO: fix
                address = read_memory(PC++);
                value = read_memory(address);
                write_memory(--SP, value);
                printf("pushing onto stack a value of: %i\n", value);
                break;

// Push Relative: Push the integer in the next word, which specifies an address that is the frame pointer + offset.
            case I_PUSHR:
                write_memory(SP--, read_memory(FP + read_memory(PC++)));
                break;

// Pop Absolute: Pop the integer in the address, which is specified in the word immediately after the pop declaration.
            case I_POPA:
                write_memory(read_memory(PC++), 0);
                break;

// Pop Relative: Pop the integer in the next word, which specifies an address that is the frame pointer + offset.
            case I_POPR:
                write_memory(FP + read_memory(PC++), 0);
                break;

            //default: continue;
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
        n_main_memory_writes += fread(main_memory, sizeof(AWORD),
                                      N_MAIN_MEMORY_WORDS, file);
        fclose(file);
    }
    else
    {
        fprintf(stderr, "ERROR: Cannot open file %s.\n", filename);
        exit(EXIT_FAILURE);
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

// DEBUG: print TOS
    printf("\nExit code: %i\n", result);
///////////////////

    return result;          // or  exit(result);
}
