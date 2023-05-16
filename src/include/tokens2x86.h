#ifndef TOKENS2X86_H
#define TOKENS2X86_H

#include "config.h"
#include "jit.h"


struct x86_cmd_t {
        uint8_t cmd[X86_CMD_MAX_LENGTH] = {};
        uint8_t length = 0;
        size_t  number = 0;
};

struct my2x86cmd_t {
        char      *my_name  = nullptr;
        int      my_incode  = 0;
        unsigned char code1 = 0;
        unsigned char code2 = 0;
        unsigned char code3 = 0;
};

#include "configs.cmds"

int    fill_jit_code_buf             (jit_code_t *jit_code, tokens_t *tokens);
size_t convert_tokens2nonstack_logic (tokens_t *tokens, size_t n_token, jit_code_t *jit_code);
void   paste_cmd_in_jit_buf          (jit_code_t *jit_code, x86_cmd_t *cmd);
int    x86_cmd_ctor                  (x86_cmd_t *cmd, token_t *token);
void   assemble_cmd                  (x86_cmd_t *cmd, token_t *token, size_t table_position);
void   incode_push_pop               (x86_cmd_t *cmd, token_t *token, size_t table_position);
void   incode_add_sub_mul            (x86_cmd_t *cmd, token_t *token, size_t table_position);

uint8_t insert_add_sub_mul2reg (uint8_t my_cmd, x86_cmd_t *cmds, tokens_t *tokens, size_t position);

uint8_t get_sizeof_number2write (size_t number);

#endif /*TOKENS2X86_H*/
