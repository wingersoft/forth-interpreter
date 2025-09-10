#include "forth.h"

// Global structures
Stack data_stack = { {0}, -1 };
Stack return_stack = { {0}, -1 };
Dictionary dict = { {NULL}, 0 };
Cell memory[STACK_SIZE] = {0};
Cell code_buffer[STACK_SIZE] = {0};
int base = 10;
int state = 0;  // 0: interpreting, 1: compiling
int code_sp = 0; // Code stack pointer
Word *current_word = NULL; // Current word being compiled
int next_mem_addr = 0; // Next available memory address

// Input handling
char *current_input = NULL;
char *input_pos = NULL;

// Control flow
BranchStack branch_stack = { {{0, CF_END}}, -1 };

// Stack operations
void stack_push(Cell value) {
    if (data_stack.sp >= STACK_SIZE - 1) {
        error("Stack overflow");
        return;
    }
    data_stack.stack[++data_stack.sp] = value;
}

Cell stack_pop(void) {
    if (data_stack.sp < 0) {
        error("Stack underflow");
        return 0;
    }
    return data_stack.stack[data_stack.sp--];
}

Cell stack_peek(void) {
    if (data_stack.sp < 0) {
        error("Stack underflow");
        return 0;
    }
    return data_stack.stack[data_stack.sp];
}

int stack_empty(void) {
    return data_stack.sp < 0;
}

int stack_full(void) {
    return data_stack.sp >= STACK_SIZE - 1;
}

// Branch stack operations
void branch_stack_push(int origin, ControlFlowType type) {
    if (branch_stack.top >= STACK_SIZE - 1) {
        error("Branch stack overflow");
        return;
    }
    branch_stack.top++;
    branch_stack.entries[branch_stack.top].origin = origin;
    branch_stack.entries[branch_stack.top].type = type;
}

BranchEntry branch_stack_pop(void) {
    if (branch_stack.top < 0) {
        error("Branch stack underflow");
        BranchEntry empty = {0, CF_END};
        return empty;
    }
    return branch_stack.entries[branch_stack.top--];
}

BranchEntry branch_stack_peek(void) {
    if (branch_stack.top < 0) {
        error("Branch stack underflow");
        BranchEntry empty = {0, CF_END};
        return empty;
    }
    return branch_stack.entries[branch_stack.top];
}

int branch_stack_empty(void) {
    return branch_stack.top < 0;
}

// Dictionary operations
void dict_init(void) {
    dict.count = 0;
    // Initialize built-in words here later
    memset(dict.words, 0, sizeof(dict.words));
}

Word *dict_find(const char *name) {
    // Simple linear search for now; optimize with hash later
    for (int i = 0; i < dict.count; i++) {
        if (strcmp(dict.words[i]->name, name) == 0) {
            return dict.words[i];
        }
    }
    return NULL;
}

void dict_add(Word *word) {
    if (dict.count >= DICT_SIZE) {
        error("Dictionary full");
        return;
    }
    dict.words[dict.count++] = word;
}

// Basic error handling
void error(const char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
    // Reset stacks and state to continue execution
    data_stack.sp = -1;
    return_stack.sp = -1;
    branch_stack.top = -1;
    state = 0;  // Back to interpret mode
    code_sp = 0;
    current_word = NULL;
}

// Memory operations
void mem_store(int addr, Cell value) {
    if (addr < 0 || addr >= STACK_SIZE) {
        error("Invalid memory address");
        return;
    }
    memory[addr] = value;
}

Cell mem_fetch(int addr) {
    if (addr < 0 || addr >= STACK_SIZE) {
        error("Invalid memory address");
        return 0;
    }
    return memory[addr];
}

// I/O operations
void print_cell(Cell value) {
    printf("%lld ", value);
}

void print_stack(void) {
    printf("< ");
    for (int i = 0; i <= data_stack.sp; i++) {
        printf("%lld ", data_stack.stack[i]);
    }
    printf("> ");
}

void cr(void) {
    printf("\n");
}

// Built-in arithmetic operations
void plus(void) {
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(a + b);
}

void minus(void) {
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(a - b);
}

void star(void) {
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(a * b);
}

void slash(void) {
    Cell b = stack_pop();
    if (b == 0) {
        error("Division by zero");
        return;
    }
    Cell a = stack_pop();
    stack_push(a / b);
}

void mod(void) {
    Cell b = stack_pop();
    if (b == 0) {
        error("Modulo by zero");
        return;
    }
    Cell a = stack_pop();
    stack_push(a % b);
}

