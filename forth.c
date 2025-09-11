#include "forth.h"

// Global structures - Core data structures for the Forth interpreter
Stack data_stack = {{0}, -1};        // Main data stack for computation
Stack return_stack = {{0}, -1};      // Return stack for control flow and loops
Dictionary dict = {{NULL}, 0};       // Dictionary containing all defined words
Cell memory[STACK_SIZE] = {0};       // Linear memory space for variables and data
Cell code_buffer[STACK_SIZE] = {0};  // Temporary buffer for compiling user-defined words
int base = 10;                       // Number base for input/output (default decimal)
int state = 0;                       // Interpreter state: 0=interpreting, 1=compiling
int code_sp = 0;                     // Code buffer stack pointer during compilation
Word *current_word = NULL;           // Pointer to word currently being compiled
int next_mem_addr = 0;               // Next available address in memory array

// Input handling - Variables for parsing input text
char *current_input = NULL;          // Current input line being processed
char *input_pos = NULL;              // Current position in input string during tokenization

// Control flow - Stack for managing compilation of control structures
BranchStack branch_stack = {{{0, CF_END}}, -1}; // Stack for tracking branch points in control flow
char *parse_string(char *str);       // Forward declaration for string parsing function

// Stack operations - Core functions for manipulating the data stack

/**
 * Push a value onto the data stack
 * @param value The cell value to push onto the stack
 */
void stack_push(Cell value)
{
    if (data_stack.sp >= STACK_SIZE - 1)
    {
        error("Stack overflow");
        return;
    }
    data_stack.stack[++data_stack.sp] = value;
}

/**
 * Pop a value from the data stack
 * @return The top value from the stack, or 0 on underflow
 */
Cell stack_pop(void)
{
    if (data_stack.sp < 0)
    {
        error("Stack underflow");
        return 0;
    }
    return data_stack.stack[data_stack.sp--];
}

/**
 * Peek at the top value on the data stack without removing it
 * @return The top value from the stack, or 0 on underflow
 */
Cell stack_peek(void)
{
    if (data_stack.sp < 0)
    {
        error("Stack underflow");
        return 0;
    }
    return data_stack.stack[data_stack.sp];
}

/**
 * Check if the data stack is empty
 * @return 1 if empty, 0 otherwise
 */
int stack_empty(void)
{
    return data_stack.sp < 0;
}

/**
 * Check if the data stack is full
 * @return 1 if full, 0 otherwise
 */
int stack_full(void)
{
    return data_stack.sp >= STACK_SIZE - 1;
}

// Return stack operations - Functions for manipulating the return stack (used for loops and control flow)

/**
 * Push a value onto the return stack
 * @param value The cell value to push onto the return stack
 */
void rstack_push(Cell value)
{
    if (return_stack.sp >= STACK_SIZE - 1)
    {
        error("Return stack overflow");
        return;
    }
    return_stack.stack[++return_stack.sp] = value;
}

/**
 * Pop a value from the return stack
 * @return The top value from the return stack, or 0 on underflow
 */
Cell rstack_pop(void)
{
    if (return_stack.sp < 0)
    {
        error("Return stack underflow");
        return 0;
    }
    return return_stack.stack[return_stack.sp--];
}

/**
 * Peek at the top value on the return stack without removing it
 * @return The top value from the return stack, or 0 on underflow
 */
Cell rstack_peek(void)
{
    if (return_stack.sp < 0)
    {
        error("Return stack underflow");
        return 0;
    }
    return return_stack.stack[return_stack.sp];
}

/**
 * Peek at a value n positions down from the top of the return stack
 * @param n Number of positions down from the top (0 = top, 1 = one below top, etc.)
 * @return The value at the specified position, or 0 on underflow
 */
Cell rstack_peek_n(int n)
{
    if (return_stack.sp < n)
    {
        error("Return stack underflow");
        return 0;
    }
    return return_stack.stack[return_stack.sp - n];
}


// Branch stack operations - Stack for tracking control flow constructs during compilation

/**
 * Push a branch entry onto the branch stack
 * @param origin The code position where this branch construct begins
 * @param type The type of control flow construct (IF, BEGIN, etc.)
 */
void branch_stack_push(int origin, ControlFlowType type)
{
    if (branch_stack.top >= STACK_SIZE - 1)
    {
        error("Branch stack overflow");
        return;
    }
    branch_stack.top++;
    branch_stack.entries[branch_stack.top].origin = origin;
    branch_stack.entries[branch_stack.top].type = type;
}

/**
 * Pop a branch entry from the branch stack
 * @return The branch entry, or empty entry on underflow
 */
BranchEntry branch_stack_pop(void)
{
    if (branch_stack.top < 0)
    {
        error("Branch stack underflow");
        BranchEntry empty = {0, CF_END};
        return empty;
    }
    BranchEntry entry = branch_stack.entries[branch_stack.top];
    branch_stack.top--;
    return entry;
}

/**
 * Peek at the top branch entry without removing it
 * @return The top branch entry, or empty entry on underflow
 */
BranchEntry branch_stack_peek(void)
{
    if (branch_stack.top < 0)
    {
        error("Branch stack underflow");
        BranchEntry empty = {0, CF_END};
        return empty;
    }
    return branch_stack.entries[branch_stack.top];
}

/**
 * Check if the branch stack is empty
 * @return 1 if empty, 0 otherwise
 */
int branch_stack_empty(void)
{
    return branch_stack.top < 0;
}

// Dictionary operations - Functions for managing the word dictionary

/**
 * Initialize the dictionary structure
 * Sets up the dictionary with zero count and clears all word pointers
 */
void dict_init(void)
{
    dict.count = 0;
    // Initialize built-in words here later
    memset(dict.words, 0, sizeof(dict.words));
}

/**
 * Search for a word in the dictionary by name
 * @param name The word name to search for
 * @return Pointer to the word if found, NULL otherwise
 */
