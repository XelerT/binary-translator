#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../include/mem_cmds.h"

void encode_mov (x86_cmd_t *cmd, cmd_info4encode_t *info, uint8_t *indent)
{
        assert(cmd);
        assert(info);
        assert(indent);

        if (info->dest_reg != INVALID_REG && info->src_reg != INVALID_REG) {
                cmd->cmd[*indent] = REG_MOV;
                ++*indent;

                cmd->cmd[*indent] = MODE_REG_ADDRESS << 6;

                cmd->cmd[*indent] |= info->dest_reg;
                cmd->cmd[*indent] |= info->src_reg << 3;

                cmd->length = *indent + 1;
        } else if (info->dest_reg != INVALID_REG && info->src_reg == INVALID_REG) {
                if ((uint32_t) info->immed_val < (uint32_t) -1) {
                        if (info->use_memory4dest) {
                                cmd->cmd[*indent] = MEM_IMMED_MOV;
                                ++*indent;
                        } else {
                                cmd->cmd[*indent] = IMMED_MOV;
                        }
                        cmd->cmd[*indent] |= info->dest_reg;
                        memcpy(cmd->cmd + 1 + *indent, &info->immed_val, sizeof(uint32_t));

                        cmd->length = 1 + *indent + (uint8_t) sizeof(uint32_t);
                } else {
                        cmd->cmd[*indent] |= x64bit_PREFIX;
                        ++*indent;

                        cmd->cmd[*indent]   = IMMED_MOV;
                        cmd->cmd[*indent]  |= info->dest_reg;
                        memcpy(cmd->cmd + 2, &info->immed_val, sizeof(uint32_t));

                        cmd->length = 2 + sizeof(uint32_t);
                }
        }
}

void encode_pop_push (x86_cmd_t *cmd, cmd_info4encode_t *info)
{
        assert(cmd);
        assert(info);

        uint8_t indent  = 0;
        uint8_t reg     = info->dest_reg;
        if (reg == INVALID_REG)
                reg = info->src_reg;
        bool use_memory = info->use_memory4dest | info->use_memory4src;

        if (info->dest_reg > RDI && info->dest_reg != INVALID_REG) {
                cmd->cmd[indent++] = USE_DEST_R_REGS;
                reg -= R8;
        } else if (info->src_reg > RDI && info->src_reg != INVALID_REG) {
                cmd->cmd[indent++] = USE_SRC_R_REG;
                reg -= R8;
        }

        if (info->dest_reg == info->src_reg && !use_memory) { /*IMMED*/
                cmd->cmd[indent] = IMMED_PUSH;                       /*0000 0110*/
                cmd->cmd[indent] = cmd->cmd[indent] << 4;            /*0110 0000*/

                cmd->cmd[indent] |= IMMED_PUSH_MRR_MASK;             /*0110 1010*/
                if (info->immed_val <= ASCII_MAX_SYMBOL && -info->immed_val <= ASCII_MAX_SYMBOL)
                        cmd->cmd[indent] |= 2;

                if (info->immed_val < 0)
                        info->immed_val = 0xFF - info->immed_val;

                memcpy(cmd->cmd + 1, &(info->immed_val), get_sizeof_number2write(info->immed_val));

                cmd->length = 1 + get_sizeof_number2write(info->immed_val);                  /* 1 byte for cmd encode and 4 for immed number */
        } else if (use_memory) { /*MEMORY+REG*/
                if (info->use_memory4src || info->src_reg != INVALID_REG) {
                        cmd->cmd[indent] = MEM_REG_PUSH;
                        cmd->cmd[indent + 1]  = IMMED_PUSH << 3;
                } else {
                        cmd->cmd[indent] = MEM_REG_POP;
                }
                cmd->cmd[indent + 1] |= reg;

                cmd->length = 2 + indent;                        /* 1 for MEM_REG_PUSH, 1 for other incoding*/
        } else if (reg != INVALID_REG && !use_memory) {
                cmd->cmd[indent] = REG_PUSH_POP << 5;           /*010 00 000*/
                if (info->use_memory4src || info->src_reg != INVALID_REG)
                        cmd->cmd[indent] |= MODE_8_BYTE_IN_ADDRESS << 3; /*010 10 000*/
                else {
                        cmd->cmd[indent] |= MODE_REG_ADDRESS << 3; /*010 11 000*/
                }
                cmd->cmd[indent] |= reg;

                cmd->length = 1 + indent;                        /*1 for incoding*/
        } else {
                log_error(2, "UNKNOWN PUSH/POP: %d", info->cmd_encode);
        }
}
