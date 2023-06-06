#ifndef MEM_CMDS_H
#define MEM_CMDS_H

#include "config.h"
#include "jit.h"

void encode_mov      (x86_cmd_t *cmd, cmd_info4encode_t *info, uint8_t *indent);
void encode_pop_push (x86_cmd_t *cmd, cmd_info4encode_t *info);

#endif /*MEM_CMDS_H*/
