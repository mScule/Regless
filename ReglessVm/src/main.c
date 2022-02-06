#include "main.h"

// Memory
uint_fast32_t mem_size = 0;
uint8_t *memory;
uint_fast32_t pc = 0;

// Stack
uint_fast32_t stack[STACK_SIZE];
uint16_t stack_depth = 0;

// Runtime
uint8_t runtime = 1, flag = 0, exitcode = 0;

void exit_runtime(uint8_t code) {
    runtime = 0;
    exitcode = code;
}

uint8_t get_value_8bit() {
    if (pc < mem_size)
        return memory[pc++];
    else
        exit_runtime(ERROR_CODE_END_OF_MEMORY_GET);
    return 0;
}

uint16_t get_value_16bit() {
    uint16_t value = get_value_8bit() << 8;
    value += get_value_8bit();
    return value;
}

uint32_t get_value_32bit() {
    uint32_t value = get_value_16bit() << 16;
    value += get_value_16bit();
    return value;
}

uint8_t fetch_value_8bit(uint32_t mem_addr) {
    if (mem_addr < mem_size) {
        return memory[mem_addr];
    } else {
        exit_runtime(ERROR_CODE_END_OF_MEMORY_FETCH);
    }
    return 0;
}

uint16_t fetch_value_16bit(uint32_t mem_addr) {
    uint16_t output = 0;
    
    output += fetch_value_8bit(mem_addr);
    output += fetch_value_8bit(mem_addr + 1) << 8;

    return output;
}

union val32 fetch_value_32bit(uint32_t mem_addr) {
    union val32 output;
    output.as_int = 0;

    output.as_int += fetch_value_16bit(mem_addr);
    output.as_int += fetch_value_16bit(mem_addr + 2) << 16;

    return output;
}

union val32 fetch_value(uint8_t size, uint32_t mem_addr) {
    union val32 val;

    val.as_int = 0;

    switch(size) {
        case SIZE_8BIT:  val.as_int = fetch_value_8bit(mem_addr);  break;
        case SIZE_16BIT: val.as_int = fetch_value_16bit(mem_addr); break;
        case SIZE_32BIT: val = fetch_value_32bit(mem_addr); break;

        default: exit_runtime(ERROR_CODE_BAD_SIZE);
    }

    return val;
}

void set_value_8bit(uint32_t mem_addr, uint8_t val) {
    if(mem_addr < mem_size) {
        memory[mem_addr] = val;
    } else {
        exit_runtime(ERROR_CODE_END_OF_MEMORY_SET);
    }
}

void set_value_16bit(uint32_t mem_addr, uint16_t val) {
    set_value_8bit(mem_addr, val);
    set_value_8bit(mem_addr + 1, val >> 8);
}

void set_value_32bit(uint32_t mem_addr, uint32_t val) {
    set_value_16bit(mem_addr, val);
    set_value_16bit(mem_addr + 2, val >> 16);
}

void write_value_mem(uint32_t mem_addr) {
    switch(get_value_8bit()) {
        case SIZE_8BIT:
            set_value_8bit(mem_addr,  fetch_value_8bit(get_value_32bit()));
        break;
        case SIZE_16BIT:
            set_value_16bit(mem_addr, fetch_value_16bit(get_value_32bit()));
        break;
        case SIZE_32BIT:
            set_value_32bit(mem_addr, fetch_value_32bit(get_value_32bit()).as_int);
        break;

        default:
            exit_runtime(ERROR_CODE_BAD_SIZE);
        break;
    }
}

void write_value_const(uint32_t mem_addr) {
    switch(get_value_8bit()) {
        case SIZE_8BIT:
            set_value_8bit(mem_addr, get_value_8bit());
        break;
        case SIZE_16BIT:
            set_value_16bit(mem_addr, get_value_16bit());
        break;
        case SIZE_32BIT:
            set_value_32bit(mem_addr, get_value_32bit());
        break;

        default:
            exit_runtime(ERROR_CODE_BAD_SIZE);
        break;
    }
}

// Instruction set

// General
void i_set() {
    uint32_t mem_addr = get_value_32bit();
    
    switch(get_value_8bit()) {
        case VALUE_MEM:
            write_value_mem(mem_addr);
        break;
        // set 
        case VALUE_CONST:
            write_value_const(mem_addr);
        break;
    }
}

