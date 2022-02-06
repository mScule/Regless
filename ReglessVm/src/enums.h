#ifndef H_ENUMS
#define H_ENUMS

// OP codes
enum {
    // General
    SET     = 0x0,
    TRAP    = 0x1,
    // Stack
    PUSH    = 0x2,
    POP     = 0x3,
    // Jumping 
    JUMP    = 0x4,
    BRANCH  = 0x5,
    // Math
    ADD     = 0x6, 
    SUB     = 0x7,
    MUL     = 0x8,
    DIV     = 0x9,
    REM     = 0xA,
    CAST    = 0xB,
    // Comparison
    EQ      = 0xC,
    NOT     = 0xD,
    S       = 0xE,
    L       = 0xF,
    S_OR_EQ = 0x10,
    L_OR_EQ = 0x11,
};

// Size modes
enum {
    SIZE_8BIT  = 0x0,
    SIZE_16BIT = 0x1,
    SIZE_32BIT = 0x2
};

// Reading mode
enum {
    TYPE_UINT8   = 0x0, TYPE_INT8  = 0x1,
    TYPE_UINT16  = 0x2, TYPE_INT16 = 0x3,
    TYPE_UINT32  = 0x4, TYPE_INT32 = 0x5,
    TYPE_FLOAT32 = 0x6
};

// Value reading mode
enum { VALUE_MEM = 0x0 , VALUE_CONST = 0x1 };

// Trap codes
enum { TRAP_END = 0x0, TRAP_OUT = 0x1 };
#endif

#define ENUMS
