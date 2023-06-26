#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../include/translate2x86.h"
#include "../include/tokens2x86.h"
#include "../include/myIO.h"

#include "../include/mem_cmds.h"
#include "../include/math_cmds.h"
#include "../include/jmp_cmds.h"
#include "../include/conditional_cmds.h"

#include "../include/consts_x86.cmds"

int fill_jit_code_buf (jit_code_t *jit_code, tokens_t *tokens)
{
        assert(jit_code);
        assert(tokens);

        size_t skipped_tokens = 0;
        labels_t label_table = {};

        x86_cmd_t set_call_stack_offset = {};
        cmd_info4encode_t cmd_info = {
                .cmd_encode = MOV,
                .dest_reg   = R15,
                .immed_val  = (int) ((size_t) jit_code->exec_memory2use + jit_code->exec_memory_capacity - sizeof(size_t))
        };

        encode_cmd(&set_call_stack_offset, &cmd_info);
        paste_cmd_in_jit_buf(jit_code, &set_call_stack_offset);

        insert_nops(jit_code, 10);

        for (size_t i = 0; i < tokens->size; i += skipped_tokens) {
                x86_cmd_t cmds[6] = {};

                skipped_tokens = convert_tokens2nonstack_logic(tokens, i, jit_code);
                if (skipped_tokens)
                        continue;

                if (tokens->tokens[i].my_cmd != CMD_MY_LABEL) {
                        tokens->tokens[i].space = (size_t) jit_code->buf + jit_code->size;
                        x86_cmd_ctor(cmds, tokens->tokens + i, &label_table);

                        write_cmds_in_jit_code(jit_code, cmds, 5);
                } else {
                        insert_label(jit_code, tokens->tokens + i, &label_table);
                }
                i++;
        }

        encode_conditional_jmps(jit_code, &label_table);
        encode_calls_jmps(jit_code, &label_table);

        change_memory_offset(jit_code);
        change_return_value_src2rax(jit_code);

        return 0;
}

void paste_cmd_in_jit_buf (jit_code_t *jit_code, x86_cmd_t *cmd)
{
        assert(jit_code);
        assert(cmd);

        for (uint8_t i = 0; i < cmd->length; i++) {
                jit_code->buf[jit_code->size] = cmd->cmd[i];
                jit_code->size++;
        }
}

int x86_cmd_ctor (x86_cmd_t *cmds, token_t *token, labels_t *label_table)
{
        assert(cmds);
        assert(token);
        assert(label_table);

        if (token->my_cmd == CMD_MY_OUT) {
                encode_print(cmds, token);
                return 0;
        } else if (token->my_cmd == CMD_MY_IN) {
                encode_scan(cmds, token);
                return 0;
        }

        for (int i = 0; i < N_COMMANDS; i++) {
                if (token->my_cmd == CMDS_TABLE[i].my_encode) {
                        assemble_cmd(cmds, token, i, label_table);
                }
        }
        return 0;
}

void assemble_cmd (x86_cmd_t *cmds, token_t *token, size_t table_position, labels_t *label_table)
{
        assert(cmds);
        assert(token);

        if (CMDS_TABLE[table_position].code2 == 0) {
                if (CMDS_TABLE[table_position].code1 == RET) {
                        encode_imitation_of_ret(cmds);
                } else if (CMDS_TABLE[table_position].code1 == CMD_MY_SQRT) {
                        encode_sqrt(cmds);
                }
        }  else if (CMDS_TABLE[table_position].code1 == RELATIVE_CALL) {
                pre_encode_imitation_of_call(cmds, token);
        } else if (CMDS_TABLE[table_position].code1 == SHORT_JMP) {
                pre_encode_jmp(cmds, token);
        } else if (CMDS_TABLE[table_position].code1 & CONDITIONAL_JMPS_MASK_REL8) {
                uint8_t n_cmds = encode_cmp(cmds);
                pre_encode_conditional_jmp(cmds + n_cmds, token, table_position, label_table);
        } else {
                if (CMDS_TABLE[table_position].code1 == IMMED_PUSH ||
                    CMDS_TABLE[table_position].code1 == REG_PUSH_POP) {
                        encode_token2push_pop(cmds, token, table_position);
                }
        }
}

void change_return_value_src2rax (jit_code_t *jit_code)
{
        assert(jit_code);

        x86_cmd_t cmd = {};
        cmd_info4encode_t pop_rax_info = {
                .cmd_encode = POP,
                .dest_reg   = RAX
        };
        encode_cmd(&cmd, &pop_rax_info);
        paste_cmd_in_jit_buf(jit_code, &cmd);
}

void change_memory_offset (jit_code_t *jit_code)
{
        assert(jit_code);

        x86_cmd_t cmd = {};

        uint8_t n_byte_after_first_pop = 0;
        while (jit_code->buf[n_byte_after_first_pop] != pop_rbx.cmd[0] && n_byte_after_first_pop < jit_code->size)
                n_byte_after_first_pop++;
        n_byte_after_first_pop++;

        cmd_info4encode_t cmd_info = {
                .cmd_encode = MOV,
                .dest_reg   = RBX,
                .immed_val  = (int) (size_t) jit_code->exec_memory2use
        };
        encode_cmd(&cmd, &cmd_info);

        uint8_t start_i = 7;            /*1 + sizeof( mov r15, address)*/
        uint8_t i = start_i;

        while (i < cmd.length + start_i) {
                jit_code->buf[i] = cmd.cmd[i - start_i];
                i++;
        }
        while (i < n_byte_after_first_pop) {
                jit_code->buf[i] = nop.cmd[0];
                i++;
        }
}

void encode_imitation_of_ret (x86_cmd_t *cmds)
{
        assert(cmds);

        cmd_info4encode_t cmd_info = {
                .cmd_encode = ADD,
                .dest_reg   = R15,
                .immed_val  = 8
        };
        encode_cmd(cmds, &cmd_info);
        cmds[1] = push_mem_r15;

        encode_ret(cmds + 2);
}
