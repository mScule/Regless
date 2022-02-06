#include <stdint.h>

#ifndef H_VAL32
#define H_VAL32
union val32 {
    uint32_t as_int;
    float as_float;
};
#endif