// Built-in stack operations
void dup(void) {
    Cell top = stack_peek();
    stack_push(top);
}

void drop(void) {
    stack_pop();
}

void swap(void) {
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(b);
    stack_push(a);
}

void over(void) {
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(a);
    stack_push(b);
    stack_push(a);
}

void rot(void) {
    Cell c = stack_pop();
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(b);
    stack_push(c);
    stack_push(a);
}

void nip(void) {
    Cell b = stack_pop();
    stack_pop(); // discard a
    stack_push(b);
}

void tuck(void) {
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(b);
    stack_push(a);
    stack_push(b);
}

// Built-in comparison operations
void equal(void) {
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(a == b ? -1 : 0);
}

void less_than(void) {
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(a < b ? -1 : 0);
}

void greater_than(void) {
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(a > b ? -1 : 0);
}

void less_equal(void) {
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(a <= b ? -1 : 0);
}

void greater_equal(void) {
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(a >= b ? -1 : 0);
}

void not_equal(void) {
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(a != b ? -1 : 0);
}

// Built-in logical operations
void and_op(void) {
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(a & b);
}

void or_op(void) {
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(a | b);
}

void not_op(void) {
    Cell a = stack_pop();
    stack_push(~a);
}

// Built-in memory operations
void store(void) {
    Cell addr = stack_pop();
    Cell value = stack_pop();
    mem_store(addr, value);
}

void fetch(void) {
    Cell addr = stack_pop();
    Cell value = mem_fetch(addr);
    stack_push(value);
}

// Built-in CREATE word
void create_word(void) {
    // In a full implementation, this would read the next token
    // For now, we'll create a simple variable word
    // This is a placeholder implementation
    int addr = next_mem_addr++;
    Word *new_word = malloc(sizeof(Word));
    // For simplicity, create a word named "var" + addr
    char name[32];
    sprintf(name, "var%d", addr);
    strcpy(new_word->name, name);
    new_word->func = NULL;
    new_word->code = malloc(sizeof(Cell));
    new_word->code[0] = addr; // Push address
    new_word->code_size = 1;
    new_word->next = NULL;
    dict_add(new_word);
    // Push the address to stack for immediate use
    stack_push(addr);
}

// Built-in VARIABLE word
void variable_word(void) {
    char name[MAX_WORD_LEN];
    if (!tokenize(name)) {
        error("VARIABLE needs a name");
        return;
    }
    int addr = next_mem_addr++;
    Word *new_word = malloc(sizeof(Word));
    strcpy(new_word->name, name);
    new_word->func = NULL;
    new_word->code = malloc(2 * sizeof(Cell));
    new_word->code[0] = OP_LIT;
    new_word->code[1] = addr;
    new_word->code_size = 2;
    new_word->next = NULL;
    dict_add(new_word);
}

// Built-in CONSTANT word
void constant_word(void) {
    char name[MAX_WORD_LEN];
    if (!tokenize(name)) {
        error("CONSTANT needs a name");
        return;
    }
    Cell value = stack_pop();
    Word *new_word = malloc(sizeof(Word));
    strcpy(new_word->name, name);
    new_word->func = NULL;
    new_word->code = malloc(2 * sizeof(Cell));
    new_word->code[0] = OP_LIT;
    new_word->code[1] = value;
    new_word->code_size = 2;
    new_word->next = NULL;
    dict_add(new_word);
}

// Execute user-defined word
void execute_word(Word *word) {
    if (word->func) {
        word->func();
        return;
    }

    // Execute code for user-defined word
    for (int i = 0; i < word->code_size; i++) {
        Cell item = word->code[i];
        if (item == OP_LIT) {
            // Next item is literal value
            i++;
            stack_push(word->code[i]);
        } else if (item == OP_BRANCH) {
            // Unconditional branch
            i++;
            i += word->code[i] - 1; // -1 because loop will increment
        } else if (item == OP_0BRANCH) {
            // Conditional branch
            i++;
            Cell top = stack_pop();
            if (top == 0) {
                i += word->code[i] - 1;
            }
        } else {
            // Assume it's an address of another word
            Word *w = (Word *)item;
            execute_word(w);
        }
    }
}

// Built-in I/O operations
void dot(void) {
    Cell value = stack_pop();
    print_cell(value);
}

void dot_s(void) {
    print_stack();
}

