#include "error.h"

char *error_msg(uint8_t error_code) {

    switch(error_code) {
        // VM
        case ERROR_CODE_BAD_OPCODE:     return ERROR_MSG_BAD_OPCODE;
        case ERROR_CODE_BAD_SIZE:       return ERROR_MSG_BAD_SIZE;
        case ERROR_CODE_STACK_OVERFLOW: return ERROR_MSG_STACK_OVERFLOW;
        case ERROR_CODE_BAD_CAST_SIZE:  return ERROR_MSG_BAD_CAST_SIZE;

        // VM - End of memory
        case ERROR_CODE_END_OF_MEMORY_SET:   return ERROR_MSG_END_OF_MEMORY_SET;
        case ERROR_CODE_END_OF_MEMORY_GET:   return ERROR_MSG_END_OF_MEMORY_GET;
        case ERROR_CODE_END_OF_MEMORY_FETCH: return ERROR_MSG_END_OF_MEMORY_FETCH;

        // Traps
        case ERROR_CODE_UNSUPPORTED_OUTPUT_FORMAT: return ERROR_MSG_UNSUPPORTED_OUTPUT_FORMAT;

        // Calculation
        case ERROR_CODE_INTEGRAL_TYPE: return ERROR_MSG_INTEGRAL_TYPE;

        // Casting
        case ERROR_CODE_SMALL_CAST_SIZE: return ERROR_MSG_SMALL_CAST_SIZE;

        // General
        default:
            return ERROR_MSG_UNKNOWN; 
    }
}