Word *dict_find(const char *name)
{
    // Simple linear search for now; optimize with hash later
    for (int i = 0; i < dict.count; i++)
    {
        if (strcmp(dict.words[i]->name, name) == 0)
        {
            return dict.words[i];
        }
    }
    return NULL;
}

/**
 * Add a new word to the dictionary
 * @param word Pointer to the word structure to add
 */
void dict_add(Word *word)
{
    if (dict.count >= DICT_SIZE)
    {
        error("Dictionary full");
        return;
    }
    dict.words[dict.count++] = word;
}

// Basic error handling - Non-fatal error recovery mechanism

/**
 * Handle errors by printing message and resetting interpreter state
 * This allows the interpreter to continue running after errors rather than exiting
 * @param msg The error message to display
 */
void error(const char *msg)
{
    fprintf(stderr, "Error: %s\n", msg);
    // Reset stacks and state to continue execution
    data_stack.sp = -1;        // Clear data stack
    return_stack.sp = -1;      // Clear return stack
    branch_stack.top = -1;     // Clear branch stack
    state = 0;                 // Back to interpret mode
    code_sp = 0;               // Reset code buffer pointer
    current_word = NULL;       // Clear current word being compiled
}

// Memory operations - Functions for accessing the linear memory array

/**
 * Store a value in memory at the specified address
 * @param addr The memory address to write to
 * @param value The value to store
 */
void mem_store(int addr, Cell value)
{
    if (addr < 0 || addr >= STACK_SIZE)
    {
        error("Invalid memory address");
        return;
    }
    memory[addr] = value;
}

/**
 * Fetch a value from memory at the specified address
 * @param addr The memory address to read from
 * @return The value at the address, or 0 on invalid address
 */
Cell mem_fetch(int addr)
{
    if (addr < 0 || addr >= STACK_SIZE)
    {
        error("Invalid memory address");
        return 0;
    }
    return memory[addr];
}

// I/O operations - Functions for input/output and debugging

/**
 * Print a cell value to stdout followed by a space
 * @param value The cell value to print
 */
void print_cell(Cell value)
{
    printf("%lld ", value);
}

/**
 * Print the entire data stack contents in Forth notation
 * Format: < val1 val2 val3 ... > with space after >
 */
void print_stack(void)
{
    printf("< ");
    for (int i = 0; i <= data_stack.sp; i++)
    {
        printf("%lld ", data_stack.stack[i]);
    }
    printf("> ");
}

/**
 * Print a carriage return (newline) to stdout
 */
void cr(void)
{
    printf("\n");
}

// Built-in arithmetic operations - Basic math functions that pop two values and push result

/**
 * Addition: (a b -- a+b)
 * Pops two values from stack and pushes their sum
 */
void plus(void)
{
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(a + b);
}

/**
 * Subtraction: (a b -- a-b)
 * Pops two values and pushes a minus b
 */
void minus(void)
{
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(a - b);
}

/**
 * Multiplication: (a b -- a*b)
 * Pops two values and pushes their product
 */
void star(void)
{
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(a * b);
}

/**
 * Division: (a b -- a/b)
 * Pops two values and pushes a divided by b
 * Handles division by zero error
 */
void slash(void)
{
    Cell b = stack_pop();
    if (b == 0)
    {
        error("Division by zero");
        return;
    }
    Cell a = stack_pop();
    stack_push(a / b);
}

/**
 * Modulo: (a b -- a%b)
 * Pops two values and pushes a modulo b
 * Handles modulo by zero error
 */
void mod(void)
{
    Cell b = stack_pop();
    if (b == 0)
    {
        error("Modulo by zero");
        return;
    }
    Cell a = stack_pop();
    stack_push(a % b);
}

// Built-in stack operations - Functions for manipulating stack contents

/**
 * DUP: (a -- a a)
 * Duplicate the top stack item
 */
void dup(void)
{
    Cell top = stack_peek();
    stack_push(top);
}

/**
 * DROP: (a -- )
 * Remove the top stack item
 */
void drop(void)
{
    stack_pop();
}

/**
 * SWAP: (a b -- b a)
 * Exchange the top two stack items
 */
void swap(void)
{
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(b);
    stack_push(a);
}

/**
 * OVER: (a b -- a b a)
 * Copy the second item to the top
 */
void over(void)
{
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(a);
    stack_push(b);
    stack_push(a);
}

/**
 * ROT: (a b c -- b c a)
 * Rotate the top three items
 */
void rot(void)
{
    Cell c = stack_pop();
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(b);
    stack_push(c);
    stack_push(a);
}

/**
 * NIP: (a b -- b)
 * Remove the second item from the stack
 */
void nip(void)
{
    Cell b = stack_pop();
    stack_pop(); // discard a
    stack_push(b);
}

/**
 * TUCK: (a b -- b a b)
 * Copy the top item below the second item
 */
void tuck(void)
{
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(b);
    stack_push(a);
    stack_push(b);
}

// Built-in comparison operations - Functions that compare two values and push boolean result

/**
 * = : (a b -- flag)
 * Push -1 if a equals b, 0 otherwise
 */
void equal(void)
{
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(a == b ? -1 : 0);
}

/**
 * < : (a b -- flag)
 * Push -1 if a is less than b, 0 otherwise
 */
void less_than(void)
{
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(a < b ? -1 : 0);
}

/**
 * > : (a b -- flag)
 * Push -1 if a is greater than b, 0 otherwise
 */
void greater_than(void)
{
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(a > b ? -1 : 0);
}

/**
 * <= : (a b -- flag)
 * Push -1 if a is less than or equal to b, 0 otherwise
 */
void less_equal(void)
{
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(a <= b ? -1 : 0);
}

/**
 * >= : (a b -- flag)
 * Push -1 if a is greater than or equal to b, 0 otherwise
 */
