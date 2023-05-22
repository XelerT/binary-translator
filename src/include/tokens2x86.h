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
        char       *my_name = nullptr;
        int       my_incode = 0;
        unsigned char code1 = 0;
        unsigned char code2 = 0;
        unsigned char code3 = 0;
};

struct label_t {
        size_t my_address  = 0;
        size_t new_address = 0;
};

struct labels_t {
        label_t labels[MAX_N_LABELS] = {};
        size_t size = 0;
};

#include "configs.cmds"

struct cmd_info4incode_t {
        uint8_t cmd_incode   = 0;
        uint8_t dest_reg     = INVALID_REG;
        uint8_t src_reg      = INVALID_REG;
        size_t immed_val     = 0;
        bool use_memory4dest = 0;
        bool use_memory4src  = 0;
};

#include "configs.cmds"

int    fill_jit_code_buf             (jit_code_t *jit_code, tokens_t *tokens);
void   change_return_value_src2rax   (jit_code_t *jit_code);
void   insert_nops                   (jit_code_t *jit_code, size_t amount2insert);
size_t convert_tokens2nonstack_logic (tokens_t *tokens, size_t n_token, jit_code_t *jit_code, labels_t *label_table);
void   change_memory_offset          (jit_code_t *jit_code);
void   paste_cmd_in_jit_buf          (jit_code_t *jit_code, x86_cmd_t *cmd);
void   incode_call                   (x86_cmd_t *cmd, cmd_info4incode_t *info);
int    x86_cmd_ctor                  (x86_cmd_t *cmd, token_t *token, labels_t *label_table);
void   assemble_cmd                  (x86_cmd_t *cmd, token_t *token, size_t table_position, labels_t *label_table);
void   incode_push_pop               (x86_cmd_t *cmd, token_t *token, size_t table_position);
void   incode_add_sub_mul            (x86_cmd_t *cmd, token_t *token, size_t table_position);

void   incode_mov (x86_cmd_t *cmd, cmd_info4incode_t *info);

uint8_t insert_add_sub_mul_div2reg (uint8_t my_cmd, x86_cmd_t *cmds, tokens_t *tokens, size_t position, labels_t *label_table);

uint8_t get_sizeof_number2write (size_t number);

void incode_print (x86_cmd_t *cmds, token_t *token);
void incode_scan  (x86_cmd_t *cmds, token_t *token);

void    incode_ret                   (x86_cmd_t *cmd);
uint8_t incode_test                  (x86_cmd_t *cmds);
uint8_t incode_cmp                   (x86_cmd_t *cmds);
void    insert_label                 (jit_code_t *jit_code, token_t *token, labels_t *label_table);
void    pre_incode_emitation_of_call (x86_cmd_t *cmds, token_t *token);
void    incode_emitation_of_ret      (x86_cmd_t *cmds);
void    incode_add_sub_mul_div       (x86_cmd_t *cmd, cmd_info4incode_t *info);
void    pre_incode_call              (x86_cmd_t *cmd, token_t *token);
void    incode_calls_jmps            (jit_code_t *jit_code, labels_t *label_table);
void    pre_incode_conditional_jmp   (x86_cmd_t *cmd, token_t *token, size_t table_position, labels_t *label_table);
void    incode_jmp                   (x86_cmd_t *cmd, token_t *token, size_t table_position, labels_t *label_table);
void    pre_incode_jmp               (x86_cmd_t *cmd, token_t *token);
size_t  find_label                   (labels_t *label_table, uint32_t my_offset);
void    incode_conditional_jmps      (jit_code_t *jit_code, labels_t *label_table);

#endif /*TOKENS2X86_H*/
