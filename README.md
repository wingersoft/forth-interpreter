# Forth Interpreter in C

A simple, stack-based Forth-like language interpreter implemented in standard C. This educational project implements a subset of Forth features, allowing users to experiment with stack-based programming, user-defined words, and control flow in a lightweight, portable environment.

## Features

- **Arithmetic Operations**: +, -, *, /, mod
- **Stack Manipulation**: dup, drop
- **Comparison Operations**: =, <, >, <=, >=, <>, 0=
- **Logical Operations**: and, or, not
- **I/O Operations**: ., .s (stack display), cr (carriage return)
- **User-Defined Words**: Define custom functions with : word-name ... ; syntax
- **Memory Operations**: ! (store), @ (fetch)
- **Control Flow**: if-then-else, begin-until, begin-while-repeat
- **CREATE Word**: Define expandable words
- **REPL**: Interactive read-eval-print loop for immediate feedback
- **Portable**: Standard C, no external dependencies

## Compilation

Compile with GCC:
```
gcc -Wall -o forth forth.c
```

## Usage

Run the interpreter:
```
./forth
```

The REPL will start with "Forth Interpreter Ready. Type 'quit' to exit."

### Basic Examples

- Arithmetic:
  ```
  5 3 + . cr
  ```
  Output: `8 `

- Stack Manipulation:
  ```
  10 dup . cr .s cr
  ```
  Output: `10 < 10 10 > `

- User-Defined Word:
  ```
  : double dup + ; 5 double . cr
  ```
  Output: `10 `

- Control Flow (If-Then):
  ```
  : test 1 if 42 . then ; test cr
  ```
  Output: `42 `

- Memory:
  ```
  10 42 ! 10 @ . cr
  ```
  Output: `42 `

- Loop (Begin-Until):
  ```
  : countdown 10 begin dup . 1 - dup 0= until drop ; countdown
  ```
  Output: `10 9 8 7 6 5 4 3 2 1 `

## Project Structure

- `forth.h`: Header with data structures (Stack, Dictionary, Word), function prototypes, and constants
- `forth.c`: Source implementation of stack operations, dictionary management, built-in words, REPL, compiler, and execution engine
- `forth_implementation_plan.md`: Detailed development plan with phases, architecture diagram (Mermaid), and implementation details
- `.gitignore`: Excludes .kilocode/ directory

## Testing

Test basic operations with piped input:
```
echo -e "5 3 + . cr quit" | ./forth
```

Expected output: `8 `

For control flow and user-defined words, use the REPL interactively.

## Extension

- Add more built-in words by implementing functions and registering in `forth_init()`
- Enhance tokenizer for multi-line definitions
- Implement full threaded code for better performance
- Add number base support for hex/binary input

## License

This project is open-source. Feel free to use, modify, and distribute under the MIT License.

## Credits

Built with guidance from the Forth implementation plan and standard Forth specifications.