void i_trap() {
    switch(get_value_8bit()) {
        case TRAP_END:
            exit_runtime(VM_EXIT_SUCCESS);
        break;

        // trap out format mem_addr cast
        case TRAP_OUT: {
            uint8_t format    = get_value_8bit();
            uint32_t mem_addr = get_value_32bit();
            uint8_t type      = get_value_8bit();
            union val32 output;

            switch(type) {
                case TYPE_UINT8:
                    output.as_int = fetch_value_8bit(mem_addr);
                    out(format, TYPE_UINT8, output);
                break;
                case TYPE_INT8:
                    output.as_int = fetch_value_8bit(mem_addr);
                    out(format, TYPE_INT8, output);
                break;
                case TYPE_UINT16:
                    output.as_int = fetch_value_16bit(mem_addr);
                    out(format, TYPE_UINT16, output);
                break;
                case TYPE_INT16:
                    output.as_int = fetch_value_16bit(mem_addr);
                    out(format, TYPE_INT16, output);
                break;
                case TYPE_UINT32:
                    out(format, TYPE_UINT32, fetch_value_32bit(mem_addr));
                break;
                case TYPE_INT32:
                    out(format, TYPE_INT32, fetch_value_32bit(mem_addr));
                break;
                case TYPE_FLOAT32:
                    out(format, TYPE_FLOAT32, fetch_value_32bit(mem_addr));
                break;

                default: exit_runtime(ERROR_CODE_BAD_CAST_SIZE); break;
            }
            break;
        }
    }
}

// Stack
void i_push() {
    if(stack_depth < STACK_SIZE) {
        stack[stack_depth++] = get_value_32bit();
    } else {
        exit_runtime(ERROR_CODE_STACK_OVERFLOW);
    }
}
void i_pop() {
    if(stack_depth > 0) {
        set_value_32bit(get_value_32bit(), stack[stack_depth--]);
    } else {
        set_value_32bit(get_value_32bit(), stack[0]);
    }
}

// Jumping
void i_jump() {
    pc = fetch_value(SIZE_32BIT, get_value_32bit()).as_int;
}
void i_branch() {
    if(flag == 1) i_jump();
    else get_value_32bit();
}

// Math

// opr type a_mem_addr b_mem_addr
void calc(uint8_t opr) {
    uint8_t  cast       = get_value_8bit();

    uint32_t a_mem_addr = get_value_32bit();
    uint32_t b_mem_addr = get_value_32bit();

    union val32 output, a_val, b_val;

    switch(cast) {

        // 8 bit integers
        case TYPE_UINT8:
            a_val = fetch_value(SIZE_8BIT, a_mem_addr);
            b_val = fetch_value(SIZE_8BIT, b_mem_addr);
            
            switch(opr) {
                case ADD: a_val.as_int += (uint8_t) b_val.as_int; break;
                case SUB: a_val.as_int -= (uint8_t) b_val.as_int; break;
                case MUL: a_val.as_int *= (uint8_t) b_val.as_int; break;
                case DIV: a_val.as_int /= (uint8_t) b_val.as_int; break;
                case REM: a_val.as_int %= (uint8_t) b_val.as_int; break;
            }

            set_value_8bit(a_mem_addr, a_val.as_int);
        break;
        case TYPE_INT8:
            a_val = fetch_value(SIZE_8BIT, a_mem_addr);
            b_val = fetch_value(SIZE_8BIT, b_mem_addr);

            switch(opr) {
                case ADD: a_val.as_int += (int8_t) b_val.as_int; break;
                case SUB: a_val.as_int -= (int8_t) b_val.as_int; break;
                case MUL: a_val.as_int *= (int8_t) b_val.as_int; break;
                case DIV: a_val.as_int /= (int8_t) b_val.as_int; break;
                case REM: a_val.as_int %= (int8_t) b_val.as_int; break;
            }
            set_value_8bit(a_mem_addr, a_val.as_int);
        break;

        // 16 bit integers
        case TYPE_UINT16:
            a_val = fetch_value(SIZE_16BIT, a_mem_addr);
            b_val = fetch_value(SIZE_16BIT, b_mem_addr);

            switch(opr) {
                case ADD: a_val.as_int += (uint16_t) b_val.as_int; break;
                case SUB: a_val.as_int -= (uint16_t) b_val.as_int; break;
                case MUL: a_val.as_int *= (uint16_t) b_val.as_int; break;
                case DIV: a_val.as_int /= (uint16_t) b_val.as_int; break;
                case REM: a_val.as_int %= (uint16_t) b_val.as_int; break;
            }
            set_value_16bit(a_mem_addr, a_val.as_int);
        break;
        case TYPE_INT16:
            a_val = fetch_value(SIZE_16BIT, a_mem_addr);
            b_val = fetch_value(SIZE_16BIT, b_mem_addr);

            switch(opr) {
                case ADD: a_val.as_int += (int16_t) b_val.as_int; break;
                case SUB: a_val.as_int -= (int16_t) b_val.as_int; break;
                case MUL: a_val.as_int *= (int16_t) b_val.as_int; break;
                case DIV: a_val.as_int /= (int16_t) b_val.as_int; break;
                case REM: a_val.as_int %= (int16_t) b_val.as_int; break;
            }
            set_value_16bit(a_mem_addr, a_val.as_int);
        break;

        // 32 bit integers
        case TYPE_UINT32:
            a_val = fetch_value(SIZE_32BIT, a_mem_addr);
            b_val = fetch_value(SIZE_32BIT, b_mem_addr);

            switch(opr) {
                case ADD: a_val.as_int += (uint32_t) b_val.as_int; break;
                case SUB: a_val.as_int -= (uint32_t) b_val.as_int; break;
                case MUL: a_val.as_int *= (uint32_t) b_val.as_int; break;
                case DIV: a_val.as_int /= (uint32_t) b_val.as_int; break;
                case REM: a_val.as_int %= (uint32_t) b_val.as_int; break;
            }
            set_value_32bit(a_mem_addr, a_val.as_int);
        break;
        case TYPE_INT32:
            a_val = fetch_value(SIZE_32BIT, a_mem_addr);
            b_val = fetch_value(SIZE_32BIT, b_mem_addr);

            switch(opr) {
                case ADD: a_val.as_int += (int32_t) b_val.as_int; break;
                case SUB: a_val.as_int -= (int32_t) b_val.as_int; break;
                case MUL: a_val.as_int *= (int32_t) b_val.as_int; break;
                case DIV: a_val.as_int /= (int32_t) b_val.as_int; break;
                case REM: a_val.as_int %= (int32_t) b_val.as_int; break;
            }
            set_value_32bit(a_mem_addr, a_val.as_int);
        break;

        // Floats
        case TYPE_FLOAT32:
            a_val = fetch_value(SIZE_32BIT, a_mem_addr);
            b_val = fetch_value(SIZE_32BIT, b_mem_addr);

            switch(opr) {
                case ADD: a_val.as_float += b_val.as_float; break;
                case SUB: a_val.as_float -= b_val.as_float; break;
                case MUL: a_val.as_float *= b_val.as_float; break;
                case DIV: a_val.as_float /= b_val.as_float; break;
                case REM: exit_runtime(ERROR_CODE_INTEGRAL_TYPE); break;
            }
            set_value_32bit(a_mem_addr, a_val.as_int);
        break;
    }
}

