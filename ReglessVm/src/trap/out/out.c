#include "out.h"

uint8_t out(uint8_t format, uint8_t type, union val32 val) {
    char* out;

    union val32 cast;

    cast.as_int = 0;

    switch(format) {
        case T_OUT_FORMAT_HEX:   out = T_OUT_STR_HEX;   break;
        case T_OUT_FORMAT_INT:   out = T_OUT_STR_INT;   break;
        case T_OUT_FORMAT_FLOAT: out = T_OUT_STR_FLOAT; break;
        case T_OUT_FORMAT_CHAR:  out = T_OUT_STR_CHAR;  break;

        default: return T_OUT_FAILURE;
    }

    switch(type) {
        case TYPE_UINT8:   cast.as_int = (uint8_t)  val.as_int; break;
        case TYPE_INT8:    cast.as_int = (int8_t)   val.as_int; break;
        case TYPE_UINT16:  cast.as_int = (uint16_t) val.as_int; break;
        case TYPE_INT16:   cast.as_int = (int16_t)  val.as_int; break;
        case TYPE_UINT32:  cast.as_int = (uint32_t) val.as_int; break;
        case TYPE_INT32:   cast.as_int = (int32_t)  val.as_int; break;
    }

    switch(format) {
        case T_OUT_FORMAT_FLOAT:
            printf(out, val.as_float);
            break;
        default:
            printf(out, cast.as_int);
            break;
    }

    return T_OUT_SUCCESS;
}
