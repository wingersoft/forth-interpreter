# Forth Interpreter in C

A simple, stack-based Forth-like language interpreter implemented in standard C. This educational project implements a subset of Forth features, allowing users to experiment with stack-based programming, user-defined words, and control flow in a lightweight, portable environment.

**Current Status**: Phase 3 (User-defined words with colon/semicolon) completed. Phase 4 (Advanced control flow and optimization) in progress. All control flow constructs (if-then-else, begin-until, begin-while-repeat, do-loop) are fully functional and tested.

## Features

- **Arithmetic Operations**: +, -, *, /, mod
- **Stack Manipulation**: dup, drop, swap, over, rot, nip, tuck
- **Comparison Operations**: =, <, >, <=, >=, <>
- **Logical Operations**: and, or, not
- **I/O Operations**: ., .s (stack display), cr (carriage return)
- **User-Defined Words**: Define custom functions with : word-name ... ; syntax
- **Memory Operations**: ! (store), @ (fetch)
- **Defining Words**: VARIABLE, CONSTANT, CREATE for expandable words
- **Control Flow**: if-then-else, begin-until, begin-while-repeat, do-loop (fully debugged and tested)
- **REPL**: Interactive read-eval-print loop for immediate feedback
- **Error Handling**: Non-fatal error recovery with state reset
- **Portable**: Standard C, no external dependencies

## Recent Improvements

- ✅ **Control Flow Debugging**: Fixed critical bugs in if-then-else, begin-until, begin-while-repeat, and do-loop implementations
- ✅ **Branch Offset Calculations**: Corrected offset storage and execution logic for all branching operations
- ✅ **Pointer Detection**: Enhanced word reference detection for proper execution
- ✅ **Error Recovery**: Improved error handling to prevent interpreter lock-ups
- ✅ **Memory Management**: Robust memory operations with bounds checking

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

- Loop (Begin-While-Repeat):
  ```
  : sum-to-10 0 1 begin dup 10 < while swap over + swap 1 + repeat drop . ; sum-to-10
  ```
  Output: `45 `

- Stack Manipulation:
  ```
  1 2 3 .s cr swap .s cr rot .s cr
  ```
  Output: `< 1 2 3 > < 1 3 2 > < 3 2 1 > `

- Variables:
  ```
  variable counter 10 counter ! counter @ . cr
  ```
  Output: `10 `

- Constants:
  ```
  42 constant answer answer . cr
  ```
  Output: `42 `

- Complex Control Flow:
  ```
  : factorial 1 swap begin dup 1 > while dup rot * swap 1 - repeat drop ; 5 factorial . cr
  ```
  Output: `120 `

## Project Structure

- `forth.h`: Header with data structures (Stack, Dictionary, Word), function prototypes, and constants
- `forth.c`: Source implementation of stack operations, dictionary management, built-in words, REPL, compiler, and execution engine
- `forth_implementation_plan.md`: Detailed development plan with phases, architecture diagram (Mermaid), and implementation details
- `README.md`: Project documentation and usage guide
- `test.forth`: Comprehensive test suite covering all features
- `basic_test.forth`, `simple_test.forth`, `demo.fth`: Additional test files
- `Makefile`: Build configuration
- `.gitignore`: Git ignore file

## Testing

### Basic Testing
Test basic operations with piped input:
```
echo -e "5 3 + . cr quit" | ./forth
```
Expected output: `8 `

### Comprehensive Test Suite
Run the full test suite:
```
./forth < test.forth
```

This tests all features including:
- Arithmetic and stack operations
- Comparison and logical operations
- User-defined words with control flow
- Memory operations with variables and constants
- All loop constructs (begin-until, begin-while-repeat, do-loop)

### Interactive Testing
For control flow and user-defined words, use the REPL interactively:
```
./forth
```

Type commands directly and see immediate results. Use `.s` to inspect the stack state.

### Test Files
- `test.forth`: Complete test suite covering all implemented features

## Extension

### Current Development Focus (Phase 4)
- Further enhance user-defined word compilation with proper token parsing
- Implement more advanced control flow (case statements, etc.)
- Add string handling and I/O words
- Optimize dictionary search and memory management
- Add debugging and introspection words
- Refine VARIABLE and CONSTANT defining words

### Future Enhancements
- Add more built-in words by implementing functions and registering in `forth_init()`
- Enhance tokenizer for multi-line definitions
- Implement full threaded code for better performance
- Add number base support for hex/binary input
- File I/O operations
- String manipulation words
- Advanced debugging tools

### Memory Bank System
This project uses a comprehensive memory bank system (`.kilocode/rules/memory-bank/`) for documentation and context management:
- `architecture.md`: System design and component relationships
- `brief.md`: Feature overview and technical summary
- `context.md`: Current work status and recent changes
- `product.md`: Project purpose and user experience goals
- `tech.md`: Technology stack and development workflow

The memory bank ensures consistent development and provides context for future enhancements.

## License

This project is open-source. Feel free to use, modify, and distribute under the MIT License.

## Credits

Built with guidance from the Forth implementation plan and standard Forth specifications. Special thanks to the comprehensive debugging effort that resolved critical control flow issues and established the memory bank documentation system for consistent development.

### Recent Contributors
- Control flow debugging and fixes
- Memory bank system implementation
- Comprehensive test suite development
- Error handling improvements