// Control flow functions
void if_word(void) {
    // In compile mode, compile a conditional branch
    if (state) {
        // Push current code position and IF type to branch stack
        branch_stack_push(code_sp, CF_IF);
        // Compile 0BRANCH opcode (conditional branch)
        code_buffer[code_sp++] = OP_0BRANCH;
        // Reserve space for branch offset (will be filled in by THEN/ELSE)
        code_buffer[code_sp++] = 0;
    } else {
        error("IF used outside of compilation mode");
    }
}

void then_word(void) {
    if (state) {
        if (branch_stack_empty()) {
            error("THEN without matching IF");
            return;
        }

        BranchEntry entry = branch_stack_pop();
        if (entry.type != CF_IF && entry.type != CF_ELSE) {
            error("THEN without matching IF");
            return;
        }

        // Fill in the branch offset
        code_buffer[entry.origin + 1] = code_sp - (entry.origin + 1);
    } else {
        error("THEN used outside of compilation mode");
    }
}

void else_word(void) {
    if (state) {
        if (branch_stack_empty()) {
            error("ELSE without matching IF");
            return;
        }

        BranchEntry entry = branch_stack_peek();
        if (entry.type != CF_IF) {
            error("ELSE without matching IF");
            return;
        }

        // First, complete the IF branch to jump over the ELSE part
        // Compile unconditional branch
        code_buffer[code_sp++] = OP_BRANCH;
        // Reserve space for branch offset (will be filled in by THEN)
        int else_branch_origin = code_sp;
        code_buffer[code_sp++] = 0;

        // Fix up the IF branch to point to here
        code_buffer[entry.origin + 1] = code_sp - (entry.origin + 1);

        // Pop the IF entry and push the ELSE entry
        branch_stack_pop();
        branch_stack_push(else_branch_origin, CF_ELSE);
    } else {
        error("ELSE used outside of compilation mode");
    }
}

void end_word(void) {
    // This would be used to end definitions, but we already have semicolon()
    // This function might be redundant or used for other purposes
    // For now, we'll leave it as a placeholder
}

