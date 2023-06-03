#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../include/jmp_cmds.h"
#include "../include/mem_cmds.h"
#include "../include/math_cmds.h"

void incode_call (x86_cmd_t *cmd, cmd_info4incode_t *info)
{
        assert(cmd);
        assert(info);

        uint8_t indent = 0;

        if (info->src_reg > RDI && info->src_reg != INVALID_REG) {
                cmd->cmd[indent++] = USE_R_REGS;
                info->src_reg -= R8;
        }

        if (info->src_reg != INVALID_REG && !info->use_memory4src) {
                cmd->cmd[indent] = REG_CALL;
                cmd->length = indent + 1;
        } else if (!info->use_memory4src) {
                cmd->cmd[indent++] = RELATIVE_CALL;

                memcpy(cmd->cmd + indent, &info->immed_val, sizeof(uint32_t));
                cmd->length = 1 + sizeof(uint32_t);
        } else if (!info->use_memory4src) {
                cmd->cmd[indent++] = FAR_CALL;

                memcpy(cmd->cmd + indent, &info->immed_val, sizeof(size_t));
                cmd->length = 1 + sizeof(size_t);
        }
}

void incode_ret (x86_cmd_t *cmd)
{
        assert(cmd);

        cmd->cmd[0] = RET;
        cmd->length = 1;
}

void pre_incode_emitation_of_call (x86_cmd_t *cmds, token_t *token)
{
        assert(cmds);

        cmd_info4incode_t cmd_info = {
                .dest_reg   = R15,
                .immed_val = token->space + 16,
                .use_memory4dest = 1
        };
        incode_mov(cmds + 0, &cmd_info);

        cmd_info.cmd_incode = SUB;
        cmd_info.use_memory4dest = 0;

        cmd_info.dest_reg   = R15;
        cmd_info.src_reg = INVALID_REG;
        cmd_info.immed_val = 8;
        cmd_info.use_memory4dest = 0;

        incode_add_sub_mul_div(cmds + 1, &cmd_info);
        pre_incode_jmp(cmds + 2, token);
}

void pre_incode_call (x86_cmd_t *cmd, token_t *token)
{
        assert(cmd);
        assert(token);

        if (token->offset) {
                size_t offset = token->offset;

                cmd->cmd[0] = RELATIVE_CALL;
                memcpy(cmd->cmd + 1, &offset, sizeof(uint8_t));
                cmd->length = 1 + sizeof(uint8_t);
        } else {
                /*register call*/
        }
}

void incode_calls_jmps (jit_code_t *jit_code, labels_t *label_table)
{
        assert(jit_code);
        assert(label_table);

        for (size_t i = 0; i < jit_code->size; i++) {
                if (jit_code->buf[i] == RELATIVE_CALL || jit_code->buf[i] == NEAR_JMP) {
                        uint32_t my_offset = *((uint32_t*)(jit_code->buf + i + 1));
                        size_t n_label = find_label(label_table, my_offset);

                        if (n_label >= label_table->size && my_offset < jit_code->size) {
                                log_error(1, "No label found");
                                return;
                        } else if (n_label >= label_table->size) {
                                continue;
                        }
                        uint32_t new_offset = (uint32_t) (label_table->labels[n_label].new_address - (i + JMP_LENGTH));

                        memcpy(jit_code->buf + i + 1, &new_offset, sizeof(uint32_t));
                        i += sizeof(uint32_t);
                }
        }
}

void pre_incode_conditional_jmp (x86_cmd_t *cmd, token_t *token, size_t table_position,
                                                        labels_t *label_table)
{
        assert(cmd);
        assert(token);
        assert(label_table);

        size_t offset = token->offset;

        cmd->cmd[0] = PREFIX_64_bit;
        cmd->cmd[1] = cmds_table[table_position].code2;
        memcpy(cmd->cmd + 2, &offset, sizeof(uint32_t));
        cmd->length = 2 + sizeof(uint32_t);
}

void incode_conditional_jmps (jit_code_t *jit_code, labels_t *label_table)
{
        assert(jit_code);
        assert(label_table);

        for (size_t i = 0; i < jit_code->size; i++) {
                if (jit_code->buf[i] == PREFIX_64_bit && (jit_code->buf[i + 1] & CONDITIONAL_JMPS_MASK_REL32)) {
                        uint32_t my_offset  = jit_code->buf[i + 2];
                        size_t n_label = find_label(label_table, my_offset);
                        if (n_label >= label_table->size) {
                                log_error(1, "No label found");
                                return;
                        }
                        uint32_t new_offset = (uint32_t) (label_table->labels[n_label].new_address - (i + JMP_LENGTH + 1));

                        memcpy(jit_code->buf + i + 2, &new_offset, sizeof(uint32_t));
                        i += sizeof(uint32_t);
                }
        }
}

void pre_incode_jmp (x86_cmd_t *cmd, token_t *token)
{
        assert(cmd);
        assert(token);

        size_t offset = token->offset;

        cmd->cmd[0] = NEAR_JMP;
        memcpy(cmd->cmd + 1, &offset, sizeof(uint32_t));
        cmd->length = 1 + sizeof(uint32_t);
}
