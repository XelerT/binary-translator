#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../include/math_cmds.h"
#include "../include/tokens2x86.h"

void encode_add_sub_mul_div (x86_cmd_t *cmd, cmd_info4encode_t *info)
{
        assert(cmd);
        assert(info);

        uint8_t indent = 0;

        cmd->cmd[indent] = x64bit_PREFIX;

        if (info->dest_reg > RDI && info->dest_reg != INVALID_REG) {
                cmd->cmd[indent] |= USE_R_REGS;
                info->dest_reg -= R8;
        }
        if (info->src_reg > RDI && info->src_reg != INVALID_REG) {
                cmd->cmd[indent] |= USE_SRC_R_REG;
                info->src_reg -= R8;
        }
        if (cmd->cmd[indent])
                indent++;

        cmd->cmd[indent++] = info->cmd_encode;

        if (info->dest_reg != INVALID_REG && info->src_reg != INVALID_REG) {
                cmd->cmd[indent - 1] |= 1;
                cmd->cmd[indent]  = (MODE_REG_ADDRESS << 6);
                cmd->cmd[indent] |= info->dest_reg;
                cmd->cmd[indent] |= (info->src_reg << 3);
                cmd->length = indent + 1;
        } else if (info->dest_reg != INVALID_REG && info->src_reg == INVALID_REG) {
                cmd->cmd[indent]  = (MODE_REG_ADDRESS << 6);
                if (info->cmd_encode == SUB || info->cmd_encode == ADD)
                        cmd->cmd[indent - 1] = ADD_SUB_IMMED;
                if (info->cmd_encode == SUB)
                        cmd->cmd[indent] |= (IMMED_SUB_MASK << 3);
                cmd->cmd[indent] |= info->dest_reg;

                if (get_sizeof_number2write(info->immed_val) == 1)
                        cmd->cmd[indent - 1] |= 2;
                memcpy(cmd->cmd + indent + 1, &info->immed_val, get_sizeof_number2write(info->immed_val));

                cmd->length = get_sizeof_number2write(info->immed_val) + indent + 1;
        } else if (info->dest_reg == INVALID_REG && info->src_reg != INVALID_REG) {
                if (info->cmd_encode != MUL && info->cmd_encode != DIV) {
                        set_red_in_terminal();
                        fprintf(stderr, "Can't use only source register for non div or mul command!");
                        reset_colour_in_terminal();
                }
                cmd->cmd[indent - 1] = MUL;
                cmd->cmd[indent]  = (MODE_REG_ADDRESS << 6);
                cmd->cmd[indent] |= info->src_reg;
                if (info->cmd_encode == MUL)
                        cmd->cmd[indent] |= MUL_MASK;
                else
                        cmd->cmd[indent] |= DIV_MASK;
                cmd->length = indent + 1;
        }

        if (info->use_memory4dest && info->use_memory4src) {
                set_red_in_terminal();
                fprintf(stderr, "Can't use memory as destination and source!");
                reset_colour_in_terminal();

                log_error(1, "Can't use memory as destination and source!");
        } else if (info->use_memory4dest) {
                cmd->cmd[indent - 1] |= 0x0;
        } else if (info->use_memory4src) {
                cmd->cmd[indent] &= ((uint8_t) ~MODE_USE_REG) >> 2;   /*00|xxxxxx to change only mode*/
        }
}
