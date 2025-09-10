#include "forth.h"

// Global structures
Stack data_stack = { {0}, -1 };
Stack return_stack = { {0}, -1 };
Dictionary dict = { {NULL}, 0 };
Cell memory[STACK_SIZE] = {0};
int base = 10;
int state = 0;  // 0: interpreting, 1: compiling

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

// Initialize the interpreter
void forth_init(void) {
    dict_init();
    data_stack.sp = -1;
    return_stack.sp = -1;
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