void colon(void) {
    char word_name[MAX_WORD_LEN];

    // Read the next token as the word name
    if (!tokenize(word_name)) {
        error("Expected word name after :");
        return;
    }

    // Check if word already exists
    if (dict_find(word_name)) {
        error("Word already exists");
        return;
    }

    // Create new word
    Word *new_word = malloc(sizeof(Word));
    if (!new_word) {
        error("Memory allocation failed");
        return;
    }

    strcpy(new_word->name, word_name);
    new_word->func = NULL;
    new_word->code = malloc(STACK_SIZE * sizeof(Cell));
    if (!new_word->code) {
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

void semicolon(void) {
    if (state == 0) {
        error("Misplaced ;");
        return;
    }

    if (!current_word) {
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

void begin_word(void) {
    if (state) {
        // Push current code position and BEGIN type to branch stack
        branch_stack_push(code_sp, CF_BEGIN);
    } else {
        error("BEGIN used outside of compilation mode");
    }
}

void until_word(void) {
    if (state) {
        if (branch_stack_empty()) {
            error("UNTIL without matching BEGIN");
            return;
        }

        BranchEntry entry = branch_stack_pop();
        if (entry.type != CF_BEGIN) {
            error("UNTIL without matching BEGIN");
            return;
        }

        // Compile conditional branch back to BEGIN
        code_buffer[code_sp++] = OP_0BRANCH;
        // Branch offset is negative to go backward
        code_buffer[code_sp++] = (entry.origin - code_sp);
    } else {
        error("UNTIL used outside of compilation mode");
    }
}

void while_word(void) {
    if (state) {
        if (branch_stack_empty()) {
            error("WHILE without matching BEGIN");
            return;
        }

        BranchEntry entry = branch_stack_peek();
        if (entry.type != CF_BEGIN) {
            error("WHILE without matching BEGIN");
            return;
        }

        // Compile conditional branch to exit loop (like IF)
        branch_stack_push(code_sp, CF_WHILE);
        code_buffer[code_sp++] = OP_0BRANCH;
        // Reserve space for branch offset (will be filled in by REPEAT)
        code_buffer[code_sp++] = 0;
    } else {
        error("WHILE used outside of compilation mode");
    }
}

void repeat_word(void) {
    if (state) {
        if (branch_stack_empty()) {
            error("REPEAT without matching BEGIN-WHILE");
            return;
        }

        BranchEntry while_entry = branch_stack_pop();
        if (while_entry.type != CF_WHILE) {
            error("REPEAT without matching WHILE");
            return;
        }

        if (branch_stack_empty()) {
            error("REPEAT without matching BEGIN");
            return;
        }

        BranchEntry begin_entry = branch_stack_pop();
        if (begin_entry.type != CF_BEGIN) {
            error("REPEAT without matching BEGIN");
            return;
        }

        // Compile unconditional branch back to BEGIN
        code_buffer[code_sp++] = OP_BRANCH;
        // Branch offset is negative to go backward
        code_buffer[code_sp++] = (begin_entry.origin - code_sp);

        // Fix up the WHILE branch to point to after the loop
        code_buffer[while_entry.origin + 1] = code_sp - (while_entry.origin + 1);
    } else {
        error("REPEAT used outside of compilation mode");
    }
}

void do_word(void) {
    if (state) {
        // Push current code position and DO type to branch stack
        branch_stack_push(code_sp, CF_DO);
        // In a full implementation, we would compile code to set up the loop variables
        // For now, this is a placeholder
    } else {
        error("DO used outside of compilation mode");
    }
}

void loop_word(void) {
    if (state) {
        if (branch_stack_empty()) {
            error("LOOP without matching DO");
            return;
        }

        BranchEntry entry = branch_stack_pop();
        if (entry.type != CF_DO) {
            error("LOOP without matching DO");
            return;
        }

        // In a full implementation, we would compile code to increment index and check limit
        // For now, compile a simple branch back
        code_buffer[code_sp++] = OP_BRANCH;
        code_buffer[code_sp++] = (entry.origin - code_sp);
    } else {
        error("LOOP used outside of compilation mode");
    }
}

// Initialize the interpreter
void forth_init(void) {
    dict_init();
    data_stack.sp = -1;
    return_stack.sp = -1;
    branch_stack.top = -1;
    code_sp = 0;
    current_word = NULL;
    base = 10;
    state = 0;

    // Add built-in words
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

// Simple tokenizer (basic implementation)
char *tokenize(char *token) {
    // Skip whitespace
    while (*input_pos && isspace(*input_pos)) input_pos++;
    if (!*input_pos) return NULL;

    char *start = input_pos;
    while (*input_pos && !isspace(*input_pos)) input_pos++;
    if (*input_pos) *input_pos++ = '\0';

    strncpy(token, start, MAX_WORD_LEN - 1);
    token[MAX_WORD_LEN - 1] = '\0';
    return input_pos;
}

// Main interpreter loop (REPL)
void repl(void) {
    char line[MAX_LINE_LEN];
    char token[MAX_WORD_LEN];

    printf("Forth Interpreter Ready. Type 'quit' to exit.\n");

    while (fgets(line, sizeof(line), stdin)) {
        if (strcmp(line, "quit\n") == 0) break;

        current_input = line;
        input_pos = line;
        while (tokenize(token) != NULL) {
            if (state == 1) { // Compile mode
                Word *word = dict_find(token);
                if (word) {
                    if (word->immediate) {
                        // Execute immediate word even in compile mode
                        execute_word(word);
                    } else {
                        // Compile word reference
                        if (code_sp >= STACK_SIZE) {
                            error("Code buffer overflow");
                            state = 0;
                            current_word = NULL;
                            break;
                        }
                        code_buffer[code_sp++] = (Cell)word;
                    }
                } else {
                    // Try to parse as number
                    char *endptr;
                    Cell num = strtoll(token, &endptr, base);
                    if (*endptr == '\0') {
                        // Compile literal
                        if (code_sp >= STACK_SIZE - 1) {
                            error("Code buffer overflow");
                            state = 0;
                            current_word = NULL;
                            break;
                        }
                        code_buffer[code_sp++] = OP_LIT;
                        code_buffer[code_sp++] = num;
                    } else {
                        error("Unknown word in compilation");
                        state = 0;
                        current_word = NULL;
                        break;
                    }
                }
            } else { // Interpret mode
                Word *word = dict_find(token);
                if (word) {
                    // Execute word
                    execute_word(word);
                } else {
                    // Try to parse as number
                    char *endptr;
                    Cell num = strtoll(token, &endptr, base);
                    if (*endptr == '\0') {
                        stack_push(num);
                    } else {
                        error("Unknown word");
                    }
                }
            }
        }
        print_stack();
        cr();
    }
}

int main(void) {
    forth_init();
    repl();
    // Free allocated words on exit (for completeness)
    for (int i = 0; i < dict.count; i++) {
        free(dict.words[i]);
    }
    return 0;
}