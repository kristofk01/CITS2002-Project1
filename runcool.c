//  CITS2002 Project 1 2021
//  Name(s):             Daniel Ling , Kristof Kovacs
//  Student number(s):   22896002    , 22869854

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

//  THE STATISTICS TO BE ACCUMULATED AND REPORTED
int n_main_memory_reads     = 0;
int n_main_memory_writes    = 0;
int n_cache_memory_hits     = 0;
int n_cache_memory_misses   = 0;

int n_number_of_instructions   = 0;
int n_number_of_function_calls = 0;
float n_percentage_of_cache_hits = 0;
int exit_status = 0;

void report_statistics(void) {
    printf("@number-of-main-memory-reads-(fast-jeq)  %i\n", n_main_memory_reads);
    printf("@number-of-main-memory-writes-(fast-jeq) %i\n", n_main_memory_writes);
    printf("@number-of-cache-memory-hits\t\t %i\n", n_cache_memory_hits);
    printf("@number-of-cache-memory-misses\t\t %i\n", n_cache_memory_misses);

    printf("\n@number-of-instructions\t\t%i\n", n_number_of_instructions);
    printf("@number-of-function-calls\t%i\n", n_number_of_function_calls);

    n_percentage_of_cache_hits = (float)n_cache_memory_hits / 
        ((float)n_cache_memory_misses + n_cache_memory_hits) * 100.0f;
    printf("@percentage-of-cache-hits\t%.1f%%\n", n_percentage_of_cache_hits);

    printf("\n@exit(%i)\n", exit_status);
}

//  -------------------------------------------------------------------

//  The cache is represented as an array of cache blocks, where each block consists of:
//      - dirty bit -> determines if the block is out-of-sync with main memory
//      - address   -> points to the corresponding main_memory address
//      - value     -> the word stored in the cache
struct cache_block {
    bool dirty; // 1 dirty, 0 o/w.
    int address;
    AWORD value;
} cache_memory[N_CACHE_WORDS];

// Initialise all cache blocks in the cache to be dirty with a temporary
// address pointing to the middle of memory.
void cache_init(void) {
    for(int i = 0; i < N_CACHE_WORDS; ++i) {
        cache_memory[i].address = N_MAIN_MEMORY_WORDS/2;
        cache_memory[i].dirty = 1;
    }
}

//  -------------------------------------------------------------------

//  Function to implement the writing aspect of the write-back cache policy.
//  Variable arguments: int address (The address where the value will be written into.)
//                      AWORD value (The value that will be stored.)

void write_memory(int address, AWORD value) {
//  Locates the cache block to use.
    int cache_address = address % N_CACHE_WORDS;
    struct cache_block block = cache_memory[cache_address];

//  cache miss
    if(block.address != address) {
        // if the block is dirty but is also not the block we initialised at the start of execution
        if(block.dirty && block.address != N_MAIN_MEMORY_WORDS/2) {
            ++n_main_memory_writes;
            main_memory[block.address] = block.value;
        }
    }

//  write new dirty block to cache
    block.dirty = 1;
    block.address = address;
    block.value = value;
    cache_memory[cache_address] = block;
}

//  Function to implement the reading aspect of the write-back cache policy.
//  Variable arguments: int address (The address where the value will be read from.)
AWORD read_memory(int address) {
//  locate cache block to use
    int cache_address = address % N_CACHE_WORDS;
    struct cache_block block = cache_memory[cache_address];

//  cache hit
    if(block.address == address) {
        ++n_cache_memory_hits;
        return block.value;
    }

//  cache miss
    else {
        ++n_cache_memory_misses;
        ++n_main_memory_reads;

        if(block.dirty) {
            ++n_main_memory_writes;
            main_memory[block.address] = block.value;
        }

//      populate new block
        block.dirty = 0;
        block.address = address;
        block.value = main_memory[address];
        cache_memory[cache_address] = block;
    }
    return block.value;
}

//  -------------------------------------------------------------------

