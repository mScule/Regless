#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "val32.h"
#include "enums.h"

// General project includes
#include "error/error.h"

// Trap includes
#include "trap/out/out.h"

// General constants
#define PROJECT_NAME "REGLESS VM"
#define PROJECT_VERSION "0.1.0 POC"

#define STACK_SIZE 4096

#define VM_EXIT_SUCCESS 0
