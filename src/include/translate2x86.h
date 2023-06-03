#ifndef TRANSLATE2x86_H
#define TRANSLATE2x86_H

#include "../include/config.h"
#include "../include/tokens.h"
#include "../include/tokens2x86.h"
#include "../include/jit.h"

int  fill_jit_code_buf (jit_code_t *jit_code, tokens_t *tokens);
void paste_cmd_in_jit_buf (jit_code_t *jit_code, x86_cmd_t *cmd);

int  x86_cmd_ctor (x86_cmd_t *cmds, token_t *token, labels_t *label_table);
void assemble_cmd (x86_cmd_t *cmds, token_t *token, size_t table_position, labels_t *label_table);

void change_return_value_src2rax (jit_code_t *jit_code);
void change_memory_offset        (jit_code_t *jit_code);
void incode_emitation_of_ret     (x86_cmd_t  *cmds);

#endif /*TRANSLATE2x86_H*/