void greater_equal(void)
{
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(a >= b ? -1 : 0);
}

/**
 * <> : (a b -- flag)
 * Push -1 if a is not equal to b, 0 otherwise
 */
void not_equal(void)
{
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(a != b ? -1 : 0);
}

// Built-in logical operations - Bitwise logical operations

/**
 * AND: (a b -- a&b)
 * Push bitwise AND of a and b
 */
void and_op(void)
{
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(a & b);
}

/**
 * OR: (a b -- a|b)
 * Push bitwise OR of a and b
 */
void or_op(void)
{
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(a | b);
}

/**
 * NOT: (a -- ~a)
 * Push bitwise NOT of a (one's complement)
 */
void not_op(void)
{
    Cell a = stack_pop();
    stack_push(~a);
}

// Built-in memory operations - Functions for storing and fetching from memory

/**
 * ! (store): (value addr -- )
 * Store value at memory address addr
 */
void store(void)
{
    Cell addr = stack_pop();
    Cell value = stack_pop();
    mem_store(addr, value);
}

/**
 * @ (fetch): (addr -- value)
 * Fetch value from memory address addr
 */
void fetch(void)
{
    Cell addr = stack_pop();
    Cell value = mem_fetch(addr);
    stack_push(value);
}

// Built-in CREATE word - Creates a word that pushes a memory address onto the stack

/**
 * CREATE: Parse next token as name and create a word that pushes its memory address
 * The created word can be used as a base for defining expandable words
 */
void create_word(void)
{
    char name[MAX_WORD_LEN];
    if (!tokenize(name))
    {
        error("CREATE needs a name");
        return;
    }
    int addr = next_mem_addr;  // Get current memory address (don't increment)
    Word *new_word = malloc(sizeof(Word));
    strcpy(new_word->name, name);
    new_word->func = NULL;
    new_word->code = malloc(2 * sizeof(Cell));
    new_word->code[0] = OP_LIT;     // Literal opcode
    new_word->code[1] = addr;       // The memory address
    new_word->code_size = 2;
    new_word->next = NULL;
    dict_add(new_word);
}

// Built-in VARIABLE word - Creates a named variable that pushes its address

/**
 * VARIABLE: Parse next token as name and create a variable at the next memory location
 * When executed, pushes the variable's memory address onto the stack
 */
void variable_word(void)
{
    char name[MAX_WORD_LEN];
    if (!tokenize(name))
    {
        error("VARIABLE needs a name");
        return;
    }
    int addr = next_mem_addr++;  // Allocate new memory location
    Word *new_word = malloc(sizeof(Word));
    strcpy(new_word->name, name);
    new_word->func = NULL;
    new_word->code = malloc(2 * sizeof(Cell));
    new_word->code[0] = OP_LIT;     // Literal opcode
    new_word->code[1] = addr;       // The variable's memory address
    new_word->code_size = 2;
    new_word->next = NULL;
    dict_add(new_word);
}

// Built-in CONSTANT word - Creates a named constant with a fixed value

/**
 * CONSTANT: Pop value from stack and create a word that pushes that value
 * The constant value is stored in the word's code and cannot be changed
 */
void constant_word(void)
{
    char name[MAX_WORD_LEN];
    if (!tokenize(name))
    {
        error("CONSTANT needs a name");
        return;
    }
    Cell value = stack_pop();  // Get the constant value from stack
    Word *new_word = malloc(sizeof(Word));
    strcpy(new_word->name, name);
    new_word->func = NULL;
    new_word->code = malloc(2 * sizeof(Cell));
    new_word->code[0] = OP_LIT;     // Literal opcode
    new_word->code[1] = value;      // The constant value
    new_word->code_size = 2;
    new_word->next = NULL;
    dict_add(new_word);
}

// Execute user-defined word - Core function for threaded code interpretation

/**
 * Execute a word, either as a built-in function or as threaded code
 * This implements the heart of the Forth interpreter's execution model
 * @param word The word to execute
 */
void execute_word(Word *word)
{
    // If it's a built-in word, call its function directly
    if (word->func)
    {
        word->func();
        return;
    }

    // Execute threaded code for user-defined words
    // Each item in code[] is either an opcode, literal value, or word pointer
    for (int i = 0; i < word->code_size; i++)
    {
        Cell item = word->code[i];

        // Handle literal values (push next item onto stack)
        if (item == OP_LIT)
        {
            i++; // Move to the literal value
            stack_push(word->code[i]);
        }
        // Handle unconditional branch (OP_BRANCH offset)
        else if (item == OP_BRANCH)
        {
            i++; // Skip OP_LIT marker
            i++; // Skip to the offset value
            Cell offset = word->code[i];
            i += offset - 1; // Jump to target (-1 because loop will increment)
        }
        // Handle conditional branch (OP_0BRANCH offset)
        else if (item == OP_0BRANCH)
        {
            i++; // Skip OP_LIT marker
            i++; // Skip to the offset value
            Cell offset = word->code[i];
            Cell top = stack_pop(); // Pop condition flag
            if (top == 0) // If false (0), take the branch
            {
                i += offset - 1; // Jump to target
            }
        }
        // Handle DO loop setup (limit start -- )
        else if (item == OP_DO)
        {
            Cell start = stack_pop(); // Starting index
            Cell limit = stack_pop(); // Loop limit
            rstack_push(limit);       // Push limit to return stack
            rstack_push(start);       // Push start index to return stack
        }
        // Handle LOOP increment and test
        else if (item == OP_LOOP)
        {
            Cell index = rstack_pop();  // Get current index
            Cell limit = rstack_peek(); // Peek at limit (don't remove yet)
            index++; // Increment index

            if (index < limit) // Continue looping?
            {
                rstack_push(index); // Push updated index back
                // Branch back to loop start
                i++; // Skip OP_LIT marker
                i++; // Skip to the offset value
                Cell offset = word->code[i];
                i += offset - 1; // Jump back
            }
            else
            {
                rstack_pop(); // Remove limit from return stack
                // Loop ends, continue to next instruction
            }
        }
        else
        {
            // Check if item is a word pointer (large address values)
            // Word pointers are typically large memory addresses
            if (item > 1000 && item < 10000000000000000LL)
            {
                Word *w = (Word *)item;
                // Basic validation of word pointer
                if (w && w->name && w->name[0] != '\0')
                {
                    execute_word(w); // Recursively execute the referenced word
                }
                else
                {
                    error("Invalid word reference");
                }
            }
            else
            {
                // Small numbers are treated as literal values
                stack_push(item);
            }
        }
    }
}

