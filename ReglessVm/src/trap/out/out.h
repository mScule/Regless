#include <stdio.h>
#include <stdint.h>

#include "../../val32.h"
#include "../../enums.h"

#define T_OUT_STR_HEX   "%x"
#define T_OUT_STR_INT   "%d"
#define T_OUT_STR_FLOAT "%f"
#define T_OUT_STR_CHAR  "%c"

enum {
    T_OUT_FORMAT_HEX   = 0x0,
    T_OUT_FORMAT_INT   = 0x1,
    T_OUT_FORMAT_FLOAT = 0x2,
    T_OUT_FORMAT_CHAR  = 0x3
};

enum {
    T_OUT_SUCCESS,
    T_OUT_FAILURE
};

uint8_t out(uint8_t format, uint8_t type, union val32 val);
