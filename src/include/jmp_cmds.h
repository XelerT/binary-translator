#ifndef JMP_CMDS_H
#define JMP_CMDS_H

#include "config.h"
#include "tokens.h"
#include "tokens2x86.h"
#include "jit.h"

void pre_encode_emitation_of_call (x86_cmd_t *cmds, token_t *token);

void pre_encode_jmp    (x86_cmd_t *cmd, token_t *token);
void encode_calls_jmps (jit_code_t *jit_code, labels_t *label_table);

void pre_encode_call   (x86_cmd_t *cmd, token_t *token);
void encode_call       (x86_cmd_t *cmd, cmd_info4encode_t *info);
void encode_ret        (x86_cmd_t *cmd);

void pre_encode_conditional_jmp (x86_cmd_t *cmd, token_t *token, size_t table_position,
                                                                 labels_t *label_table);
void encode_conditional_jmps (jit_code_t *jit_code, labels_t *label_table);

#endif /*JMP_CMDS_H*/
