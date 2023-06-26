#ifndef MATH_CMDS_H
#define MATH_CMDS_H

#include "config.h"
#include "jit.h"
#include "mem_cmds.h"

/**
 * @brief encode add/sub/mul/div commands using cmd_info4encode_t
 *
 * @param cmd
 * @param info must contain information about command
 * @param indent show start position in cmd buffer to incode Opcode and further bytes
 */
void encode_add_sub_mul_div (x86_cmd_t *cmd, cmd_info4encode_t *info, uint8_t *indent);

/**
 * @brief encode stack sqrt function using xmm commands
 *
 * @param cmds pointer to 5 cmds
 */
void encode_sqrt            (x86_cmd_t *cmds);

#endif /*MATH_CMDS_H*/
