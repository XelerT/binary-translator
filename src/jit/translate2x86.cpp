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
        cmd_info4incode_t cmd_info = {
                .dest_reg   = R15,
                .immed_val  = (size_t) jit_code->exec_memory2use + jit_code->exec_memory_capacity - sizeof(size_t)
        };
        incode_mov(&set_call_stack_offset, &cmd_info);
        paste_cmd_in_jit_buf(jit_code, &set_call_stack_offset);

        insert_nops(jit_code, 10);

        for (size_t i = 0; i < tokens->size; i += skipped_tokens) {
                x86_cmd_t cmds[5] = {};
                skipped_tokens = convert_tokens2nonstack_logic(tokens, i, jit_code, &label_table);
                if (skipped_tokens)
                        continue;

                if (tokens->tokens[i].my_cmd != CMD_MY_LABEL) {
                        tokens->tokens[i].space = (size_t) jit_code->buf + jit_code->size;
                        x86_cmd_ctor(cmds, tokens->tokens + i, &label_table);

                        for (uint8_t j = 0; cmds[j].length != 0 && j < 5; j++) {
                                paste_cmd_in_jit_buf(jit_code, cmds + j);
                        }
                } else {
                        insert_label(jit_code, tokens->tokens + i, &label_table);
                }
                i++;
        }
        //paste_io_decimal_function(jit_code, &label_table, INVALID_PRINT_ADDRESS);
        //paset_io_decimal_function(jit_code, &label_table, INVALID_SCAN_ADDRESS);

        incode_conditional_jmps(jit_code, &label_table);
        incode_calls_jmps(jit_code, &label_table);

        change_memory_offset(jit_code);
        change_return_value_src2rax(jit_code);

        return 0;
}

void change_return_value_src2rax (jit_code_t *jit_code)
{
        assert(jit_code);

        x86_cmd_t cmd = pop_rax;
        paste_cmd_in_jit_buf(jit_code, &cmd);
}

void change_memory_offset (jit_code_t *jit_code)
{
        assert(jit_code);

        x86_cmd_t cmd = {};

        uint8_t n_byte_after_first_pop = 0;
        while (jit_code->buf[n_byte_after_first_pop] != pop_rbx.cmd[0])
                n_byte_after_first_pop++;
        n_byte_after_first_pop++;

        cmd_info4incode_t cmd_info = {
                .dest_reg   = RBX,
                .immed_val  = (size_t) jit_code->exec_memory2use
        };
        incode_mov(&cmd, &cmd_info);

        uint8_t start_i = 7;
        uint8_t i = start_i;          /*1 + sizeof( mov r15, address)*/
        while (i < cmd.length + start_i) {
                jit_code->buf[i] = cmd.cmd[i - start_i];
                i++;
        }
        while (i < n_byte_after_first_pop) {
                jit_code->buf[i] = nop.cmd[0];
                i++;
        }
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
                // pre_incode_printf_scanf_call(cmds, token, label_table);
                incode_print(cmds, token);
                return 0;
        } else if (token->my_cmd == CMD_MY_IN) {
                incode_scan(cmds, token);
                return 0;
        }

        for (int i = 0; i < N_COMMANDS; i++) {
                if (token->my_cmd == cmds_table[i].my_incode) {
                        assemble_cmd(cmds, token, i, label_table);
                }
        }
        return 0;
}

void assemble_cmd (x86_cmd_t *cmds, token_t *token, size_t table_position, labels_t *label_table)
{
        assert(cmds);
        assert(token);

        if (cmds_table[table_position].code2 == 0) {
                if (cmds_table[table_position].code1 == ADD ||
                    cmds_table[table_position].code1 == SUB ||
                    cmds_table[table_position].code1 == MUL ) {
                        incode_add_sub_mul(cmds, token, table_position);
                } else if (cmds_table[table_position].code1 == RET) {
                        incode_emitation_of_ret(cmds);
                }
        }  else if (cmds_table[table_position].code1 == RELATIVE_CALL) {
                pre_incode_emitation_of_call(cmds, token);
        } else if (cmds_table[table_position].code1 == SHORT_JMP) {
                pre_incode_jmp(cmds, token);
        } else if (cmds_table[table_position].code1 & CONDITIONAL_JMPS_MASK_REL8) {
                uint8_t n_cmds = incode_cmp(cmds);
                pre_incode_conditional_jmp(cmds + n_cmds, token, table_position, label_table);
        } else {
                if (cmds_table[table_position].code1 == IMMED_PUSH ||
                    cmds_table[table_position].code1 == REG_PUSH_POP) {
                        incode_token2push_pop(cmds, token, table_position);
                }
        }
}

void incode_emitation_of_ret (x86_cmd_t *cmds)
{
        assert(cmds);

        cmd_info4incode_t cmd_info = {
                .cmd_incode = ADD,
                .dest_reg   = R15,
                .immed_val  = 8
        };
        incode_add_sub_mul_div(cmds, &cmd_info);
        cmds[1] = push_mem_r15;

        incode_ret(cmds + 2);
}