// Built-in I/O operations
void dot(void)
{
    Cell value = stack_pop();
    print_cell(value);
}

void dot_s(void)
{
    print_stack();
}

void dot_quote(void)
{
    char str[MAX_LINE_LEN];
    if (!parse_string(str)) {
        error("Expected string after .\"");
        return;
    }
    printf("%s", str);
}
void cells_word(void)
{
    Cell n = stack_pop();
    stack_push(n * sizeof(Cell));
}

void allot_word(void)
{
    Cell n = stack_pop();
    next_mem_addr += n;
}

void i_word(void)
{
    stack_push(rstack_peek());
}

void j_word(void)
{
    stack_push(rstack_peek_n(2));
}

// Control flow functions - Words for implementing conditional and looping constructs

/**
 * IF: Compile conditional branch for if-then-else structure
 * Compiles: OP_0BRANCH OP_LIT 0 (placeholder for offset)
 * Pushes IF entry to branch stack for later resolution by THEN/ELSE
 */
void if_word(void)
{
    if (state) // Only works in compile mode
    {
        branch_stack_push(code_sp, CF_IF); // Track this IF for matching THEN/ELSE
        code_buffer[code_sp++] = OP_0BRANCH; // Conditional branch opcode
        code_buffer[code_sp++] = OP_LIT;     // Literal marker
        code_buffer[code_sp++] = 0;          // Placeholder for branch offset
    }
    else
    {
        error("IF used outside of compilation mode");
    }
}

/**
 * THEN: Complete IF or ELSE branch by filling in the jump offset
 * Calculates offset from IF/ELSE origin to current position and stores it
 */
void then_word(void)
{
    if (state) // Only works in compile mode
    {
        if (branch_stack_empty())
        {
            error("THEN without matching IF");
            return;
        }

        BranchEntry entry = branch_stack_pop();
        if (entry.type != CF_IF && entry.type != CF_ELSE)
        {
            error("THEN without matching IF");
            return;
        }

        // Calculate and store the branch offset
        Cell offset = code_sp - (entry.origin + 2);
        code_buffer[entry.origin + 2] = offset;
    }
    else
    {
        error("THEN used outside of compilation mode");
    }
}

/**
 * ELSE: Handle the else part of if-then-else structure
 * Completes the IF branch to jump over ELSE, then starts ELSE branch
 */
void else_word(void)
{
    if (state) // Only works in compile mode
    {
        if (branch_stack_empty())
        {
            error("ELSE without matching IF");
            return;
        }

        BranchEntry entry = branch_stack_peek();
        if (entry.type != CF_IF)
        {
            error("ELSE without matching IF");
            return;
        }

        // Compile jump from end of IF to skip over ELSE part
        int else_branch_origin = code_sp;
        code_buffer[code_sp++] = OP_BRANCH; // Unconditional branch
        code_buffer[code_sp++] = OP_LIT;    // Literal marker
        code_buffer[code_sp++] = 0;         // Placeholder for offset

        // Fix the IF branch to jump to start of ELSE
        Cell if_offset = (code_sp + 3) - (entry.origin + 2);
        code_buffer[entry.origin + 2] = if_offset;

        // Replace IF entry with ELSE entry on branch stack
        branch_stack_pop();
        branch_stack_push(else_branch_origin, CF_ELSE);
    }
    else
    {
        error("ELSE used outside of compilation mode");
    }
}

/**
 * END: Placeholder for ending definitions (currently unused)
 * This word is defined but not actively used since ; (semicolon) handles definition ending
 */
void end_word(void)
{
    // This would be used to end definitions, but we already have semicolon()
    // This function might be redundant or used for other purposes
    // For now, we'll leave it as a placeholder
}

void colon(void)
{
    char word_name[MAX_WORD_LEN];

    // Read the next token as the word name
    if (!tokenize(word_name))
    {
        error("Expected word name after :\"");
        return;
    }

    // Check if word already exists
    if (dict_find(word_name))
    {
        error("Word already exists");
        return;
    }

    // Create new word
    Word *new_word = malloc(sizeof(Word));
    if (!new_word)
    {
        error("Memory allocation failed");
        return;
    }

    strcpy(new_word->name, word_name);
    new_word->func = NULL;
    new_word->code = malloc(STACK_SIZE * sizeof(Cell));
    if (!new_word->code)
    {
        free(new_word);
        error("Memory allocation failed");
        return;
    }
    new_word->code_size = 0;
    new_word->next = NULL;

    // Set current word and switch to compile mode
    current_word = new_word;
    state = 1;
    code_sp = 0; // Reset code buffer pointer
}

void semicolon(void)
{
    if (state == 0)
    {
        error("Misplaced ;");
        return;
    }

    if (!current_word)
    {
        error("No word being defined");
        return;
    }

    // Copy compiled code from buffer to word
    current_word->code_size = code_sp;
    memcpy(current_word->code, code_buffer, code_sp * sizeof(Cell));

    // Add to dictionary
    dict_add(current_word);

    // Reset state
    state = 0;
    current_word = NULL;
    code_sp = 0;
}

/**
 * BEGIN: Mark the start of a loop construct
 * Pushes current position to branch stack for UNTIL/WHILE to reference
 */