// cast type mem_addr type
void cast() {
    uint8_t  cast_from = get_value_8bit();
    uint32_t mem_addr  = get_value_32bit();
    uint8_t  cast_to   = get_value_8bit();

    switch(cast_from) {
        case TYPE_UINT8: {
            uint8_t val = (uint8_t)fetch_value_8bit(mem_addr);

            switch(cast_to) {
                case TYPE_UINT8: val = (uint8_t) val; break;
                case TYPE_INT8:  val = (int8_t)  val; break;
                default: exit_runtime(ERROR_CODE_SMALL_CAST_SIZE); break;
            }

            set_value_8bit(mem_addr, val);
            break;
        }
        case TYPE_INT8: {
            int8_t val = (int8_t)fetch_value_8bit(mem_addr);

            switch(cast_to) {
                case TYPE_UINT8: val = (uint8_t) val; break;
                case TYPE_INT8:  val = (int8_t)  val; break;
                default: exit_runtime(ERROR_CODE_SMALL_CAST_SIZE); break;
            }

            set_value_8bit(mem_addr, val);
            break;
        }
        case TYPE_UINT16: {
            uint16_t val = (uint16_t)fetch_value_16bit(mem_addr);

            switch(cast_to) {
                case TYPE_UINT8:   val = (uint8_t)  val; break;
                case TYPE_INT8:    val = (int8_t)   val; break;

                case TYPE_UINT16:  val = (uint16_t) val; break;
                case TYPE_INT16:   val = (int16_t)  val; break;

                default: exit_runtime(ERROR_CODE_SMALL_CAST_SIZE); break;
            }

            set_value_16bit(mem_addr, val);
            break;
        }
        case TYPE_INT16: {
            int16_t val = (int16_t)fetch_value_16bit(mem_addr);

            switch(cast_to) {
                case TYPE_UINT8:   val = (uint8_t)  val; break;
                case TYPE_INT8:    val = (int8_t)   val; break;

                case TYPE_UINT16:  val = (uint16_t) val; break;
                case TYPE_INT16:   val = (int16_t)  val; break;

                default: exit_runtime(ERROR_CODE_SMALL_CAST_SIZE); break;
            }

            set_value_16bit(mem_addr, val);
            break;
        }
        case TYPE_UINT32: {
            uint32_t val = (uint32_t)fetch_value_32bit(mem_addr).as_int;

            switch(cast_to) {
                case TYPE_UINT8:   val = (uint8_t)  val; break;
                case TYPE_INT8:    val = (int8_t)   val; break;

                case TYPE_UINT16:  val = (uint16_t) val; break;
                case TYPE_INT16:   val = (int16_t)  val; break;

                case TYPE_UINT32:  val = (uint32_t) val; break;
                case TYPE_INT32:   val = (int32_t)  val; break;

                case TYPE_FLOAT32: val = (float)    val; break;
            }

            set_value_32bit(mem_addr, val);
            break;
        }
        case TYPE_INT32: {
            int32_t val = (int32_t)fetch_value_32bit(mem_addr).as_int;

            switch(cast_to) {
                case TYPE_UINT8:   val = (uint8_t)  val; break;
                case TYPE_INT8:    val = (int8_t)   val; break;

                case TYPE_UINT16:  val = (uint16_t) val; break;
                case TYPE_INT16:   val = (int16_t)  val; break;

                case TYPE_UINT32:  val = (uint32_t) val; break;
                case TYPE_INT32:   val = (int32_t)  val; break;

                case TYPE_FLOAT32: val = (float)    val; break;
            }

            set_value_32bit(mem_addr, val);
            break;
        }
        case TYPE_FLOAT32: {
            float val = (float)fetch_value_32bit(mem_addr).as_float;

            switch(cast_to) {
                case TYPE_UINT8:   val = (uint8_t)  val; break;
                case TYPE_INT8:    val = (int8_t)   val; break;

                case TYPE_UINT16:  val = (uint16_t) val; break;
                case TYPE_INT16:   val = (int16_t)  val; break;

                case TYPE_UINT32:  val = (uint32_t) val; break;
                case TYPE_INT32:   val = (int32_t)  val; break;

                case TYPE_FLOAT32: val = (float)    val; break;
            }

            set_value_32bit(mem_addr, val);
            break;
        }
    }
}

