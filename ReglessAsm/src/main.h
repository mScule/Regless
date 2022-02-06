#include <stdio.h>  // printf(), fopen(), fseek(), fread(), ftell(), fclose(), FILE
#include <stdlib.h> // malloc(), free(), atoi()
#include <stdint.h> // uint16_t

#include "str\str.h"

// General

#define PROJECT_NAME "REGLESS"
#define PROJECT_VERSION "0.1.0"

#define FILE_FORMAT_NAME ".rbin"
#define HEX "0x"

// Error messages

#define ERROR_MSG_VALUE_PREFIX "you need to give ':' before the value"
#define ERROR_MSG_UNSUPPORTED_KEYWORD "Unsupported keyword "
#define ERROR_MSG_DECIMAL_POINT "you need to include the '.' before the decimal value"

// Syntax

#define S_VALUE_FORMAT_SETTER ':'
#define S_COMMENT '#'

#define S_LOCATION_POINTER '@'
#define S_LOCATION_POINTER_CALL ':'

#define S_CHAR '\''
#define S_STRING '"'

#define S_ESCAPER '\\'
#define S_NEWLINE 'n'
#define S_TAB 't'
#define S_NULLPOINT_TERMINATOR '\0'

#define UNSIGNED 0
#define SIGNED   1

union split_float {
    float val;
    unsigned char split[4];
};