void begin_word(void)
{
    if (state) // Only works in compile mode
    {
        branch_stack_push(code_sp, CF_BEGIN); // Mark loop start position
    }
    else
    {
        error("BEGIN used outside of compilation mode");
    }
}

/**
 * UNTIL: End a begin-until loop with conditional test
 * Compiles conditional branch back to BEGIN if top of stack is false (0)
 */
void until_word(void)
{
    if (state) // Only works in compile mode
    {
        if (branch_stack_empty())
        {
            error("UNTIL without matching BEGIN");
            return;
        }

        BranchEntry entry = branch_stack_pop();
        if (entry.type != CF_BEGIN)
        {
            error("UNTIL without matching BEGIN");
            return;
        }

        // Compile conditional branch back to BEGIN
        code_buffer[code_sp++] = OP_0BRANCH; // Branch if false
        code_buffer[code_sp++] = OP_LIT;     // Literal marker
        // Negative offset to branch backward to BEGIN
        code_buffer[code_sp++] = (entry.origin - code_sp);
    }
    else
    {
        error("UNTIL used outside of compilation mode");
    }
}

/**
 * WHILE: Conditional test within begin-while-repeat loop
 * Compiles conditional branch to exit loop if condition is false
 */
void while_word(void)
{
    if (state) // Only works in compile mode
    {
        if (branch_stack_empty())
        {
            error("WHILE without matching BEGIN");
            return;
        }

        BranchEntry entry = branch_stack_peek();
        if (entry.type != CF_BEGIN)
        {
            error("WHILE without matching BEGIN");
            return;
        }

        // Compile conditional exit branch (like IF)
        branch_stack_push(code_sp, CF_WHILE);
        code_buffer[code_sp++] = OP_0BRANCH; // Branch if false
        code_buffer[code_sp++] = OP_LIT;     // Literal marker
        code_buffer[code_sp++] = 0;          // Placeholder for exit offset
    }
    else
    {
        error("WHILE used outside of compilation mode");
    }
}

/**
 * REPEAT: Complete begin-while-repeat loop structure
 * Compiles unconditional branch back to BEGIN and fixes WHILE exit offset
 */
void repeat_word(void)
{
    if (state) // Only works in compile mode
    {
        if (branch_stack_empty())
        {
            error("REPEAT without matching BEGIN-WHILE");
            return;
        }

        BranchEntry while_entry = branch_stack_pop();
        if (while_entry.type != CF_WHILE)
        {
            error("REPEAT without matching WHILE");
            return;
        }

        if (branch_stack_empty())
        {
            error("REPEAT without matching BEGIN");
            return;
        }

        BranchEntry begin_entry = branch_stack_pop();
        if (begin_entry.type != CF_BEGIN)
        {
            error("REPEAT without matching BEGIN");
            return;
        }

        // Compile unconditional branch back to BEGIN
        code_buffer[code_sp++] = OP_BRANCH; // Always branch
        code_buffer[code_sp++] = OP_LIT;    // Literal marker
        // Negative offset to branch backward to BEGIN
        code_buffer[code_sp++] = (begin_entry.origin - code_sp);

        // Fix WHILE branch to exit to current position (after loop)
        code_buffer[while_entry.origin + 2] = code_sp - (while_entry.origin + 2);
    }
    else
    {
        error("REPEAT used outside of compilation mode");
    }
}

/**
 * DO: Start a counted loop (limit start -- )
 * Pops start and limit from stack, pushes them to return stack for LOOP
 */
void do_word(void)
{
    if (state) // Only works in compile mode
    {
        branch_stack_push(code_sp, CF_DO); // Track loop start
        code_buffer[code_sp++] = OP_DO;    // Compile DO opcode
    }
    else
    {
        error("DO used outside of compilation mode");
    }
}

/**
 * LOOP: End a DO loop, increment index and test against limit
 * Compiles LOOP opcode with backward branch offset to DO position
 */
void loop_word(void)
{
    if (state) // Only works in compile mode
    {
        if (branch_stack_empty())
        {
            error("LOOP without matching DO");
            return;
        }

        BranchEntry entry = branch_stack_pop();
        if (entry.type != CF_DO)
        {
            error("LOOP without matching DO");
            return;
        }

        code_buffer[code_sp++] = OP_LOOP; // Compile LOOP opcode
        code_buffer[code_sp++] = OP_LIT;  // Literal marker
        // Backward offset to branch to instruction after DO
        code_buffer[code_sp++] = (entry.origin + 1 - code_sp);
    }
    else
    {
        error("LOOP used outside of compilation mode");
    }
}

