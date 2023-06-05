#ifndef TOKENS2X86_H
#define TOKENS2X86_H

#include "config.h"
#include "jit.h"

struct label_t {
        size_t my_address  = 0;
        size_t new_address = 0;
};

struct labels_t {
        label_t labels[MAX_N_LABELS] = {};
        size_t size = 0;
};

void    insert_nops (jit_code_t *jit_code, size_t amount2insert);

size_t  convert_tokens2nonstack_logic (tokens_t *tokens, size_t n_token, jit_code_t *jit_code);
void    find_convert_memory_access    (jit_code_t *jit_code, tokens_t *tokens, size_t n_token);
void    change_return_value_src       (jit_code_t *jit_code, tokens_t *tokens, size_t n_token);
uint8_t get_cmd_from_token            (token_t *token);

size_t find_label   (labels_t *label_table, uint32_t my_offset);
void   insert_label (jit_code_t *jit_code, token_t *token, labels_t *label_table);

void pre_encode_print_scan_call (x86_cmd_t *cmd, token_t *token, labels_t *label_table);
void encode_print               (x86_cmd_t *cmds, token_t *token);
void encode_scan                (x86_cmd_t *cmds, token_t *token);

void encode_token2push_pop      (x86_cmd_t *cmd, token_t *token, size_t table_position);

#endif /*TOKENS2X86_H*/
