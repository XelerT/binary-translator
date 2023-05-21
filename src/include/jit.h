#ifndef JIT_H
#define JIT_H

#include <stdint.h>

#include "../include/config.h"
#include "../include/tokens.h"

struct jit_code_t {
        uint8_t *buf     = nullptr;
        size_t  capacity = 0;
        size_t  size     = 0;

        uint8_t *exec_memory2use = nullptr;
        size_t exec_memory_capacity = 0;
};

enum jit_errors {
        CHANGE_PROTECTION_ERROR = 0xBAD9301
};

int  jit (tokens_t *tokens);
int  jit_code_ctor (jit_code_t *jit_code, size_t capacity);
void jit_code_dtor (jit_code_t *jit_code);
int  make_buf_executable (void *buf, size_t buf_capacity);
int  make_buf_writable   (void *buf, size_t buf_capacity);
void run_jit_buffer      (void (*buf_func) ());

#endif /*JIT_H*/
