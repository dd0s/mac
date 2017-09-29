// BUG FOUND: IP is not ever explicitly initialized in the code
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define STACK_SIZE 256
static int stack[STACK_SIZE];

// Instructions
typedef enum {
    HLT,
    PSH,
    POP,
    ADD,
    MUL,
    DIV,
    SUB,
    SLT,
    MOV,
    SET,
    LOG,
    IF,
    IFN,
    GLD,
    GPT,
    NOP
} Instructions;

// Registers
typedef enum {
    A, B, C, D, E, F, I, J, // general purpose
    EX,                     // excess
    EXA,                    // more excess
    IP,                     // instruction pointer
    SP,                     // stack pointer
    REGISTER_SIZE
} Registers;
static int registers[REGISTER_SIZE];

// instructions array
int *instructions;

// how many instructions were read from file
int instructions_count = 0;

// how much space is allocated for the instructions
// 4 instructions by default
int instructions_space = 4;

static bool running = true;

// if the IP is assigned by jmp instructions(such as IF, IFN), 
// it should not increase 1 anymore
bool is_jmp = false;

// quick way to get SP and IP; usage: SP = ...
#define SP (registers[SP])
#define IP (registers[IP])

// fetch current instruction set
#define FETCH (instructions[IP])

// prints the stack from A to B
void print_stack() {
    for (int i = 0; i < SP; i++) {
        printf("0x%04d ", stack[i]);
        if ((i + 1) % 4 == 0) {
            printf("\n");
        }
    }
    if (SP != 0) {
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        printf("error: no input files\n");
        return -1;
    }

    char *filename = argv[1];

    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("error: could not read file `%s`\n", filename);
        return -1;
    }

    // allocate space for instructions // TODO: remove (int*) or not
    instructions = (int*)malloc(sizeof(*instructions) * instructions_space); // TODO: sizeof (int)

    // read the binary file
    int num;
    int i = 0;
    while(fscanf(file, "%d", &num) > 0) {
        instructions[i] = num;
        printf("%d\n", instructions[i]);
        i++;
        if (i >= instructions_count) {
            instructions_space *= 2;
            instructions = realloc(instructions, sizeof(*instructions) * instructions_space);
        }
    }
    instructions_count = i;

    fclose(file);
   
    // TODO: report a bug: where is IP initialization?
    SP = -1;

    while(running && IP < instructions_count) {
        eval(FETCH);
        if (!is_jmp) {
            IP = IP+1;
        }
    }

    free(instructions);

    return 0;
}