// Initialize the interpreter - Set up all core data structures and built-in words
void forth_init(void)
{
    // Initialize core data structures
    dict_init();
    data_stack.sp = -1;        // Empty data stack
    return_stack.sp = -1;      // Empty return stack
    branch_stack.top = -1;     // Empty branch stack
    code_sp = 0;               // Reset code buffer pointer
    current_word = NULL;       // No word being compiled
    base = 10;                 // Default to decimal number base
    state = 0;                 // Start in interpret mode

    // Add all built-in words to the dictionary
    Word *w_plus = malloc(sizeof(Word));
    strcpy(w_plus->name, "+");
    w_plus->func = plus;
    w_plus->code = NULL;
    w_plus->code_size = 0;
    w_plus->immediate = 0;
    w_plus->next = NULL;
    dict_add(w_plus);

    Word *w_minus = malloc(sizeof(Word));
    strcpy(w_minus->name, "-");
    w_minus->func = minus;
    w_minus->code = NULL;
    w_minus->code_size = 0;
    w_minus->immediate = 0;
    w_minus->next = NULL;
    dict_add(w_minus);

    Word *w_star = malloc(sizeof(Word));
    strcpy(w_star->name, "*");
    w_star->func = star;
    w_star->code = NULL;
    w_star->code_size = 0;
    w_star->immediate = 0;
    w_star->next = NULL;
    dict_add(w_star);

    Word *w_slash = malloc(sizeof(Word));
    strcpy(w_slash->name, "/");
    w_slash->func = slash;
    w_slash->code = NULL;
    w_slash->code_size = 0;
    w_slash->immediate = 0;
    w_slash->next = NULL;
    dict_add(w_slash);

    Word *w_mod = malloc(sizeof(Word));
    strcpy(w_mod->name, "mod");
    w_mod->func = mod;
    w_mod->code = NULL;
    w_mod->code_size = 0;
    w_mod->immediate = 0;
    w_mod->next = NULL;
    dict_add(w_mod);

    Word *w_dup = malloc(sizeof(Word));
    strcpy(w_dup->name, "dup");
    w_dup->func = dup;
    w_dup->code = NULL;
    w_dup->code_size = 0;
    w_dup->immediate = 0;
    w_dup->next = NULL;
    dict_add(w_dup);

    Word *w_drop = malloc(sizeof(Word));
    strcpy(w_drop->name, "drop");
    w_drop->func = drop;
    w_drop->code = NULL;
    w_drop->code_size = 0;
    w_drop->immediate = 0;
    w_drop->next = NULL;
    dict_add(w_drop);

    Word *w_swap = malloc(sizeof(Word));
    strcpy(w_swap->name, "swap");
    w_swap->func = swap;
    w_swap->code = NULL;
    w_swap->code_size = 0;
    w_swap->immediate = 0;
    w_swap->next = NULL;
    dict_add(w_swap);

    Word *w_over = malloc(sizeof(Word));
    strcpy(w_over->name, "over");
    w_over->func = over;
    w_over->code = NULL;
    w_over->code_size = 0;
    w_over->immediate = 0;
    w_over->next = NULL;
    dict_add(w_over);

    Word *w_rot = malloc(sizeof(Word));
    strcpy(w_rot->name, "rot");
    w_rot->func = rot;
    w_rot->code = NULL;
    w_rot->code_size = 0;
    w_rot->immediate = 0;
    w_rot->next = NULL;
    dict_add(w_rot);

    Word *w_nip = malloc(sizeof(Word));
    strcpy(w_nip->name, "nip");
    w_nip->func = nip;
    w_nip->code = NULL;
    w_nip->code_size = 0;
    w_nip->immediate = 0;
    w_nip->next = NULL;
    dict_add(w_nip);

    Word *w_tuck = malloc(sizeof(Word));
    strcpy(w_tuck->name, "tuck");
    w_tuck->func = tuck;
    w_tuck->code = NULL;
    w_tuck->code_size = 0;
    w_tuck->immediate = 0;
    w_tuck->next = NULL;
    dict_add(w_tuck);

    Word *w_equal = malloc(sizeof(Word));
    strcpy(w_equal->name, "=");
    w_equal->func = equal;
    w_equal->code = NULL;
    w_equal->code_size = 0;
    w_equal->immediate = 0;
    w_equal->next = NULL;
    dict_add(w_equal);

    Word *w_less = malloc(sizeof(Word));
    strcpy(w_less->name, "<");
    w_less->func = less_than;
    w_less->code = NULL;
    w_less->code_size = 0;
    w_less->immediate = 0;
    w_less->next = NULL;
    dict_add(w_less);

    Word *w_greater = malloc(sizeof(Word));
    strcpy(w_greater->name, ">");
    w_greater->func = greater_than;
    w_greater->code = NULL;
    w_greater->code_size = 0;
    w_greater->immediate = 0;
    w_greater->next = NULL;
    dict_add(w_greater);

    Word *w_less_eq = malloc(sizeof(Word));
    strcpy(w_less_eq->name, "<=");
    w_less_eq->func = less_equal;
    w_less_eq->code = NULL;
    w_less_eq->code_size = 0;
    w_less_eq->immediate = 0;
    w_less_eq->next = NULL;
    dict_add(w_less_eq);

    Word *w_greater_eq = malloc(sizeof(Word));
    strcpy(w_greater_eq->name, ">=");
    w_greater_eq->func = greater_equal;
    w_greater_eq->code = NULL;
    w_greater_eq->code_size = 0;
    w_greater_eq->immediate = 0;
    w_greater_eq->next = NULL;
    dict_add(w_greater_eq);

    Word *w_not_eq = malloc(sizeof(Word));
    strcpy(w_not_eq->name, "<>");
    w_not_eq->func = not_equal;
    w_not_eq->code = NULL;
    w_not_eq->code_size = 0;
    w_not_eq->immediate = 0;
    w_not_eq->next = NULL;
    dict_add(w_not_eq);

    Word *w_and = malloc(sizeof(Word));
    strcpy(w_and->name, "and");
    w_and->func = and_op;
    w_and->code = NULL;
    w_and->code_size = 0;
    w_and->immediate = 0;
    w_and->next = NULL;
    dict_add(w_and);

    Word *w_or = malloc(sizeof(Word));
    strcpy(w_or->name, "or");
    w_or->func = or_op;
    w_or->code = NULL;
    w_or->code_size = 0;
    w_or->immediate = 0;
    w_or->next = NULL;
    dict_add(w_or);

    Word *w_not = malloc(sizeof(Word));
    strcpy(w_not->name, "not");
    w_not->func = not_op;
    w_not->code = NULL;
    w_not->code_size = 0;
    w_not->immediate = 0;
    w_not->next = NULL;
    dict_add(w_not);

    Word *w_store = malloc(sizeof(Word));
    strcpy(w_store->name, "!");
    w_store->func = store;
    w_store->code = NULL;
    w_store->code_size = 0;
    w_store->immediate = 0;
    w_store->next = NULL;
    dict_add(w_store);

    Word *w_fetch = malloc(sizeof(Word));
    strcpy(w_fetch->name, "@");
    w_fetch->func = fetch;
    w_fetch->code = NULL;
    w_fetch->code_size = 0;
    w_fetch->immediate = 0;
    w_fetch->next = NULL;
    dict_add(w_fetch);

    Word *w_create = malloc(sizeof(Word));
    strcpy(w_create->name, "CREATE");
    w_create->func = create_word;
    w_create->code = NULL;
    w_create->code_size = 0;
    w_create->immediate = 0;
    w_create->next = NULL;
    dict_add(w_create);

    Word *w_variable = malloc(sizeof(Word));
    strcpy(w_variable->name, "VARIABLE");
    w_variable->func = variable_word;
    w_variable->code = NULL;
    w_variable->code_size = 0;
    w_variable->immediate = 0;
    w_variable->next = NULL;
    dict_add(w_variable);

    Word *w_constant = malloc(sizeof(Word));
    strcpy(w_constant->name, "CONSTANT");
    w_constant->func = constant_word;
    w_constant->code = NULL;
    w_constant->code_size = 0;
    w_constant->immediate = 0;
    w_constant->next = NULL;
    dict_add(w_constant);

    Word *w_dot = malloc(sizeof(Word));
    strcpy(w_dot->name, ".");
    w_dot->func = dot;
    w_dot->code = NULL;
    w_dot->code_size = 0;
    w_dot->immediate = 0;
    w_dot->next = NULL;
    dict_add(w_dot);
    Word *w_dot_quote = malloc(sizeof(Word));
    strcpy(w_dot_quote->name, ".\"");
    w_dot_quote->func = dot_quote;
    w_dot_quote->code = NULL;
    w_dot_quote->code_size = 0;
    w_dot_quote->immediate = 1;
    w_dot_quote->next = NULL;
    dict_add(w_dot_quote);
    Word *w_cells = malloc(sizeof(Word));
    strcpy(w_cells->name, "cells");
    w_cells->func = cells_word;
    w_cells->code = NULL;
    w_cells->code_size = 0;
    w_cells->immediate = 0;
    w_cells->next = NULL;
    dict_add(w_cells);

    Word *w_allot = malloc(sizeof(Word));
    strcpy(w_allot->name, "allot");
    w_allot->func = allot_word;
    w_allot->code = NULL;
    w_allot->code_size = 0;
    w_allot->immediate = 0;
    w_allot->next = NULL;
    dict_add(w_allot);

    Word *w_i = malloc(sizeof(Word));
    strcpy(w_i->name, "i");
    w_i->func = i_word;
    w_i->code = NULL;
    w_i->code_size = 0;
    w_i->immediate = 0;
    w_i->next = NULL;
    dict_add(w_i);

    Word *w_j = malloc(sizeof(Word));
    strcpy(w_j->name, "j");
    w_j->func = j_word;
    w_j->code = NULL;
    w_j->code_size = 0;
    w_j->immediate = 0;
    w_j->next = NULL;
    dict_add(w_j);

    Word *w_dot_s = malloc(sizeof(Word));
    strcpy(w_dot_s->name, ".s");
    w_dot_s->func = dot_s;
    w_dot_s->code = NULL;
    w_dot_s->code_size = 0;
    w_dot_s->immediate = 0;
    w_dot_s->next = NULL;
    dict_add(w_dot_s);

    Word *w_cr = malloc(sizeof(Word));
    strcpy(w_cr->name, "cr");
    w_cr->func = cr;
    w_cr->code = NULL;
    w_cr->code_size = 0;
    w_cr->immediate = 0;
    w_cr->next = NULL;
    dict_add(w_cr);

    // Add control flow words
    Word *w_if = malloc(sizeof(Word));
    strcpy(w_if->name, "if");
    w_if->func = if_word;
    w_if->code = NULL;
    w_if->code_size = 0;
    w_if->immediate = 1;
    w_if->next = NULL;
    dict_add(w_if);

    Word *w_then = malloc(sizeof(Word));
    strcpy(w_then->name, "then");
    w_then->func = then_word;
    w_then->code = NULL;
    w_then->code_size = 0;
    w_then->immediate = 1;
    w_then->next = NULL;
    dict_add(w_then);

    Word *w_else = malloc(sizeof(Word));
    strcpy(w_else->name, "else");
    w_else->func = else_word;
    w_else->code = NULL;
    w_else->code_size = 0;
    w_else->immediate = 1;
    w_else->next = NULL;
    dict_add(w_else);

    Word *w_begin = malloc(sizeof(Word));
    strcpy(w_begin->name, "begin");
    w_begin->func = begin_word;
    w_begin->code = NULL;
    w_begin->code_size = 0;
    w_begin->immediate = 1;
    w_begin->next = NULL;
    dict_add(w_begin);

    Word *w_until = malloc(sizeof(Word));
    strcpy(w_until->name, "until");
    w_until->func = until_word;
    w_until->code = NULL;
    w_until->code_size = 0;
    w_until->immediate = 1;
    w_until->next = NULL;
    dict_add(w_until);

    Word *w_while = malloc(sizeof(Word));
    strcpy(w_while->name, "while");
    w_while->func = while_word;
    w_while->code = NULL;
    w_while->code_size = 0;
    w_while->immediate = 1;
    w_while->next = NULL;
    dict_add(w_while);

    Word *w_repeat = malloc(sizeof(Word));
    strcpy(w_repeat->name, "repeat");
    w_repeat->func = repeat_word;
    w_repeat->code = NULL;
    w_repeat->code_size = 0;
    w_repeat->immediate = 1;
    w_repeat->next = NULL;
    dict_add(w_repeat);

    Word *w_do = malloc(sizeof(Word));
    strcpy(w_do->name, "do");
    w_do->func = do_word;
    w_do->code = NULL;
    w_do->code_size = 0;
    w_do->immediate = 1;
    w_do->next = NULL;
    dict_add(w_do);

    Word *w_loop = malloc(sizeof(Word));
    strcpy(w_loop->name, "loop");
    w_loop->func = loop_word;
    w_loop->code = NULL;
    w_loop->code_size = 0;
    w_loop->immediate = 1;
    w_loop->next = NULL;
    dict_add(w_loop);

    Word *w_end = malloc(sizeof(Word));
    strcpy(w_end->name, "end");
    w_end->func = end_word;
    w_end->code = NULL;
    w_end->code_size = 0;
    w_end->immediate = 1;
    w_end->next = NULL;
    dict_add(w_end);

    Word *w_colon = malloc(sizeof(Word));
    strcpy(w_colon->name, ":");
    w_colon->func = colon;
    w_colon->code = NULL;
    w_colon->code_size = 0;
    w_colon->immediate = 1;
    w_colon->next = NULL;
    dict_add(w_colon);

    Word *w_semicolon = malloc(sizeof(Word));
    strcpy(w_semicolon->name, ";");
    w_semicolon->func = semicolon;
    w_semicolon->code = NULL;
    w_semicolon->code_size = 0;
    w_semicolon->immediate = 1;
    w_semicolon->next = NULL;
    dict_add(w_semicolon);
}