// Comparison

// opr type mem_addr mem_addr
void comparison(uint8_t opr) {
    uint8_t cast = get_value_8bit();

    uint32_t a_mem_addr = get_value_32bit();
    uint32_t b_mem_addr = get_value_32bit();

    switch(cast) {
        // 8 bit integers
        case TYPE_UINT8: {
            uint8_t
                a = (uint8_t)fetch_value_8bit(a_mem_addr),
                b = (uint8_t)fetch_value_8bit(a_mem_addr);

                switch(opr) {
                    case EQ:      flag = a == b ? 1 : 0; break;
                    case NOT:     flag = a != b ? 1 : 0; break;
                    case S:       flag = a < b  ? 1 : 0; break;
                    case L:       flag = a > b  ? 1 : 0; break;
                    case S_OR_EQ: flag = a <= b ? 1 : 0; break;
                    case L_OR_EQ: flag = a >= b ? 1 : 0; break;
                }
            break;
        }
        case TYPE_INT8: {
            int8_t
                a = (int8_t)fetch_value_8bit(a_mem_addr),
                b = (int8_t)fetch_value_8bit(a_mem_addr);

                switch(opr) {
                    case EQ:      flag = a == b ? 1 : 0; break;
                    case NOT:     flag = a != b ? 1 : 0; break;
                    case S:       flag = a < b  ? 1 : 0; break;
                    case L:       flag = a > b  ? 1 : 0; break;
                    case S_OR_EQ: flag = a <= b ? 1 : 0; break;
                    case L_OR_EQ: flag = a >= b ? 1 : 0; break;
                }
            break;
        }

        // 16 bit integers
        case TYPE_UINT16: {
            uint16_t
                a = (uint16_t)fetch_value_16bit(a_mem_addr),
                b = (uint16_t)fetch_value_16bit(a_mem_addr);

                switch(opr) {
                    case EQ:      flag = a == b ? 1 : 0; break;
                    case NOT:     flag = a != b ? 1 : 0; break;
                    case S:       flag = a < b  ? 1 : 0; break;
                    case L:       flag = a > b  ? 1 : 0; break;
                    case S_OR_EQ: flag = a <= b ? 1 : 0; break;
                    case L_OR_EQ: flag = a >= b ? 1 : 0; break;
                }
            break;
        }
        case TYPE_INT16: {
            int16_t
                a = (int16_t)fetch_value_16bit(a_mem_addr),
                b = (int16_t)fetch_value_16bit(a_mem_addr);

                switch(opr) {
                    case EQ:      flag = a == b ? 1 : 0; break;
                    case NOT:     flag = a != b ? 1 : 0; break;
                    case S:       flag = a < b  ? 1 : 0; break;
                    case L:       flag = a > b  ? 1 : 0; break;
                    case S_OR_EQ: flag = a <= b ? 1 : 0; break;
                    case L_OR_EQ: flag = a >= b ? 1 : 0; break;
                }
            break;
        }

        // 32 bit integers
        case TYPE_UINT32: {
            uint32_t
                a = (uint32_t)fetch_value_32bit(a_mem_addr).as_int,
                b = (uint32_t)fetch_value_32bit(a_mem_addr).as_int;

                switch(opr) {
                    case EQ:      flag = a == b ? 1 : 0; break;
                    case NOT:     flag = a != b ? 1 : 0; break;
                    case S:       flag = a < b  ? 1 : 0; break;
                    case L:       flag = a > b  ? 1 : 0; break;
                    case S_OR_EQ: flag = a <= b ? 1 : 0; break;
                    case L_OR_EQ: flag = a >= b ? 1 : 0; break;
                }
            break;
        }
        case TYPE_INT32: {
            int32_t
                a = (int32_t)fetch_value_32bit(a_mem_addr).as_int,
                b = (int32_t)fetch_value_32bit(a_mem_addr).as_int;

                switch(opr) {
                    case EQ:      flag = a == b ? 1 : 0; break;
                    case NOT:     flag = a != b ? 1 : 0; break;
                    case S:       flag = a < b  ? 1 : 0; break;
                    case L:       flag = a > b  ? 1 : 0; break;
                    case S_OR_EQ: flag = a <= b ? 1 : 0; break;
                    case L_OR_EQ: flag = a >= b ? 1 : 0; break;
                }
            break;
        }

        // Floats
        case TYPE_FLOAT32: {
            float
                a = fetch_value_32bit(a_mem_addr).as_float,
                b = fetch_value_32bit(a_mem_addr).as_float;

                switch(opr) {
                    case EQ:      flag = a == b ? 1 : 0; break;
                    case NOT:     flag = a != b ? 1 : 0; break;
                    case S:       flag = a < b  ? 1 : 0; break;
                    case L:       flag = a > b  ? 1 : 0; break;
                    case S_OR_EQ: flag = a <= b ? 1 : 0; break;
                    case L_OR_EQ: flag = a >= b ? 1 : 0; break;
                }
            break;
        }
    }
}

