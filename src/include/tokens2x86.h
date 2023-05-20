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

int    fill_jit_code_buf             (jit_code_t *jit_code, tokens_t *tokens);
void   insert_nops                   (jit_code_t *jit_code, size_t amount2insert);
size_t convert_tokens2nonstack_logic (tokens_t *tokens, size_t n_token, jit_code_t *jit_code, labels_t *label_table);
void   change_memory_offset          (jit_code_t *jit_code);
void   paste_cmd_in_jit_buf          (jit_code_t *jit_code, x86_cmd_t *cmd);
int    x86_cmd_ctor                  (jit_code_t *jit_code, x86_cmd_t *cmd, token_t *token, labels_t *label_table);
void   assemble_cmd                  (jit_code_t *jit_code, x86_cmd_t *cmd, token_t *token, size_t table_position, labels_t *label_table);
void   incode_push_pop               (x86_cmd_t *cmd, token_t *token, size_t table_position);
void   incode_add_sub_mul            (x86_cmd_t *cmd, token_t *token, size_t table_position);

void incode_mov (x86_cmd_t *cmd, uint8_t dest_reg, uint8_t src_reg, size_t val);

uint8_t insert_add_sub_mul_div2reg (jit_code_t *jit_code, uint8_t my_cmd, x86_cmd_t *cmds, tokens_t *tokens, size_t position, labels_t *label_table);

uint8_t get_sizeof_number2write (size_t number);

void    incode_ret      (x86_cmd_t *cmd);
uint8_t incode_test  (x86_cmd_t *cmds);
uint8_t incode_cmp   (x86_cmd_t *cmds);
void insert_label    (jit_code_t *jit_code, token_t *token, labels_t *label_table);
void pre_incode_call (x86_cmd_t *cmds, token_t *token, size_t table_position, labels_t *label_table);
void incode_calls_jmps (jit_code_t *jit_code, labels_t *label_table);
void pre_incode_conditional_jmp (x86_cmd_t *cmd, token_t *token, size_t table_position, labels_t *label_table);
void incode_jmp (x86_cmd_t *cmd, token_t *token, size_t table_position, labels_t *label_table);
void pre_incode_jmp (x86_cmd_t *cmd, token_t *token, size_t table_position, labels_t *label_table);
size_t find_label (labels_t *label_table, uint32_t my_offset);
void incode_conditional_jmps (jit_code_t *jit_code, labels_t *label_table);

#endif /*TOKENS2X86_H*/
