#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../include/tokens2x86.h"
#include "../include/encode_cmd.h"

#include "../include/configs.cmds"

void encode_cmd (x86_cmd_t *cmd, cmd_info4encode_t *info)
{
        assert(cmd);
        assert(info);

        switch (info->cmd_encode) {
        case MOV:
        case ADD:
        case SUB:
        case MUL:
        case DIV:
                encode_two_operands_cmds(cmd, info);
                break;
        case PUSH:
        case POP:
                encode_pop_push(cmd, info);
                break;
        case JMP:
        case JB:
        case JAE:
        case JE:
        case JNE:
        case JBE:
        case JA:
                set_red_in_terminal();
                fprintf(stderr, "In this version you can only autogenerate jmps using 2 step passage(\n");
                reset_colour_in_terminal();
                break;
        case SQRT:
                log(1, "FYI: Cmd must point on array of 5 cmds.");
                encode_sqrt(cmd);
                break;
        case CALL:
                encode_call(cmd, info);
                break;
        case RET:
                encode_ret(cmd);
                break;
        default:
                set_red_in_terminal();
                fprintf(stderr, "Unknown command!\n");
                reset_colour_in_terminal();
        }
}

void encode_two_operands_cmds (x86_cmd_t *cmd, cmd_info4encode_t *info)
{
        assert(cmd);
        assert(info);

        uint8_t indent = 0;

        if (info->dest_reg > RDI && info->dest_reg != INVALID_REG) {
                cmd->cmd[indent] = USE_DEST_R_REGS;
                info->dest_reg  -= R8;
        }
        if (info->src_reg > RDI && info->src_reg != INVALID_REG) {
                cmd->cmd[indent] = USE_SRC_R_REG;
                info->src_reg   -= R8;
        }
        if ((!cmd->cmd[indent] && !(info->cmd_encode == MOV && info->src_reg == INVALID_REG)) ||
             info->src_reg != INVALID_REG)
                cmd->cmd[indent] |= x64bit_PREFIX;

        if (cmd->cmd[indent])
                indent++;

        if (info->use_memory4src) {
                uint8_t swap = info->dest_reg;
                info->dest_reg = info->src_reg;
                info->src_reg = swap;
        }
        cmd->cmd[indent] |= USE_64bit_OPERANDS;

        switch (info->cmd_encode) {
        case ADD:
        case SUB:
        case MUL:
        case DIV:
                encode_add_sub_mul_div(cmd, info, &indent);
                break;
        case MOV:
                encode_mov(cmd, info, &indent);
                break;
        default:
                set_red_in_terminal();
                fprintf(stderr, "Unknown two operand command!\n");
                reset_colour_in_terminal();
        }

        if (info->use_memory4dest && info->use_memory4src) {
                set_red_in_terminal();
                fprintf(stderr, "Can't use memory as destination and source!\n");
                reset_colour_in_terminal();

                log_error(1, "Can't use memory as destination and source!");
        } else if (info->use_memory4dest || info->use_memory4src) {
                cmd->cmd[indent] &= ((uint8_t) ~MODE_USE_REG) >> 2;   /*00|xxxxxx to change only mode*/
        }
        if (info->use_memory4dest)
                cmd->cmd[indent - 1] |= DEST_REG_MEMORY;
        else if (info->use_memory4src)
                cmd->cmd[indent - 1] |= DEST_MEMORY_REG;
}
