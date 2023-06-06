#ifndef ENCODE_CMD_H
#define ENCODE_CMD_H

#include "config.h"
#include "jit.h"

#include "tokens2x86.h"

#include "mem_cmds.h"
#include "math_cmds.h"
#include "jmp_cmds.h"
#include "conditional_cmds.h"

void encode_cmd               (x86_cmd_t *cmd, cmd_info4encode_t *info);
void encode_two_operands_cmds (x86_cmd_t *cmd, cmd_info4encode_t *info);

#endif /*ENCODE_CMD_H*/
