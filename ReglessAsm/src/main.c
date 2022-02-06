#include "main.h"

// Assembling

char *input_buffer;
long input_len;

char cur_char;
long index = 0;

FILE *output;

void error(char *message) {
    printf("%s ERROR! %s", PROJECT_NAME, message);
    free(message);

    getchar();
    exit(EXIT_FAILURE);
}

char peek() {
    if (index + 1 < input_len)
        return input_buffer[index + 1];
    else
        return '\0';
}

void get_next_char() {
    if (index < input_len)
        cur_char = input_buffer[index++];
    else
        cur_char = '\0';
}

void demand_char(char ch, char *error_msg) {
    if (cur_char != ch)
        error(error_msg);
    get_next_char();
}

// Location pointers

typedef struct location_pointers {
    char *id;
    uint32_t mem_loc;
} loc_ptr;

// Used for keeping track of the memory location for
// location pointers
uint32_t cur_mem_loc;
loc_ptr loc_ptrs[UINT16_MAX];
uint16_t loc_ptrs_len = 0;

loc_ptr loc_ptr_calls[UINT16_MAX];
uint16_t loc_ptr_calls_len = 0;

// Used for saving the intermediate version from
// the final output, so missing location pointers
// can be filled in.
char output_buffer[UINT16_MAX];

int loc_ptr_exists(char *id) {
    for (int i = 0; i < loc_ptrs_len; i++)
        if (str_equals(id, loc_ptrs[i].id))
            return 1;

    return 0;
}

uint32_t get_loc_ptr(char *id) {
    for (int i = 0; i < loc_ptrs_len; i++)
        if (str_equals(id, loc_ptrs[i].id))
            return loc_ptrs[i].mem_loc;

    return 0;
}

// Char recognition functions

int is_blank() {
    // 0XA = newline 0XD carriage return
    if (cur_char == 0xA || cur_char == 0xD)
        return 1;
    return 0;
}

int is_alphabet() {
    if (
        cur_char >= 'A' && cur_char <= 'Z' ||
        cur_char >= 'a' && cur_char <= 'z')
        return 1;
    return 0;
}

int is_number() {
    if (cur_char >= '0' && cur_char <= '9')
        return 1;
    return 0;
}

// Input reading functions

void skip_blanks() {
    while (is_blank())
        get_next_char();
}

char *build_word() {
    char *word = str_new("");

    while (is_alphabet() || is_number() || cur_char == '-' || cur_char == '_') {
        if (cur_char >= 'a' && cur_char <= 'z')
            cur_char -= 32;

        word = str_combine(word, &cur_char);
        get_next_char();
    }

    return word;
}

char *build_float() {
    char *whole = build_word();
    demand_char('.', ERROR_MSG_DECIMAL_POINT);
    whole = str_combine(whole, ".");
    char *decimal = build_word();
    whole = str_combine(whole, decimal);

    return whole;
}

// Writing functions

void write(unsigned char bytecode) {
    output_buffer[cur_mem_loc] = bytecode;
    cur_mem_loc++;
}

void write_8bit_hex(char sign) {
    demand_char(S_VALUE_FORMAT_SETTER, ERROR_MSG_VALUE_PREFIX);

    // String to number
    char *str = str_new(HEX);
    str = str_combine(str, build_word());
    uint8_t num = ((uint8_t)strtol(str, NULL, 0));

    // Writing
    if(sign)
        write(-num);
    else
        write(num);

    free(str);
}

void write_16bit_hex(char sign) {
    demand_char(S_VALUE_FORMAT_SETTER, ERROR_MSG_VALUE_PREFIX);

    // String to number
    char *str = str_new(HEX);
    str = str_combine(str, build_word());
    uint16_t num = ((uint16_t)strtol(str, NULL, 0));

    // Writing
    if(sign) {
        write(-num);
        write(-num >> 8);
    } else {
        write(num);
        write(num >> 8);
    }

    free(str);
}

void write_32bit_hex(char sign) {
    demand_char(S_VALUE_FORMAT_SETTER, ERROR_MSG_VALUE_PREFIX);

    // String to number
    char *str = str_new(HEX);
    str = str_combine(str, build_word());
    uint32_t num = ((uint32_t)strtol(str, NULL, 0));

    // Writing
    if(sign) {
        write(-num);
        write(-num >> 8);
        write(-num >> (8 + 8));
        write(-num >> (8 + 8 + 8));
        
    } else {
        write(num);
        write(num >> 8);
        write(num >> (8 + 8));
        write(num >> (8 + 8 + 8));
    }

    free(str);
}

void write_8bit_int(char sign) {
    demand_char(S_VALUE_FORMAT_SETTER, ERROR_MSG_VALUE_PREFIX);

    // Building string
    char *num = build_word();

    // Writing
    if(sign)
        write(-(char)atoi(num));
    else
        write((char)atoi(num));

    free(num);
}

