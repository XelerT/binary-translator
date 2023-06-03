#ifndef JMP_CMDS_H
#define JMP_CMDS_H

#include "config.h"
#include "tokens.h"
#include "tokens2x86.h"
#include "jit.h"

void pre_incode_emitation_of_call (x86_cmd_t *cmds, token_t *token);

void pre_incode_jmp    (x86_cmd_t *cmd, token_t *token);
void incode_calls_jmps (jit_code_t *jit_code, labels_t *label_table);

void pre_incode_call   (x86_cmd_t *cmd, token_t *token);
void incode_call       (x86_cmd_t *cmd, cmd_info4incode_t *info);
void incode_ret        (x86_cmd_t *cmd);

void pre_incode_conditional_jmp (x86_cmd_t *cmd, token_t *token, size_t table_position,
                                                                 labels_t *label_table);
void incode_conditional_jmps (jit_code_t *jit_code, labels_t *label_table);

#endif /*JMP_CMDS_H*/
