#ifndef TOKENS2X86_H
#define TOKENS2X86_H

#include "config.h"
#include "jit.h"

/**
 * @brief Contains address of label in soft-cpu bit code and it's new address in jit_code buffer
 *
 */
struct label_t {
        size_t my_address  = 0;
        size_t new_address = 0;
};

/**
 * @brief Array of label_t
 *
 */
struct labels_t {
        label_t labels[MAX_N_LABELS] = {};
        size_t size = 0;
};

/**
 * @brief insert nop commands in jit_code buffer
 *
 * @param jit_code
 * @param amount2insert
 */
void    insert_nops (jit_code_t *jit_code, size_t amount2insert);

/**
 * Check and encode stack constructions like
 *      push 12
 *      add
 * to
 *      pop rax
 *      add rax, 12
 *
 * @param tokens
 * @param n_token
 * @param jit_code
 * @return size_t number of tokens which was translated
 */
size_t  convert_tokens2nonstack_logic (tokens_t *tokens, size_t n_token, jit_code_t *jit_code);

/**
 * @brief If commands access memory and change rbx it will mov rbx to rdi and change rdi access memory through rdi
 *
 * @param jit_code
 * @param tokens
 * @param n_token
 */
void    find_convert_memory_access    (jit_code_t *jit_code, tokens_t *tokens, size_t n_token);

/**
 * @brief Change returning value of functions from rdx to rax
 *
 * @param jit_code
 * @param tokens
 * @param n_token
 */
void    change_return_value_src       (jit_code_t *jit_code, tokens_t *tokens, size_t n_token);

uint8_t get_cmd_from_token            (token_t *token);
void write_cmds_in_jit_code           (jit_code_t *jit_code, x86_cmd_t *cmds, uint8_t max_cmds2write);

/**
 * @brief Find label in labels_t using offset in jmp/call to jump
 *
 * @param label_table
 * @param my_offset
 * @return size_t position in table if label is in table and amount of labels in label_table + 1
 */
size_t find_label   (labels_t *label_table, uint32_t my_offset);

/**
 * @brief Insert label in labels_t
 *
 * @param jit_code
 * @param token contains my_label info
 * @param label_table
 */
void   insert_label (jit_code_t *jit_code, token_t *token, labels_t *label_table);

void pre_encode_print_scan_call (x86_cmd_t *cmd, token_t *token, labels_t *label_table);
void encode_print               (x86_cmd_t *cmds, token_t *token);
void encode_scan                (x86_cmd_t *cmds, token_t *token);

/**
 * @brief encode push/pop using information from token
 *
 * @param cmd
 * @param token
 * @param table_position
 */
void encode_token2push_pop      (x86_cmd_t *cmd, token_t *token, size_t table_position);

#endif /*TOKENS2X86_H*/
