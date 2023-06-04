#ifndef JIT_H
#define JIT_H

#include <stdint.h>

#include "config.h"
#include "tokens.h"

#include "configs.cmds"

struct jit_code_t {
        uint8_t *buf     = nullptr;
        size_t  capacity = 0;
        size_t  size     = 0;

        uint8_t *exec_memory2use = nullptr;
        size_t exec_memory_capacity = 0;

        size_t print_func_offset = 10;
        size_t print_func_size   =  0;

        size_t scan_func_offset  = 10;
        size_t scan_func_size    =  0;
};

struct x86_cmd_t {
        uint8_t cmd[X86_CMD_MAX_LENGTH] = {};
        uint8_t length = 0;
        size_t  number = 0;
};

struct cmd_info4incode_t {
        uint8_t cmd_incode   = 0;
        uint8_t dest_reg     = INVALID_REG;
        uint8_t src_reg      = INVALID_REG;
        size_t immed_val     = 0;
        bool use_memory4dest = 0;
        bool use_memory4src  = 0;
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