void write_16bit_int(char sign) {
    demand_char(S_VALUE_FORMAT_SETTER, ERROR_MSG_VALUE_PREFIX);

    // String to number
    char *str = build_word();
    uint32_t num = ((uint16_t)atoi(str));

    // Writing
    if(sign) {
        write(-num);
        write(-num >> 8);
    } else {
        write(num);
        write(num >> 8);
    }

    free(str);
}

void write_32bit_int(char sign) {
    demand_char(S_VALUE_FORMAT_SETTER, ERROR_MSG_VALUE_PREFIX);

    // String to Number
    char *str = build_word();
    uint32_t num = ((uint32_t)atoi(str));

    // Writing
    if(sign) {
        write(-num);
        write(-num >> 8);
        write(-num >> (8 + 8));
        write(-num >> (8 + 8 + 8));
    } else {
        write(num);
        write(num >> 8);
        write(num >> (8 + 8));
        write(num >> (8 + 8 + 8));
    }
    free(str);
}

void write_32bit_float(char sign) {
    demand_char(S_VALUE_FORMAT_SETTER, ERROR_MSG_VALUE_PREFIX);

    // String to Number
    char *str = build_float();

    union split_float num;

    num.val = ((float)atof(str));

    // Writing
    if(sign) {
        unsigned char first = num.split[0];
        first |= 1 << 7;
        write(first);
        write(num.split[1]);
        write(num.split[2]);
        write(num.split[3]);
    } else {
        write(num.split[0]);
        write(num.split[1]);
        write(num.split[2]);
        write(num.split[3]);
    }

    free(str);
}

void export() {
    printf("Export:\n");
    for (int i = 0; i < cur_mem_loc; i++) {
        printf("%x ", (unsigned char)output_buffer[i]);
        fwrite(&output_buffer[i], sizeof(char), 1, output);
    }
    printf("\n");
}

// Assembling functions

void write_keyword(char *word) {
    // Op codes

    // General
    if      (str_equals(word, "SET"    )) write(0x0);
    else if (str_equals(word, "TRAP"   )) write(0x1);
    else if (str_equals(word, "PUSH"   )) write(0x2);
    else if (str_equals(word, "POP"    )) write(0x3);
    else if (str_equals(word, "JUMP"   )) write(0x4);
    else if (str_equals(word, "BRANCH" )) write(0x5);
    else if (str_equals(word, "ADD"    )) write(0x6);
    else if (str_equals(word, "SUB"    )) write(0x7);
    else if (str_equals(word, "MUL"    )) write(0x8);
    else if (str_equals(word, "DIV"    )) write(0x9);
    else if (str_equals(word, "REM"    )) write(0xA);
    else if (str_equals(word, "CAST"   )) write(0xB);
    else if (str_equals(word, "EQ"     )) write(0xC);
    else if (str_equals(word, "NOT"    )) write(0xD);
    else if (str_equals(word, "S"      )) write(0xE);
    else if (str_equals(word, "L"      )) write(0xF);
    else if (str_equals(word, "S_OR_EQ")) write(0x10);
    else if (str_equals(word, "L_OR_EQ")) write(0x11);

    // Sizes
    else if (str_equals(word, "S_SIZE")) write(0x0);
    else if (str_equals(word, "D_SIZE")) write(0x1);
    else if (str_equals(word, "Q_SIZE")) write(0x2);

    // Casts
    else if (str_equals(word, "S_UINT" )) write(0x0);
    else if (str_equals(word, "S_INT"  )) write(0x1);
    else if (str_equals(word, "D_UINT" )) write(0x2);
    else if (str_equals(word, "D_INT"  )) write(0x3);
    else if (str_equals(word, "Q_UINT" )) write(0x4);
    else if (str_equals(word, "Q_INT"  )) write(0x5);
    else if (str_equals(word, "Q_FLOAT")) write(0x6);

    // Reading mode
    else if (str_equals(word, "MEM")) write(0x0);
    else if (str_equals(word, "VAL")) write(0x1); 
    
    // Trap codes
    else if (str_equals(word, "END")) write(0x0);
    else if (str_equals(word, "OUT")) write(0x1);

    // Print formats
    else if (str_equals(word, "F_HEX"  )) write(0x0);
    else if (str_equals(word, "F_INT"  )) write(0x1);
    else if (str_equals(word, "F_FLOAT")) write(0x2);
    else if (str_equals(word, "F_CHAR" )) write(0x3);

    // Value formats

    // Hex
    else if (str_equals(word, "SX" )) write_8bit_hex(UNSIGNED);
    else if (str_equals(word, "-SX")) write_8bit_hex(SIGNED);

    else if (str_equals(word, "DX" )) write_16bit_hex(UNSIGNED);
    else if (str_equals(word, "-DX")) write_16bit_hex(SIGNED);

    else if (str_equals(word, "QX" )) write_32bit_hex(UNSIGNED);
    else if (str_equals(word, "-QX")) write_32bit_hex(SIGNED);

    // Int
    else if (str_equals(word, "SI" )) write_8bit_int(UNSIGNED);
    else if (str_equals(word, "-SI")) write_8bit_int(SIGNED);

    else if (str_equals(word, "DI" )) write_16bit_int(UNSIGNED);
    else if (str_equals(word, "-DI")) write_16bit_int(SIGNED);

    else if (str_equals(word, "QI" )) write_32bit_int(UNSIGNED);
    else if (str_equals(word, "-QI")) write_32bit_int(SIGNED);

    // Float
    else if (str_equals(word, "QF" )) write_32bit_float(UNSIGNED);
    else if (str_equals(word, "-QF")) write_32bit_float(SIGNED);

    // Keyword is unsupported
    else { 
        char *err_msg = str_new("");

        err_msg = str_combine(err_msg, ERROR_MSG_UNSUPPORTED_KEYWORD);
        err_msg = str_combine(err_msg, word);

        error(err_msg);
    }
}

