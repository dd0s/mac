#include <stdio.h>
#include <stdbool.h>

bool running = true;

// Instruction pointer. The program itself is stored as an array of integers. 
// The ip variable serves as an index in the array as to which instruction is currently being 
// executed.
int ip = 0;

// stack pointer
int sp = -1;

int stack[256];

typedef enum {
    PSH,
    ADD,
    POP,
    HLT
} instructions;

const int program[] = {
    PSH, 5,
    PSH, 6,
    ADD,
    POP,
    HLT
};

int fetch() {
    return program[ip];
}

void eval(int instr) {
    switch(instr) {
        case HLT: {
            running = false;
            printf("done\n");
            break;
        }
        case PSH: {
            sp++;
            stack[sp] = program[++ip];
            break;
        }
        case POP: {
            int val_popped = stack[sp--];
            printf("popped: %d\n", val_popped);
            break;
        }
        case ADD: {
            int a = stack[sp--];
            int b = stack[sp--];
            int result = a + b;
            sp++;
            stack[sp] = result;
            break;
        }
    }
}

int main() {
    while(running) {
        eval(fetch());
        ip++;
    }
}