// Simple tokenizer - Parse input text into individual words and tokens
/**
 * Extract the next token from input stream
 * @param token Buffer to store the extracted token
 * @return Pointer to next position in input, or NULL if no more tokens
 */
char *tokenize(char *token)
{
    // Skip leading whitespace
    while (*input_pos && isspace(*input_pos))
        input_pos++;
    if (!*input_pos)  // End of input
        return NULL;

    char *start = input_pos;

    // Special handling for .\" (dot-quote) word
    if (strncmp(start, ".\"", 2) == 0) {
        input_pos += 2;
        strncpy(token, start, 2);
        token[2] = '\0';
        return input_pos;
    }

    // Find end of token (whitespace or end of input)
    while (*input_pos && !isspace(*input_pos))
        input_pos++;
    if (*input_pos)
        *input_pos++ = '\0';  // Null-terminate token and advance

    // Copy token to output buffer with length limit
    strncpy(token, start, MAX_WORD_LEN - 1);
    token[MAX_WORD_LEN - 1] = '\0';
    return input_pos;
}

/**
 * Parse a quoted string from input stream
 * @param str Buffer to store the parsed string
 * @return Pointer to parsed string, or NULL if parsing failed
 */
char *parse_string(char *str)
{
    // Skip leading whitespace
    while (*input_pos && isspace(*input_pos))
        input_pos++;

    char *start = input_pos;

    // Find closing quote
    while (*input_pos && *input_pos != '"')
        input_pos++;

    if (*input_pos == '"') {
        *input_pos++ = '\0';  // Replace quote with null terminator
        strcpy(str, start);   // Copy the string content
        return str;
    }
    return NULL;  // No closing quote found
}

