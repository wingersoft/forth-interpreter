#ifndef FORTH_H
#define FORTH_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Stack size constants
#define STACK_SIZE 1024
#define DICT_SIZE 1024
#define MAX_WORD_LEN 32
#define MAX_LINE_LEN 256

// Data types
typedef int Cell; // Assuming 32-bit integers for simplicity

// Stack structure
typedef struct
{
    Cell stack[STACK_SIZE];
    int sp; // Stack pointer
} Stack;

// Dictionary entry structure
typedef struct Word
{
    char name[MAX_WORD_LEN];
    void (*func)(); // Function pointer for built-in words
    Cell *code;     // Code field for user-defined words
    int code_size;
    struct Word *next;
} Word;

// Dictionary structure
typedef struct
{
    Word *words[DICT_SIZE];
    int count;
} Dictionary;

// Global structures
extern Stack data_stack;
extern Stack return_stack;
extern Dictionary dict;
extern Cell memory[STACK_SIZE];
extern int base;                     // Number base for input/output
extern int state;                    // 0: interpreting, 1: compiling
extern Cell code_buffer[STACK_SIZE]; // Buffer for compiling words
extern int code_sp;                  // Code stack pointer
extern Word *current_word;           // Current word being compiled

// Control flow
typedef enum {
    CF_IF,
    CF_ELSE,
    CF_BEGIN,
    CF_WHILE,
    CF_DO,
    CF_UNTIL,
    CF_REPEAT,
    CF_END
} ControlFlowType;

typedef struct
{
    int origin;
    ControlFlowType type;
} BranchEntry;

typedef struct
{
    BranchEntry entries[STACK_SIZE];
    int top;
} BranchStack;

extern BranchStack branch_stack;

// Opcodes for code field
#define OP_BRANCH -2
#define OP_0BRANCH -3
#define OP_LIT -1

// Control flow words
void if_word(void);
void then_word(void);
void else_word(void);
void begin_word(void);
void until_word(void);
void while_word(void);
void repeat_word(void);
void end_word(void);

// Stack operations
void stack_push(Cell value);
Cell stack_pop(void);
Cell stack_peek(void);
int stack_empty(void);
int stack_full(void);

// Dictionary operations
void dict_init(void);
Word *dict_find(const char *name);
void dict_add(Word *word);

// Basic error handling
void error(const char *msg);

// Memory operations
void mem_store(int addr, Cell value);
Cell mem_fetch(int addr);

// I/O operations
void print_cell(Cell value);
void print_stack(void);
void cr(void);

// Compiler words
void colon(void);
void semicolon(void);

// Helper functions
char *tokenize(char *line, char *token);

// Execute user-defined word
void execute_word(Word *word);

// Helper functions
int dict_find_index(const char *name);

// Initialize the interpreter
void forth_init(void);

// Main interpreter loop
void repl(void);

#endif /* FORTH_H */