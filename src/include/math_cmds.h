#ifndef MATH_CMDS_H
#define MATH_CMDS_H

#include "config.h"
#include "jit.h"
#include "mem_cmds.h"

void encode_add_sub_mul_div (x86_cmd_t *cmd, cmd_info4encode_t *info);
void encode_sqrt            (x86_cmd_t *cmds);

#endif /*MATH_CMDS_H*/