// Main interpreter loop (REPL) - Read-Eval-Print Loop for interactive Forth execution
void repl(void)
{
    char line[MAX_LINE_LEN];     // Input line buffer
    char token[MAX_WORD_LEN];    // Token buffer for current word

    printf("Forth Interpreter Ready. Type 'quit' to exit.\n");

    // Main REPL loop - read lines from stdin until 'quit'
    while (fgets(line, sizeof(line), stdin))
    {
        // Check for quit command
        if (strcmp(line, "quit\n") == 0)
            break;

        // Set up input processing for this line
        current_input = line;
        input_pos = line;

        // Process each token in the input line
        while (tokenize(token) != NULL)
        {
            if (state == 1) // Compile mode - building user-defined words
            {
                Word *word = dict_find(token);
                if (word)
                {
                    if (word->immediate)
                    {
                        // Execute immediate words (like control flow) during compilation
                        execute_word(word);
                    }
                    else
                    {
                        // Compile word reference into code buffer
                        if (code_sp >= STACK_SIZE)
                        {
                            error("Code buffer overflow");
                            state = 0;  // Reset to interpret mode
                            current_word = NULL;
                            break;
                        }
                        code_buffer[code_sp++] = (Cell)word;
                    }
                }
                else
                {
                    // Try to parse token as a number for literal compilation
                    char *endptr;
                    Cell num = strtoll(token, &endptr, base);
                    if (*endptr == '\0')  // Successfully parsed as number
                    {
                        // Compile literal value
                        if (code_sp >= STACK_SIZE - 1)
                        {
                            error("Code buffer overflow");
                            state = 0;  // Reset to interpret mode
                            current_word = NULL;
                            break;
                        }
                        code_buffer[code_sp++] = OP_LIT;  // Literal opcode
                        code_buffer[code_sp++] = num;     // The literal value
                    }
                    else
                    {
                        error("Unknown word in compilation");
                        state = 0;  // Reset to interpret mode
                        current_word = NULL;
                        break;
                    }
                }
            }
            else
            { // Interpret mode - immediate execution
                Word *word = dict_find(token);
                if (word)
                {
                    // Execute the found word
                    execute_word(word);
                }
                else
                {
                    // Try to parse token as a number
                    char *endptr;
                    Cell num = strtoll(token, &endptr, base);
                    if (*endptr == '\0')  // Successfully parsed as number
                    {
                        stack_push(num);  // Push number onto data stack
                    }
                    else
                    {
                        error("Unknown word");
                    }
                }
            }
        }
        // Optional: uncomment to show stack after each line
        // print_stack();
        // cr();
    }
}

/**
 * Program entry point
 * Initialize the Forth interpreter and start the REPL
 */
int main(void)
{
    forth_init();  // Set up interpreter with built-in words
    repl();        // Start Read-Eval-Print Loop

    // Clean up allocated memory (optional, but good practice)
    for (int i = 0; i < dict.count; i++)
    {
        free(dict.words[i]);  // Free each word structure
    }
    return 0;
}
