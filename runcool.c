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

// TODO REMOVE
int n_number_of_instructions   = 0;
int n_number_of_function_calls = 0;
///////////

void report_statistics(void)
{
    printf("@number-of-main-memory-reads\t%i\n",    n_main_memory_reads);
    printf("@number-of-main-memory-writes\t%i\n",   n_main_memory_writes);
    printf("@number-of-cache-memory-hits\t%i\n",    n_cache_memory_hits);
    printf("@number-of-cache-memory-misses\t%i\n",  n_cache_memory_misses);

    // TODO REMOVE
    printf("\n@number-of-instructions\t\t%i\n", n_number_of_instructions);
    printf("@number-of-function-calls\t%i\n", n_number_of_function_calls);
    /////////
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
void DEBUG_print_tos(int n, int SP)
{
    int k = N_MAIN_MEMORY_WORDS;
    printf("\nStack -> address: value\n\t");
    for(int i = k; i > (k - n); --i)
    {
        printf("%i: %i, ", i, main_memory[i]);
    }
    printf("\nTOS -> %i: %i\n\n", SP, main_memory[SP]);
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
    AWORD address;
    AWORD valueStr;
    uint8_t bytes[2];
    IWORD returnVal;
    IWORD value, value2;
    
    DEBUG_print_memory(25);
    while(true) {

//  FETCH THE NEXT INSTRUCTION TO BE EXECUTED
        IWORD instruction = read_memory(PC);
        ++PC;

        ++n_number_of_instructions;

        //printf("################################\n");
        //printf("\n>> %s\n", INSTRUCTION_name[instruction]);
        //printf("SP: %i\nPC: %i\nFP: %i\n", SP, PC, FP);
        //DEBUG_print_tos(9, SP);

        if(instruction == I_HALT) {
            break;
        }

        switch(instruction)
        {
// No operation: PC advanced to the next instruction.
            case I_NOP:
                break;

// Add: Two integers on TOS popped and added. Result is left on the TOS.
            case I_ADD:
                value = read_memory(SP++);
                value2 = read_memory(SP);
                write_memory(SP, value2 + value);
                //printf("Arithmetic I_ADD: %i + %i\n", value2, value);
                break;

// Subtract: Two integers on TOS popped, second subtracted from first. Result is left on the TOS.
            case I_SUB:
                value = read_memory(SP++);
                value2 = read_memory(SP);
                write_memory(SP, value2 - value);
                //printf("Arithmetic I_SUB: %i - %i\n", value2, value);
                break;

//Multiply: Two integers on TOS popped and multiplied. Result is left on TOS.
            case I_MULT:
                value = read_memory(SP++);
                value2 = read_memory(SP);
                write_memory(SP, value2 * value);
                //printf("Arithmetic I_MULT: %i * %i\n", value2, value);
                break;

//Div: Two integers on TOS popped, second divided from first. Result is left on the TOS.
            case I_DIV:
                value = read_memory(SP++);
                value2 = read_memory(SP);
                write_memory(SP, value2 / value);
                //printf("Arithmetic I_DIV: %i / %i\n", value2, value);
                break;

// Call: Move PC to the next instruction to be executed, and set FP as required.
            case I_CALL:
                ++n_number_of_function_calls;
                // save the address of the next instruction onto the stack
                write_memory(--SP, PC + 1);
                
                // save the current value of FP onto the stack
                write_memory(--SP, FP);
                FP = SP;

                // move PC to the first instruction of the function we are calling
                PC = read_memory(PC);

                //printf("calling function at address: %i\n", PC);
                break;

// Return: WIP
            case I_RETURN:
                //DEBUG_print_tos(5, SP);
                //printf("SP: %i, PC: %i, FP: %i\n\n", SP, PC, FP);

                // read return value from TOS
                address = FP + read_memory(PC);
                returnVal = read_memory(SP);

                // restore PC and FP to continue execution of the calling function
                PC = read_memory(address);
                FP = read_memory(FP);

                // write return value to FP-offset
                write_memory(address, returnVal);

                //printf("addrs: %i, val: %i\n", address, returnVal);

                //DEBUG_print_tos(5, SP);
                //printf("SP: %i, PC: %i, FP: %i\n\n", SP, PC, FP);
                //printf("return from function with value: %i\n", returnVal);
                break;

// Unconditional jump: Flow of execution jumps to the next specified address.
            case I_JMP:
                PC = read_memory(PC);
                break;

// Conditional jump:  Value at TOS popped. Iff the value is zero, flow of execution jumps to the next specified address.
            case I_JEQ:
                value = read_memory(SP++);
                if(value == 0) { PC = read_memory(PC); }
                else { ++PC; }
                break;

// Print integer: Value at TOS popped and printed to stdout.
            case I_PRINTI:
                value = read_memory(SP);
                fprintf(stdout, "%i", value);
                break;

// Print String: Print the next NULL-byte terminated character string. 
            case I_PRINTS:
                address = read_memory(PC++);
                while(true)
                {
                    valueStr = read_memory(address++);
                    bytes[0] = valueStr & 0x00FF;
                    bytes[1] = valueStr >> 8;
                    fprintf(stdout, "%c%c", bytes[0], bytes[1]);
                    if(bytes[0] == '\0' || bytes[1] == '\0') {break;}
                }
                break;

// Push Constant: This should push an integer constant onto the stack.
            case I_PUSHC:
                value = read_memory(PC++);
                write_memory(--SP, value);
                //printf("pushing onto stack a value of: %i\n", value);
                break;

// Push Absolute: Push the integer in the address, which is specified in the word immediately after the push declaration..
            case I_PUSHA:
                address = read_memory(PC++);
                value = read_memory(address);
                write_memory(--SP, value);
                //printf("pushing onto stack a value of %i from address %i.\n", value, address);
                break;

// Push Relative: Push the integer in the next word, which specifies an address that is the frame pointer + offset.
            case I_PUSHR:
                address = read_memory(PC++) + FP;
                value = read_memory(address);
                write_memory(--SP, value);
                //printf("pushing value of %i from address %i.\n", value, address);
                break;

// Pop Absolute: A value from the TOS is popped. The next word provides the address to the offset from the TOS.
            case I_POPA:
                address = read_memory(PC++);
                value = read_memory(SP++);
                write_memory(address, value);
                //printf("pop to address: %i, with value: %i\n", address, value);
                break;

// Pop Relative: A value from the TOS is popped. The next word provides the offset added to the FP, which provides an address to the offset from the TOS popped.
            case I_POPR:
                address = read_memory(PC++) + FP;
                value = read_memory(SP++);
                write_memory(address, value);
                //printf("pop to address: %i, with value: %i\n", address, value);
                break;
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
        fread(main_memory, sizeof(AWORD), N_MAIN_MEMORY_WORDS, file);
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
