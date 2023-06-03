#ifndef MEM_CMDS_H
#define MEM_CMDS_H

#include "../include/config.h"
#include "../include/jit.h"

void incode_mov      (x86_cmd_t *cmd, cmd_info4incode_t *info);
void incode_pop_push (x86_cmd_t *cmd, cmd_info4incode_t *info);

#endif /*MEM_CMDS_H*/
