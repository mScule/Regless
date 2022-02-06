#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

// VM
#define ERROR_MSG_BAD_OPCODE     "Bad OP code"
#define ERROR_MSG_BAD_SIZE       "Bad size"
#define ERROR_MSG_STACK_OVERFLOW "Stack overflow"
#define ERROR_MSG_BAD_CAST_SIZE  "Bad cast size"

#define ERROR_MSG_END_OF_MEMORY_SET   "End of memory (set)"
#define ERROR_MSG_END_OF_MEMORY_GET   "End of memory (get)"
#define ERROR_MSG_END_OF_MEMORY_FETCH "End of memory (fetch)"

// Traps
#define ERROR_MSG_UNSUPPORTED_OUTPUT_FORMAT "Unsupported output format"

// General
#define ERROR_MSG_UNKNOWN "Unknown error"

// Calculation
#define ERROR_MSG_INTEGRAL_TYPE "The value has to be integral type"

// Casting
#define ERROR_MSG_SMALL_CAST_SIZE "Cannot cast to bigger size"

enum {
    // VM
    ERROR_CODE_BAD_OPCODE = 1,
    ERROR_CODE_BAD_SIZE,
    ERROR_CODE_STACK_OVERFLOW,
    ERROR_CODE_BAD_CAST_SIZE,

    // VM - End of memory
    ERROR_CODE_END_OF_MEMORY_SET,
    ERROR_CODE_END_OF_MEMORY_GET,
    ERROR_CODE_END_OF_MEMORY_FETCH,

    // Traps
    ERROR_CODE_UNSUPPORTED_OUTPUT_FORMAT,

    // Calculation
    ERROR_CODE_INTEGRAL_TYPE,

    // Casting
    ERROR_CODE_SMALL_CAST_SIZE
};

char *error_msg(uint8_t error_code); 
