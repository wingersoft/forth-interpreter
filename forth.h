#ifndef FORTH_H
#define FORTH_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Core system constants - Define memory and buffer sizes for the Forth interpreter
#define STACK_SIZE 1024    // Size of data stack, return stack, and branch stack
#define DICT_SIZE 1024     // Maximum number of words in dictionary
#define MAX_WORD_LEN 32    // Maximum length of word names
#define MAX_LINE_LEN 256   // Maximum length of input lines

// Core data type - Cell is the fundamental unit of data in Forth
typedef long long Cell;    // Use long long for maximum compatibility with pointers and large integers

// Stack data structure - LIFO (Last In, First Out) container for data manipulation
typedef struct
{
    Cell stack[STACK_SIZE];  // Array holding stack elements
    int sp;                  // Stack pointer (-1 = empty, 0+ = valid indices)
} Stack;

// Word dictionary entry - Represents both built-in and user-defined words
typedef struct Word
{
    char name[MAX_WORD_LEN];     // Word name (null-terminated string)
    void (*func)();              // Function pointer for built-in words (NULL for user-defined)
    Cell *code;                  // Code array for user-defined words (NULL for built-ins)
    int code_size;               // Number of cells in code array
    int immediate;               // 1 = execute immediately even in compile mode, 0 = normal
    struct Word *next;           // Linked list pointer (currently unused)
} Word;

// Dictionary structure - Contains all defined words (built-in and user-defined)
typedef struct
{
    Word *words[DICT_SIZE];  // Array of word pointers
    int count;               // Number of words currently in dictionary
} Dictionary;

// Global interpreter state - Core data structures accessible throughout the program
extern Stack data_stack;             // Main data stack for computation
extern Stack return_stack;           // Return stack for loops and control flow
extern Dictionary dict;              // Dictionary of all defined words
extern Cell memory[STACK_SIZE];      // Linear memory array for variables
extern int base;                     // Number base for input/output (10 = decimal)
extern int state;                    // Interpreter state: 0=interpreting, 1=compiling
extern Cell code_buffer[STACK_SIZE]; // Temporary buffer for compiling user-defined words
extern int code_sp;                  // Current position in code buffer
extern Word *current_word;           // Word currently being defined (NULL when not compiling)

// Control flow structures - Support for compiling conditional and looping constructs

// Types of control flow constructs tracked during compilation
typedef enum
{
    CF_IF,      // Start of if-then-else
    CF_ELSE,    // Else branch of if-then-else
    CF_BEGIN,   // Start of begin-until or begin-while-repeat
    CF_WHILE,   // While condition in begin-while-repeat
    CF_DO,      // Start of do-loop
    CF_UNTIL,   // Until condition (unused)
    CF_REPEAT,  // Repeat in begin-while-repeat (unused)
    CF_END      // End marker
} ControlFlowType;

// Entry in branch stack for tracking control flow compilation
typedef struct
{
    int origin;             // Code position where this construct begins
    ControlFlowType type;   // Type of control flow construct
} BranchEntry;

// Stack for managing nested control flow constructs during compilation
typedef struct
{
    BranchEntry entries[STACK_SIZE];  // Array of branch entries
    int top;                          // Stack top (-1 = empty)
} BranchStack;

extern BranchStack branch_stack;

// Opcodes for threaded code execution - Special values used in user-defined word code arrays
#define OP_LIT -1      // Marks a literal value (next cell contains the value)
#define OP_BRANCH -2   // Unconditional branch (next cell contains offset)
#define OP_0BRANCH -3  // Conditional branch if top of stack is 0 (next cell contains offset)
#define OP_DO -4       // Setup for DO loop (pushes index and limit to return stack)
#define OP_LOOP -5     // LOOP construct (increments index, tests limit, branches back)
#define OP_J -6        // Access second loop index (currently unused)

// Control flow word implementations - Functions for compiling conditional and looping constructs
void if_word(void);     // Compile IF (conditional branch)
void then_word(void);   // Complete IF or ELSE branch
void else_word(void);   // Handle ELSE part of IF-THEN-ELSE
void begin_word(void);  // Mark start of loop construct
void until_word(void);  // End begin-until loop with condition
void while_word(void);  // Conditional test in begin-while-repeat
void repeat_word(void); // Complete begin-while-repeat loop
void end_word(void);    // Placeholder for ending definitions
void do_word(void);     // Start counted DO loop
void loop_word(void);   // End DO loop with increment/test
void i_word(void);      // Access current loop index (DO loop)
void j_word(void);      // Access outer loop index (nested DO loops)


// Data stack operations - Core functions for manipulating the data stack
void stack_push(Cell value);  // Push value onto data stack
Cell stack_pop(void);         // Pop and return top value from data stack
Cell stack_peek(void);        // Return top value without removing it
int stack_empty(void);        // Check if data stack is empty
int stack_full(void);         // Check if data stack is full

// Dictionary management - Functions for maintaining the word dictionary
void dict_init(void);           // Initialize dictionary structure
Word *dict_find(const char *name); // Search for word by name
void dict_add(Word *word);      // Add new word to dictionary

// Error handling - Non-fatal error recovery mechanism
void error(const char *msg);    // Print error and reset interpreter state

// Memory operations - Access to linear memory array
void mem_store(int addr, Cell value); // Store value at memory address
Cell mem_fetch(int addr);       // Retrieve value from memory address

// Input/Output operations - Functions for displaying data and debugging
void print_cell(Cell value);    // Print a single cell value
void print_stack(void);         // Display entire data stack contents
void cr(void);                  // Print carriage return (newline)

// Word definition - Compiler directives for creating user-defined words
void colon(void);               // Start word definition (:)
void semicolon(void);           // End word definition (;)

// Input processing state - Variables for parsing input text during tokenization
extern char *current_input;  // Current input line being processed
extern char *input_pos;      // Current position within input line
extern int next_mem_addr;    // Next available memory address for variables

// Input processing - Functions for parsing and tokenizing input text
char *tokenize(char *token);    // Extract next token from input stream

// Word execution - Core execution engine for both built-in and user-defined words
void execute_word(Word *word);  // Execute a word by name or reference

// Defining words - Special words that create new words in the dictionary
void variable_word(void);      // Create a variable (pushes address)
void constant_word(void);      // Create a constant (pushes fixed value)

// Utility functions - Helper functions for internal operations
int dict_find_index(const char *name); // Find word index in dictionary (unused)

// Core interpreter functions - Main entry points for the Forth system
void forth_init(void);         // Initialize interpreter with built-in words
void repl(void);               // Run Read-Eval-Print Loop for interactive use

#endif /* FORTH_H */