#ifndef CONDITIONAL_CMDS_H
#define CONDITIONAL_CMDS_H

#include "jit.h"

uint8_t encode_test (x86_cmd_t *cmds);
uint8_t encode_cmp  (x86_cmd_t *cmds);

#endif /*CONDITIONAL_CMDS_H*/
