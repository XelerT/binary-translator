#ifndef JIT_H
#define JIT_H

#include <stdint.h>

#include "config.h"
#include "tokens.h"

#include "configs.cmds"

/** \struct
 * @brief Contains information about code for JIT compilation
 *
 * It contains pointer to buffer with encoded x86 commands, capacity and size
 * of buffer, exec_memory2use is pointer to the "data" section, offsets for print and scan functions.
 *
 */

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

/** \struct
 * @brief Contains x86 command
 * Contains buffer for command, it's length a
 */

struct x86_cmd_t {
        uint8_t cmd[X86_CMD_MAX_LENGTH] = {};
        uint8_t length = 0;
};

/** \struct
 * @brief Contains information which used to encode x86_cmd_t.
 *
 */

struct cmd_info4encode_t {
        uint8_t cmd_encode   = 0;
        uint8_t dest_reg     = INVALID_REG;
        uint8_t src_reg      = INVALID_REG;
        int     immed_val    = 0;
        bool use_memory4dest = 0;
        bool use_memory4src  = 0;
};

enum jit_errors {
        CHANGE_PROTECTION_ERROR = 0xBAD9301
};

/**
 * @brief Execute code, contained in tokens.
 *
 * @param tokens contains processed bit code.
 * @return int errors
 */

int  jit (tokens_t *tokens);

/**
 * @brief Constructor for jit_code_t variable
 *
 * @param jit_code pointer to jit_code_t to construct
 * @param capacity maximum size of "text" and "data" sections buffers
 * @return int errors
 */
int  jit_code_ctor (jit_code_t *jit_code, size_t capacity);

/**
 * @brief Destructor for jit_code_t
 *
 * @param jit_code pointer to jit_code_t to destruct
 */
void jit_code_dtor (jit_code_t *jit_code);

/**
 * @brief Makes buffer executable, readable and writable using mprotect
 *
 * @param buf pointer to buffer to make executable.
 * @param buf_capacity size of buffer
 * @return int errors
 */
int  make_buf_executable (void *buf, size_t buf_capacity);
int  make_buf_writable   (void *buf, size_t buf_capacity);

/**
 * @brief Run buffer as a function
 *
 * @param buf_func pointer to buffer to execute
 */
void run_jit_buffer      (void (*buf_func) ());

#endif /*JIT_H*/
