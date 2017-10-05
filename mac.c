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
// Note, this is array of zeroes, so there's really no need to initialize IP as IP = 0
// on line 255
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

void print_registers() {
    printf("Register Dump:\n");
    for (int i = 0; i < REGISTER_SIZE; i++) {
        printf("%04d ", registers[i]);
        if ((i + 1) % 4 == 0) {
            printf("\n");
        }
    }
}

void eval(int instr) {
    is_jmp = false;
    switch(instr) {
        case HLT: {
            running = false;
            printf("Finished Execution\n");
            break;
        }
        case PSH: {
            SP = SP + 1;
            IP = IP + 1;
            stack[SP] = instructions[IP];
            break;
        }
        case POP: {
            SP = SP - 1;
            break;
        }
        case ADD: {
            registers[A] = stack[SP];
            SP = SP - 1;
            registers[B] = stack[SP];
            registers[C] = registers[A] + registers[B];
            stack[SP] = registers[C];
            printf("%d + %d = %d\n", registers[B], registers[A], registers[C]);
            break;
        }
        case MUL: {
            registers[A] = stack[SP];
            SP = SP - 1;
            registers[B] = stack[SP];
            registers[C] = registers[A] * registers[B];
            stack[SP] = registers[C];
            printf("%d * %d = %d\n", registers[B], registers[A], registers[C]);
            break;
        }
        case DIV: {
            registers[A] = stack[SP];
            SP = SP - 1;
            registers[B] = stack[SP];
            registers[C] = registers[A] / registers[B];
            stack[SP] = registers[C];
            printf("%d / %d = %d\n", registers[B], registers[A], registers[C]);
            break;
        }
        case SUB: {
            registers[A] = stack[SP];
            SP = SP - 1;
            registers[B] = stack[SP];
            registers[C] = registers[A] - registers[B];
            stack[SP] = registers[C];
            printf("%d - %d = %d\n", registers[B], registers[A], registers[C]);
            break;
        }
        // SLT: pushes (REG_A < REG_B) to stack
        case SLT: {
            SP = SP - 1;
            stack[SP] = stack[SP + 1] < stack[SP];
            break;
        }
        // MOV REG_A, REG_B: moves value from REG_A to REG_B
        // args: instructions[IP + 1], [IP + 2] gotta be general purpose registers
        case MOV: {
            int reg_a = instructions[IP + 1];      // 0 - 15
            int reg_b = instructions[IP + 2];      // 0 - 15
            registers[reg_b] = registers[reg_a];   // reg[0-15] = reg[0-15]
            IP = IP + 2;
            break;
        }
        // SET REG, VAL: sets the reg to value
        case SET: {
            int reg = instructions[IP + 1];
            int val = instructions[IP + 2];
            registers[reg] = val;
            IP = IP + 2;
            break;
        }
        // LOG: prints out reg value
        case LOG: {
            int reg = instructions[IP + 1];
            printf("%d\n", registers[reg]);
            IP = IP + 1;
            break;
        }
        // IF REG, VAL: if reg == val, go to branch to the ip 
        case IF: {
             int reg = instructions[IP + 1];
             int val = instructions[IP + 2];
             if (registers[reg] == val) {
                 IP = instructions[IP + 3];
                 is_jmp = true;
             }
             else {
                 IP = IP + 3;
             }
             break;
        }
        case IFN: {
             int reg = instructions[IP + 1];
             int val = instructions[IP + 2];
             if (registers[reg] != val) {
                 IP = instructions[IP + 3];
                 is_jmp = true;
             }
             else {
                 IP = IP + 3;
             }
             break;
        }
        case GLD: {
            SP = SP + 1;
            IP = IP + 1;
            stack[SP] = registers[instructions[IP]];
            break;
        }
        case GPT: {
             registers[instructions[IP + 1]] = stack[SP];
             IP = IP + 1;
             break;
        }
        case NOP: {
            printf("Do nothing.\n");
            break;
        }
        default: {
            printf("Unknown instruction %d\n", instr);
            break;
        }
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
    IP = 0;
    while(running && IP < instructions_count) {
        eval(FETCH);
        if (!is_jmp) {
            IP = IP+1;
        }
    }

    free(instructions);

    return 0;
}


