#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../include/math_cmds.h"
#include "../include/tokens2x86.h"
#include "../include/encode_cmd.h"

#pragma GCC diagnostic ignored "-Wconversion"

void encode_add_sub_mul_div (x86_cmd_t *cmd, cmd_info4encode_t *info, uint8_t *indent)
{
        assert(cmd);
        assert(info);
        assert(indent);

        if (info->src_reg != INVALID_REG && (info->cmd_encode == MUL || info->cmd_encode == DIV)) {
                set_red_in_terminal();
                fprintf(stderr, "Can't use source for mul/div(cmd_incode = %d)!\n", info->cmd_encode);
                reset_colour_in_terminal();
                return;
        }

        cmd->cmd[*indent] |= info->cmd_encode;
        ++*indent;

        if (info->dest_reg != INVALID_REG && info->src_reg != INVALID_REG) {
                cmd->cmd[*indent]  = MODE_REG_ADDRESS << 6;

                cmd->cmd[*indent] |= info->dest_reg;
                cmd->cmd[*indent] |= info->src_reg << 3;

                cmd->length = *indent + 1;
        } else if ((info->dest_reg != INVALID_REG && info->src_reg == INVALID_REG) &&
                   (info->cmd_encode != MUL && info->cmd_encode != DIV)) {
                cmd->cmd[*indent] = (MODE_REG_ADDRESS << 6);

                if (info->cmd_encode == SUB || info->cmd_encode == ADD)
                        cmd->cmd[*indent - 1] = ADD_SUB_IMMED;
                if (info->cmd_encode == SUB)
                        cmd->cmd[*indent] |= (IMMED_SUB_MASK << 3);

                cmd->cmd[*indent] |= info->dest_reg;

                if (get_sizeof_number2write(info->immed_val) == 1)
                        cmd->cmd[*indent - 1] |= 2;
                memcpy(cmd->cmd + *indent + 1, &info->immed_val, get_sizeof_number2write(info->immed_val));

                cmd->length = get_sizeof_number2write(info->immed_val) + *indent + 1;
        } else if (info->cmd_encode == MUL || info->cmd_encode == DIV) {
                cmd->cmd[*indent - 1] = MUL;
                cmd->cmd[*indent]  = (MODE_REG_ADDRESS << 6);
                cmd->cmd[*indent] |= info->dest_reg;

                if (info->cmd_encode == MUL)
                        cmd->cmd[*indent] |= MUL_MASK;
                else
                        cmd->cmd[*indent] |= DIV_MASK;

                cmd->length = *indent + 1;
        }
}

#pragma GCC diagnostic warning "-Wconversion"

#include "../include/consts_x86.cmds"

void encode_sqrt (x86_cmd_t *cmds)
{
        assert(cmds);

        cmd_info4encode_t cmd_info = {
                .cmd_encode = POP,
                .dest_reg = RAX
        };
        encode_cmd(cmds + 0, &cmd_info);

        cmds[1] = cvtsi2sd_xmm0_rax;
        cmds[2] = sqrtsd_xmm0_xmm0;
        cmds[3] = cvttsd2si_rax_xmm0;

        cmd_info.dest_reg   = INVALID_REG;
        cmd_info.src_reg    = RAX;
        cmd_info.cmd_encode = POP;
        encode_cmd(cmds + 4, &cmd_info);
}