int main(int argc, char *argv[])
{
    // Startup
    printf("%s Assembler %s\nFile: %s\n", PROJECT_NAME, PROJECT_VERSION, argv[1]);

    if (argc < 2) {
        printf("No input file. Exiting...");
        return EXIT_SUCCESS;
    }

    // Reading input file
    FILE *input_file = fopen(argv[1], "rb");

    // Counting file length
    fseek(input_file, 0x0, SEEK_END);
    input_len = ftell(input_file);
    fseek(input_file, 0x0, SEEK_SET);

    // Creating input buffer with input file length size
    // and add contents from input file to it
    input_buffer = malloc(sizeof(char) * input_len + 1);

    fread(input_buffer, sizeof(char) * input_len, 1, input_file);
    fclose(input_file);

    input_buffer[input_len] = '\0';

    printf("Program size: %d bytes\n", input_len);

    // Creating name for the output
    char *output_file_name = str_new("");

    output_file_name = str_combine(output_file_name, argv[1]);
    output_file_name = str_combine(output_file_name, FILE_FORMAT_NAME);

    output_file_name[str_length(output_file_name)] = '\0';

    printf("Exporting to: %s\n", output_file_name);

    // Assembling
    get_next_char();
    char *cur_word;

    while (cur_char != '\0') {
        skip_blanks();

        // Stated
        switch (cur_char) {
        case S_LOCATION_POINTER_CALL:
            get_next_char();

            char *id = build_word();

            if (loc_ptr_exists(id)) {
                uint32_t loc = get_loc_ptr(id);
                
                write(loc);
                write(loc >> 8);
                write(loc >> (8 + 8));
                write(loc >> (8 + 8 + 8));

                printf("Non buffered location call %s: %i\n", id, loc);
            } else {
                loc_ptr_calls[loc_ptr_calls_len].id = id;
                loc_ptr_calls[loc_ptr_calls_len].mem_loc = cur_mem_loc;
                loc_ptr_calls_len++;

                write(0x0);
                write(0x0);
                write(0x0);
                write(0x0);
            }
            break;

        case S_LOCATION_POINTER:
            get_next_char();

            loc_ptrs[loc_ptrs_len].id = build_word();
            loc_ptrs[loc_ptrs_len].mem_loc = cur_mem_loc;

            loc_ptrs_len++;
            break;

        case S_COMMENT:
            while (cur_char != '\n' && cur_char != '\0')
                get_next_char();
            break;

        case '\"':           // String
            get_next_char(); // for the starting '"'
            while (cur_char != '\0' && cur_char != '"' && cur_char != '\n') {
                write(cur_char);
                get_next_char();
            }
            get_next_char(); // for the trailing '"'
            break;
        }

        // Keywords
        if (is_alphabet() || cur_char == '-') {
            cur_word = build_word();
            write_keyword(cur_word);
            free(cur_word);
        }

        get_next_char();
    }

    // Filling missing location pointers
    printf("Filling in possible missing location pointers...\n");
    for (int i = 0; i < loc_ptr_calls_len; i++) {
        if (loc_ptr_exists(loc_ptr_calls[i].id)) {
            uint32_t loc = get_loc_ptr(loc_ptr_calls[i].id);

            output_buffer[loc_ptr_calls[i].mem_loc + 3] = loc;
            output_buffer[loc_ptr_calls[i].mem_loc + 2] = loc >> 8;
            output_buffer[loc_ptr_calls[i].mem_loc + 1] = loc >> (8 + 8);
            output_buffer[loc_ptr_calls[i].mem_loc]     = loc >> (8 + 8 + 8);

            printf("Location call %s: %x\n", loc_ptr_calls[i].id, loc);
        } else {
            char *errmsg = str_new("There isn't memory location specified for \"");
            errmsg = str_combine(errmsg, loc_ptr_calls[i].id);
            errmsg = str_combine(errmsg, "\"");
            error(errmsg);
        }
    }

    // Exporting

    output = fopen(output_file_name, "wb+");
    export();
    fclose(output);

    free(input_buffer);

    // Printing all location pointers
    for (uint32_t i = 0; i < loc_ptrs_len; i++) {
        printf("Location %s: %x\n", loc_ptrs[i].id, loc_ptrs[i].mem_loc);
        free(loc_ptrs[i].id);
    }

    getchar();
}