void run(uint8_t op_code) {
    switch(op_code) {
        // General
        case SET:     return i_set();
        case TRAP:    return i_trap();
        // Stack
        case PUSH:    return i_push();
        case POP:     return i_pop();
        // Jumping
        case JUMP:    return i_jump();
        case BRANCH:  return i_branch();
        // Math 
        case ADD:     return calc(ADD);
        case SUB:     return calc(SUB);
        case MUL:     return calc(MUL);
        case DIV:     return calc(DIV);
        case REM:     return calc(REM);
        case CAST:    return cast();
        // Comparison
        case EQ:      return comparison(EQ);
        case NOT:     return comparison(NOT);
        case S:       return comparison(S);
        case L:       return comparison(L);
        case S_OR_EQ: return comparison(S_OR_EQ);
        case L_OR_EQ: return comparison(L_OR_EQ);

        default:
            runtime  = 0;
            exitcode = ERROR_CODE_BAD_OPCODE;
        break;
    }
}

void open_file(const char *path) {
    FILE *file = fopen(path, "rb");

    // Counting file size
    fseek(file, 0x0, SEEK_END);
    mem_size = ftell(file);
    fseek(file, 0x0, SEEK_SET);

    // Memory allocation
    memory = malloc(sizeof(uint8_t) * mem_size);

    // Reading bytecode to memory
    fread(memory, sizeof(char) * mem_size, 1, file);
    fclose(file);

    printf("Program size: %d bytes\n", mem_size);
}

int main(int argc, const char *argv[]) {
    // Startup
    printf("%s %s\nFile: %s\n", PROJECT_NAME, PROJECT_VERSION, argv[1]);

    if (argc < 2) {
        printf("No input file. Exiting...");
        return EXIT_SUCCESS;
    }

    open_file(argv[1]);

    // Runtime
    while(runtime)
        run(get_value_8bit());

    // Shutdown
    if (exitcode != VM_EXIT_SUCCESS)
        printf("\nERROR: %s pc: %x", error_msg(exitcode), pc);
    else
        printf("\nVM exit success");

    free(memory);
    getchar();
    return EXIT_SUCCESS;
}
