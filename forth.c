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
    // Reset state if needed
    data_stack.sp = -1;
    exit(1);  // For simplicity; make non-fatal later
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
    printf("%d ", value);
}

void print_stack(void) {
    printf("< ");
    for (int i = 0; i <= data_stack.sp; i++) {
        printf("%d ", data_stack.stack[i]);
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

// Built-in comparison operations
void equal(void) {
    Cell b = stack_pop();
    Cell a = stack_pop();
    stack_push(a == b ? -1 : 0);
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
    // Switch to compile mode
    state = 1;
    // In a full implementation, we would also:
    // 1. Read the next token as the word name
    // 2. Create a new dictionary entry
    // 3. Set current_word to this new entry
    // For now, we'll just switch to compile mode
}

void semicolon(void) {
    if (state == 0) {
        error("Misplaced ;");
        return;
    }

    // Switch back to interpret mode
    state = 0;

    // In a full implementation, we would also:
    // 1. End the current word definition
    // 2. Add the word to the dictionary
    // 3. Reset code_sp
    // For now, we'll just switch modes
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
    w_plus->next = NULL;
    dict_add(w_plus);

    Word *w_minus = malloc(sizeof(Word));
    strcpy(w_minus->name, "-");
    w_minus->func = minus;
    w_minus->code = NULL;
    w_minus->code_size = 0;
    w_minus->next = NULL;
    dict_add(w_minus);

    Word *w_star = malloc(sizeof(Word));
    strcpy(w_star->name, "*");
    w_star->func = star;
    w_star->code = NULL;
    w_star->code_size = 0;
    w_star->next = NULL;
    dict_add(w_star);

    Word *w_slash = malloc(sizeof(Word));
    strcpy(w_slash->name, "/");
    w_slash->func = slash;
    w_slash->code = NULL;
    w_slash->code_size = 0;
    w_slash->next = NULL;
    dict_add(w_slash);

    Word *w_mod = malloc(sizeof(Word));
    strcpy(w_mod->name, "mod");
    w_mod->func = mod;
    w_mod->code = NULL;
    w_mod->code_size = 0;
    w_mod->next = NULL;
    dict_add(w_mod);

    Word *w_dup = malloc(sizeof(Word));
    strcpy(w_dup->name, "dup");
    w_dup->func = dup;
    w_dup->code = NULL;
    w_dup->code_size = 0;
    w_dup->next = NULL;
    dict_add(w_dup);

    Word *w_drop = malloc(sizeof(Word));
    strcpy(w_drop->name, "drop");
    w_drop->func = drop;
    w_drop->code = NULL;
    w_drop->code_size = 0;
    w_drop->next = NULL;
    dict_add(w_drop);

    Word *w_equal = malloc(sizeof(Word));
    strcpy(w_equal->name, "=");
    w_equal->func = equal;
    w_equal->code = NULL;
    w_equal->code_size = 0;
    w_equal->next = NULL;
    dict_add(w_equal);

    Word *w_dot = malloc(sizeof(Word));
    strcpy(w_dot->name, ".");
    w_dot->func = dot;
    w_dot->code = NULL;
    w_dot->code_size = 0;
    w_dot->next = NULL;
    dict_add(w_dot);

    Word *w_dot_s = malloc(sizeof(Word));
    strcpy(w_dot_s->name, ".s");
    w_dot_s->func = dot_s;
    w_dot_s->code = NULL;
    w_dot_s->code_size = 0;
    w_dot_s->next = NULL;
    dict_add(w_dot_s);

    Word *w_cr = malloc(sizeof(Word));
    strcpy(w_cr->name, "cr");
    w_cr->func = cr;
    w_cr->code = NULL;
    w_cr->code_size = 0;
    w_cr->next = NULL;
    dict_add(w_cr);

    // Add control flow words
    Word *w_if = malloc(sizeof(Word));
    strcpy(w_if->name, "if");
    w_if->func = if_word;
    w_if->code = NULL;
    w_if->code_size = 0;
    w_if->next = NULL;
    dict_add(w_if);

    Word *w_then = malloc(sizeof(Word));
    strcpy(w_then->name, "then");
    w_then->func = then_word;
    w_then->code = NULL;
    w_then->code_size = 0;
    w_then->next = NULL;
    dict_add(w_then);

    Word *w_else = malloc(sizeof(Word));
    strcpy(w_else->name, "else");
    w_else->func = else_word;
    w_else->code = NULL;
    w_else->code_size = 0;
    w_else->next = NULL;
    dict_add(w_else);

    Word *w_begin = malloc(sizeof(Word));
    strcpy(w_begin->name, "begin");
    w_begin->func = begin_word;
    w_begin->code = NULL;
    w_begin->code_size = 0;
    w_begin->next = NULL;
    dict_add(w_begin);

    Word *w_until = malloc(sizeof(Word));
    strcpy(w_until->name, "until");
    w_until->func = until_word;
    w_until->code = NULL;
    w_until->code_size = 0;
    w_until->next = NULL;
    dict_add(w_until);

    Word *w_while = malloc(sizeof(Word));
    strcpy(w_while->name, "while");
    w_while->func = while_word;
    w_while->code = NULL;
    w_while->code_size = 0;
    w_while->next = NULL;
    dict_add(w_while);

    Word *w_repeat = malloc(sizeof(Word));
    strcpy(w_repeat->name, "repeat");
    w_repeat->func = repeat_word;
    w_repeat->code = NULL;
    w_repeat->code_size = 0;
    w_repeat->next = NULL;
    dict_add(w_repeat);

    Word *w_end = malloc(sizeof(Word));
    strcpy(w_end->name, "end");
    w_end->func = end_word;
    w_end->code = NULL;
    w_end->code_size = 0;
    w_end->next = NULL;
    dict_add(w_end);

    Word *w_colon = malloc(sizeof(Word));
    strcpy(w_colon->name, ":");
    w_colon->func = colon;
    w_colon->code = NULL;
    w_colon->code_size = 0;
    w_colon->next = NULL;
    dict_add(w_colon);

    Word *w_semicolon = malloc(sizeof(Word));
    strcpy(w_semicolon->name, ";");
    w_semicolon->func = semicolon;
    w_semicolon->code = NULL;
    w_semicolon->code_size = 0;
    w_semicolon->next = NULL;
    dict_add(w_semicolon);
}

// Simple tokenizer (basic implementation)
char *tokenize(char *line, char *token) {
    // Skip whitespace
    while (*line && isspace(*line)) line++;
    if (!*line) return NULL;

    char *start = line;
    while (*line && !isspace(*line)) line++;
    if (*line) *line++ = '\0';

    strncpy(token, start, MAX_WORD_LEN - 1);
    token[MAX_WORD_LEN - 1] = '\0';
    return line;
}

// Main interpreter loop (REPL)
void repl(void) {
    char line[MAX_LINE_LEN];
    char token[MAX_WORD_LEN];

    printf("Forth Interpreter Ready. Type 'quit' to exit.\n");

    while (fgets(line, sizeof(line), stdin)) {
        if (strcmp(line, "quit\n") == 0) break;

        char *pos = line;
        while ((pos = tokenize(pos, token)) != NULL) {
            Word *word = dict_find(token);
            if (word) {
                // Execute word (for now, assume built-in)
                if (word->func) {
                    word->func();
                } else {
                    // Handle user-defined word execution
                    // Implementation pending
                }
            } else {
                // Try to parse as number
                Cell num = strtol(token, NULL, base);
                stack_push(num);
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