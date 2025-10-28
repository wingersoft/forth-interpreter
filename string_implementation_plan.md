# String Handling Implementation Plan - Phase 3

## Overview
This plan focuses on implementing the **core essential** string handling features for the Forth interpreter, starting with three fundamental operations that enable basic string manipulation.

## Core Features (Phase 3A)

### 1. TYPE - Display String
**Signature:** `( c-addr u -- )`

**Purpose:** Output u characters starting at address c-addr

**Implementation:**
- Loop through u characters starting at c-addr
- Use putchar() for each character
- No stack manipulation needed after consuming inputs

**Usage Example:**
```forth
." Hello" TYPE    \ Displays "Hello"
```

### 2. COUNT - Counted String to Address/Length
**Signature:** `( c-addr -- c-addr+1 u )`

**Purpose:** Convert counted string (first byte is length) to modern addr+length format

**Implementation:**
- Read length byte at c-addr (memory[c-addr] & 0xFF)
- Push c-addr+1 (address of first character)
- Push length u

**Usage Example:**
```forth
mystring COUNT TYPE    \ Display a counted string
```

### 3. S" - String Literal (Compile and Interpret Mode)
**Signature:** `( "ccc<quote>" -- c-addr u )`

**Purpose:** Parse string until closing quote and return address and length

**Behavior:**
- **Interpret mode**: Store string in transient buffer, return buffer address and length
- **Compile mode**: Compile string inline, emit code to push address and length at runtime

**Implementation Strategy:**
```
Interpret Mode:
1. Parse string until closing quote
2. Copy to transient string buffer
3. Push buffer address
4. Push string length

Compile Mode:
1. Parse string until closing quote
2. Compile OP_LIT with address to string storage
3. Compile string bytes into code buffer
4. Compile OP_LIT with string length
5. Update code pointers
```

## Memory Architecture

### String Buffer Layout
```
memory[0]        ... memory[99]     - Variables (existing)
memory[100]      ... memory[299]    - Transient string buffer (200 bytes)
memory[300]      ... memory[1023]   - Available for future use
```

### Global Variables Needed
```c
#define STRING_BUFFER_START 100
#define STRING_BUFFER_SIZE 200
int string_buffer_pos = STRING_BUFFER_START;  // Current position in transient buffer
```

### Transient Buffer Strategy
- Used only in interpret mode
- Overwrites previous strings (transient)
- Resets after each use (wraps around)
- Sufficient for interactive use and passing to TYPE

## Implementation Details

### S" Compilation Format
When S" is compiled into a user-defined word, the format is:
```
[OP_LIT] [string_addr] [char1] [char2] ... [charN] [OP_LIT] [length]
```

At execution time:
1. OP_LIT pushes the address (position of char1)
2. String data is skipped over
3. OP_LIT pushes the length

### Parse String Helper
Enhance existing `parse_string()` function to:
- Handle both interpret and compile contexts
- Return string address and length
- Work with both transient buffer and inline compilation

## New Data Structures

### String Descriptor (optional, for future)
```c
typedef struct {
    Cell addr;    // Address of string data
    Cell length;  // Length in characters
} StringDesc;
```

## Testing Strategy

### Test Cases
```forth
\ Test TYPE with inline string
." Testing TYPE: " 72 emit 105 emit cr  \ Output: Testing TYPE: Hi

\ Test S" in interpret mode
S" Hello, World!" TYPE cr               \ Output: Hello, World!

\ Test S" in user-defined word
: greet S" Hello!" TYPE cr ;
greet                                   \ Output: Hello!

\ Test COUNT (requires creating counted string first)
CREATE cstring 5 c, 72 c, 101 c, 108 c, 108 c, 111 c,
cstring COUNT TYPE cr                   \ Output: Hello

\ Test nested strings
: test1 S" First " TYPE S" Second" TYPE cr ;
test1                                   \ Output: First Second

\ Test empty string
S" " TYPE cr                            \ Output: (empty line)
```

## Integration Points

### Modifications Required

**forth.h:**
- Add string buffer constants
- Add function prototypes for TYPE, COUNT, S"
- Add string_buffer_pos global

**forth.c:**
- Implement type_word() function
- Implement count_word() function
- Implement s_quote() function (immediate word)
- Update forth_init() to register new words
- Add string buffer initialization

**test.forth:**
- Add string operation test cases
- Verify all three operations work correctly

## Implementation Order

1. **Add global string buffer variables** to forth.c
2. **Implement TYPE** - simplest, no parsing needed
3. **Implement COUNT** - simple address manipulation
4. **Implement S" (interpret mode first)** - parser integration
5. **Enhance S" for compile mode** - threaded code compilation
6. **Update forth.h** with new prototypes
7. **Register words** in forth_init()
8. **Create test suite** in test.forth
9. **Test and debug** all three operations

## Expected Outcomes

After implementation:
- Users can create and display string literals with S"
- TYPE provides flexible string output (any address + length)
- COUNT enables working with counted strings from memory
- Foundation established for future string operations

## Future Extensions (Phase 3B+)

Once core essentials work, can add:
- **CMOVE** - Memory copy for strings
- **COMPARE** - String comparison
- **SEARCH** - Substring search
- **-TRAILING** - Remove trailing spaces
- **/STRING** - Adjust string address/length
- **BLANK**, **ERASE** - Fill memory operations
- **C"** - Create counted string
- **.(** - Immediate string output (like ." but during compilation)

## Notes

- S" must be marked as **immediate** (immediate=1)
- String buffer wraps when full (circular buffer behavior)
- Compiled strings are embedded in word's code array
- No dynamic memory allocation needed
- Follows ANS Forth string conventions (address + length)

## Mermaid Diagram - String Data Flow

```mermaid
graph TD
    A[User Input: S" Hello"] --> B{Mode?}
    B -->|Interpret| C[Parse String]
    B -->|Compile| D[Parse String]
    C --> E[Copy to Transient Buffer]
    D --> F[Compile String Inline]
    E --> G[Push Buffer Address]
    F --> H[Compile OP_LIT addr]
    G --> I[Push String Length]
    F --> J[Compile String Data]
    I --> K[Stack: addr u]
    J --> L[Compile OP_LIT length]
    L --> M[Code: ... addr data... length]
    K --> N[Ready for TYPE]
    M --> O[Execute Later: addr u]
    O --> N
```

## Compatibility

- Follows ANS Forth standard for string handling
- Address + length convention (not zero-terminated)
- Compatible with existing memory operations
- Can extend to full ANS string word set later