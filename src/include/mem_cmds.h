#ifndef MEM_CMDS_H
#define MEM_CMDS_H

#include "config.h"
#include "jit.h"

/**
 * @brief encode mov using information from cmd_info4encode_t
 *
 * @param cmd
 * @param info information about mov
 * @param indent show start position in cmd buffer to incode Opcode and further bytes
 */

void encode_mov (x86_cmd_t *cmd, cmd_info4encode_t *info, uint8_t *indent);

/**
 * @brief encode pop/push command using information from cmd_info4encode_t
 *
 * @param cmd
 * @param info information about pop/push
 */

void encode_pop_push (x86_cmd_t *cmd, cmd_info4encode_t *info);

#endif /*MEM_CMDS_H*/