// EXECUTE THE INSTRUCTIONS IN main_memory[]. Returns an int, representing the item stored at
// the top of the stack.
int execute_stackmachine(void) {
//  THE 3 ON-CPU CONTROL REGISTERS:
    int PC      = 0;                    // 1st instruction is at address=0
    int SP      = N_MAIN_MEMORY_WORDS;  // initialised to top-of-stack
    int FP      = 0;                    // frame pointer

    AWORD address;
    IWORD value, value2;    // signed
    AWORD uValue;           // unsigned
    uint8_t bytes[2];

    cache_init();

    while(true) {
//  FETCH THE NEXT INSTRUCTION TO BE EXECUTED
        IWORD instruction = read_memory(PC);
        ++PC;
        ++n_number_of_instructions;

//      Halt: Program terminates.
        if(instruction == I_HALT) {
            break;
        }

        switch(instruction) {
//          No operation: PC advanced to the next instruction.
            case I_NOP:
                break;

//          Add: Two integers on the top of the stack popped and added. Result is left on top of the stack.
            case I_ADD:
                value = read_memory(SP++);
                value2 = read_memory(SP);
                write_memory(SP, value2 + value);
                break;

//          Subtract: Two integers on the top of the stack popped, second subtracted from first. 
//                    Result is left on the top of the stack.
            case I_SUB:
                value = read_memory(SP++);
                value2 = read_memory(SP);
                write_memory(SP, value2 - value);
                break;

//          Multiply: Two integers on the top of the stack popped and multiplied. Result is left on top of the stack.
            case I_MULT:
                value = read_memory(SP++);
                value2 = read_memory(SP);
                write_memory(SP, value2 * value);
                break;

//          Div: Two integers on the top of the stack popped, second divided from first. 
//               Result is left on top of the stack.
            case I_DIV:
                value = read_memory(SP++);
                value2 = read_memory(SP);
                write_memory(SP, value2 / value);
                break;

//          Call: Move PC to the next instruction to be executed, and set FP as required.
//                Also saves the address of the next instruction and the current
//                value of the FP, in that order, onto the top of the stack.
            case I_CALL:
                ++n_number_of_function_calls;
//              save the address of the next instruction onto the stack
                write_memory(--SP, PC + 1);
                
//              save the current value of FP onto the stack
                write_memory(--SP, FP);
                FP = SP;

//              move PC to the first instruction of the function we are calling
                PC = read_memory(PC);
                break;

//          Return: Saves the value on the top of the stack into FP + an offset.
//                  Program then returns to the instruction immediately after the
//                  last call.
            case I_RETURN:
//              read return value from TOS and compute FP-offset
                address = FP + read_memory(PC);
                value = read_memory(SP);

//              restore PC, FP and SP to continue execution of the calling function
                PC = read_memory(FP + 1);
                FP = read_memory(FP);
                SP = address;

//              write return value to FP-offset
                write_memory(address, value);
                break;

//          Unconditional jump: Flow of execution jumps to the next specified address.
            case I_JMP:
                PC = read_memory(PC);
                break;

//          Conditional jump:  Value at the top of the stack popped. Iff the value is 
//                             zero, flow of execution jumps to the next specified address.
            case I_JEQ:
                value = read_memory(SP++);
                if(value == 0) PC = read_memory(PC);
                else ++PC;
                break;

//          Print integer: Value at top of the stack popped and printed.
            case I_PRINTI:
                value = read_memory(SP++);
                fprintf(stdout, "%i", value);
                break;

//          Print String: Print the next NULL-byte terminated character string. 
            case I_PRINTS:
                address = read_memory(PC++);
//              while we do not see a NULL-byte, mask two sets of 8-bits from the 16-bit word
//              read from memory. each byte representing a character which we print to stdout.
                while(true) {
                    uValue = read_memory(address++);
                    bytes[0] = uValue & 0x00FF;
                    bytes[1] = uValue >> 8;
                    fprintf(stdout, "%c%c", bytes[0], bytes[1]);
                    if(bytes[0] == '\0' || bytes[1] == '\0') break;
                }
                break;

//          Push Constant: Pushes an integer constant onto the stack.
            case I_PUSHC:
                value = read_memory(PC++);
                write_memory(--SP, value);
                break;

//          Push Absolute: Push the integer in the address, which is specified in 
//                         the word immediately after the push declaration.
            case I_PUSHA:
                address = read_memory(PC++);
                value = read_memory(address);
                write_memory(--SP, value);
                break;

//          Push Relative: The next word specifies an offset added to the FP to read the value pushed
//                         onto the top of the stack.
            case I_PUSHR:
                address = read_memory(PC++) + FP;
                value = read_memory(address);
                write_memory(--SP, value);
                break;

//          Pop Absolute: A value from the top of the stack is popped. The next 
//                        word provides the location to write the popped value to.
            case I_POPA:
                address = read_memory(PC++);
                value = read_memory(SP++);
                write_memory(address, value);
                break;

//          Pop Relative: A value from the top of the stack is popped. The next word 
//                        provides the offset added to the FP to provide the 
//                        location to write the popped value to.
            case I_POPR:
                address = read_memory(PC++) + FP;
                value = read_memory(SP++);
                write_memory(address, value);
                break;
        }
    }

//  THE RESULT OF EXECUTING THE INSTRUCTIONS IS FOUND ON THE TOP-OF-STACK
    exit_status = read_memory(SP);
    return exit_status;
}

//  -------------------------------------------------------------------

//  READ THE PROVIDED coolexe FILE INTO main_memory[]
void read_coolexe_file(char filename[]) {
    memset(main_memory, 0, sizeof main_memory);   //  clear all memory

//  READ CONTENTS OF coolexe FILE
    FILE *file = fopen(filename, "rb");
    if(file) {
        fread(main_memory, sizeof(AWORD), N_MAIN_MEMORY_WORDS, file);
        fclose(file);
    }
    else {
        fprintf(stderr, "ERROR: Cannot open file %s.\n", filename);
        exit(EXIT_FAILURE);
    }
}

//  -------------------------------------------------------------------

int main(int argc, char *argv[]) {
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
