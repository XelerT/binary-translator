#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../include/config.h"
#include "../include/tokens.h"
#include "../include/jit.h"
#include "../include/tokens2x86.h"

int fill_jit_code_buf (jit_code_t *jit_code, tokens_t *tokens)
{
        assert(jit_code);
        assert(tokens);

        size_t skipped_tokens = 0;

        for (size_t i = 0; i < tokens->size; i += skipped_tokens) {
                x86_cmd_t cmd = {};
                skipped_tokens = convert_tokens2nonstack_logic(tokens, i, jit_code);
                $
                if (skipped_tokens)
                        continue;
                $
                x86_cmd_ctor(&cmd, tokens->tokens + i);
                paste_cmd_in_jit_buf(jit_code, &cmd);
                i++;
        }
        return 0;
}

#include "../include/consts_x86.cmds"

#define INSERT_x86_CMD(cmds, indent, x86_cmd, value_ptr)                                                               \
                do {                                                                                                        \
                        cmds[indent] = x86_cmd;                                                                             \
                        memcpy(cmds[indent].cmd + cmds[indent].length, value_ptr, sizeof(int));           /*mistake?*/\
                        cmds[indent].length += 4;                                                                           \
                } while(0)

size_t convert_tokens2nonstack_logic (tokens_t *tokens, size_t n_token, jit_code_t *jit_code)
{
        assert(tokens);
        assert(jit_code);

        size_t indent = n_token;
        uint8_t n_new_cmds = 0;

        x86_cmd_t cmds[10] = {};

        if (tokens->tokens[n_token].my_cmd == CMD_MY_PUSH && tokens->tokens[n_token + 1].my_cmd == CMD_MY_PUSH) {
                switch (tokens->tokens[n_token + 2].my_cmd) {
                case CMD_MY_ADD:
                if (tokens->tokens[n_token + 0].use_immed &&
                    tokens->tokens[n_token + 1].use_immed) {
                        INSERT_x86_CMD(cmds, 0, mov_eax, &tokens->tokens[n_token++].immed);
                        INSERT_x86_CMD(cmds, 1, add_rax, &tokens->tokens[n_token++].immed);

                        cmds[2] = push_rax;
                        n_token++;
                        n_new_cmds = 3;
                } else {
                        n_new_cmds = insert_add_sub_mul2reg(CMD_MY_ADD, cmds, tokens, n_token);
                        n_token += 3;
                        n_new_cmds += 3;
                }
                        break;
                case CMD_MY_SUB:
                        if (tokens->tokens[n_token + 0].use_immed &&
                            tokens->tokens[n_token + 1].use_immed) {
                                INSERT_x86_CMD(cmds, 0, mov_eax, &tokens->tokens[n_token++].immed);
                                INSERT_x86_CMD(cmds, 1, sub_rax, &tokens->tokens[n_token++].immed);
                                cmds[2] = push_rax;
                                n_token++;
                                n_new_cmds = 3;
                        } else {
                                n_new_cmds = insert_add_sub_mul2reg(CMD_MY_SUB, cmds, tokens, n_token);
                                n_token += 3;
                                n_new_cmds += 3;
                        }
                        break;
                case CMD_MY_MUL:
                         if (tokens->tokens[n_token + 0].use_immed &&
                             tokens->tokens[n_token + 1].use_immed) {
                                INSERT_x86_CMD(cmds, 0, mov_eax, &tokens->tokens[n_token++].immed);
                                INSERT_x86_CMD(cmds, 1, mov_ebx, &tokens->tokens[n_token++].immed);

                                cmds[2] = mul_rdi;
                                n_token++;

                                cmds[3] = push_rax;
                                n_token++;
                                n_new_cmds = 3;
                        } else {
                                n_new_cmds = insert_add_sub_mul2reg(CMD_MY_MUL, cmds, tokens, n_token);
                                n_token += 3;
                                n_new_cmds += 3;
                        }
                        break;
                default:
                        $
                        return 0;
                }
        } else if (tokens->tokens[n_token].my_cmd == CMD_MY_PUSH) {
                if (tokens->tokens[n_token].my_cmd == CMD_MY_ADD ||
                    tokens->tokens[n_token].my_cmd == CMD_MY_SUB ||
                    tokens->tokens[n_token].my_cmd == CMD_MY_MUL ||
                    tokens->tokens[n_token].my_cmd == CMD_MY_DIV) {

                        cmds[0] = pop_rax;
                        INSERT_x86_CMD(cmds, 1, mov_edi, &tokens->tokens[n_token++].immed);

                        switch (tokens->tokens[n_token].my_cmd) {
                        case CMD_MY_ADD:
                                cmds[2] = add_rax_rdi;
                                break;
                        case CMD_MY_SUB:
                                cmds[2] = sub_rax_rdi;
                                break;
                        case CMD_MY_MUL:
                                cmds[2] = mul_rdi;
                                break;
                        case CMD_MY_DIV:
                                cmds[2] = div_rdi;
                                break;
                        default:
                                log_error(1, "Smth went too wrong.");
                        }
                        cmds[3] = push_rax;
                        n_token++;
                        n_new_cmds = 4;
                }
        } else {
                if (tokens->tokens[n_token].my_cmd == CMD_MY_ADD ||
                    tokens->tokens[n_token].my_cmd == CMD_MY_SUB ||
                    tokens->tokens[n_token].my_cmd == CMD_MY_MUL ||
                    tokens->tokens[n_token].my_cmd == CMD_MY_DIV) {

                        cmds[0] = pop_rdi;
                        cmds[1] = pop_rax;

                        switch (tokens->tokens[n_token].my_cmd) {
                        case CMD_MY_ADD:
                                cmds[2] = add_rax_rdi;
                                break;
                        case CMD_MY_SUB:
                                cmds[2] = sub_rax_rdi;
                                break;
                        case CMD_MY_MUL:
                                $
                                cmds[2] = mul_rdi;
                                break;
                        case CMD_MY_DIV:
                                cmds[2] = div_rdi;
                                break;
                        default:
                                log_error(1, "Smth went too wrong.");
                        }
                        cmds[3] = push_rax;
                        n_token++;
                        n_new_cmds = 4;
                }
        }

        indent = n_token - indent;

        if (indent) {
                for (uint8_t i = 0; i < n_new_cmds; i++) {
                        paste_cmd_in_jit_buf(jit_code, cmds + i);
                }
        }

        return indent;
}

void paste_cmd_in_jit_buf (jit_code_t *jit_code, x86_cmd_t *cmd)
{
        assert(jit_code);
        assert(cmd);

        for (uint8_t i = 0; i < cmd->length; i++) {
        $
                jit_code->buf[jit_code->size] = cmd->cmd[i];
                jit_code->size++;
        }
}

int x86_cmd_ctor (x86_cmd_t *cmd, token_t *token)
{
        assert(cmd);
        assert(token);

        for (int i = 0; i < N_COMMANDS; i++) {
                if (token->my_cmd == cmds_table[i].my_incode) {
                        assemble_cmd(cmd, token, i);
                }
        }
        return 0;
}

void assemble_cmd (x86_cmd_t *cmd, token_t *token, size_t table_position)
{
        assert(cmd);
        assert(token);

        if (cmds_table[table_position].code2 == 0) {
                if (cmds_table[table_position].code1 == ADD ||
                    cmds_table[table_position].code1 == SUB ||
                    cmds_table[table_position].code1 == MUL )
                        incode_add_sub_mul(cmd, token, table_position);
                $
        } else {
                if (cmds_table[table_position].code1 == IMMED_PUSH ||
                    cmds_table[table_position].code1 == REG_PUSH_POP) {
                        incode_push_pop(cmd, token, table_position);
                }
        }
}

void incode_add_sub_mul (x86_cmd_t *cmd, token_t *token, size_t table_position)
{
        assert(cmd);
        assert(token);

        uint8_t indent = 0;

        cmd->cmd[indent++] = mov_eax.cmd[0];
        cmd->cmd[indent++] = mov_edi.cmd[0];
        cmd->length = mov_eax.length + mov_edi.length;

        switch (cmds_table[table_position].code1) {
        case ADD:
                cmd->cmd[indent++] = add_rax_rdi.cmd[0];
                cmd->cmd[indent++] = add_rax_rdi.cmd[1];
                cmd->cmd[indent++] = add_rax_rdi.cmd[2];
                cmd->length += add_rax_rdi.length;
                break;
        case SUB:
                cmd->cmd[indent++] = sub_rax_rdi.cmd[0];
                cmd->cmd[indent++] = sub_rax_rdi.cmd[1];
                cmd->cmd[indent++] = sub_rax_rdi.cmd[2];
                cmd->length += add_rax_rdi.length;
                break;
        case MUL:
                cmd->cmd[indent++] = mul_rdi.cmd[0];
                cmd->cmd[indent++] = mul_rdi.cmd[1];
                cmd->cmd[indent++] = mul_rdi.cmd[2];
                cmd->length += add_rax_rdi.length;
                break;
        default:
                log_error(1, "UNKNOWN COMMAND!");
        }
}

void incode_push_pop (x86_cmd_t *cmd, token_t *token, size_t table_position)
{
        assert(cmd);
        assert(token);

        uint8_t offset = 0;

        if (token->use_r8plus_regs) {
                cmd->cmd[offset] = USE_R_REGS;
                offset++;
        }

        if (token->use_immed) {
                cmd->cmd[offset] = IMMED_PUSH;                       /*0000 0110*/
                cmd->cmd[offset] = cmd->cmd[0] << 4;                 /*0110 0000*/

                cmd->cmd[offset] |= IMMED_PUSH_MRR_MASK;             /*0110 1010*/
                if (token->immed <= ASCII_MAX_SYMBOL)
                        cmd->cmd[offset] |= 2;

                memcpy(cmd->cmd + 1, &(token->immed), get_sizeof_number2write((size_t) token->immed));

                cmd->length = 1 + get_sizeof_number2write((size_t) token->immed);                  /* 1 byte for cmd incode and 4 for immed number */
        } else if (token->mode == MODE_REG_ADDRESS) {
                if (cmds_table[table_position].code1 == IMMED_PUSH) {
                        cmd->cmd[offset] = MEM_REG_PUSH;
                        cmd->cmd[offset + 1]  = IMMED_PUSH << 3;
                } else {
                        cmd->cmd[offset] = MEM_REG_POP;
                }

                cmd->cmd[offset + 1] |= token->reg;

                cmd->length = 2 + offset;                        /* 1 for MEM_REG_PUSH, 1 for other incoding*/
        } else if (token->mode == MODE_8_BYTE_IN_ADDRESS) {
                cmd->cmd[offset] = REG_PUSH_POP << 5;           /*010 00 000*/
                if (cmds_table[table_position].code1 == IMMED_PUSH)
                        cmd->cmd[offset] |= MODE_8_BYTE_IN_ADDRESS << 3; /*010 10 000*/
                else
                        cmd->cmd[offset] |= MODE_REG_ADDRESS << 3; /*010 10 000*/

                cmd->cmd[offset] |= token->reg;

                cmd->length = 1 + offset;                        /*1 for incoding*/
        } else if (token->delete_stack_elem) {
                *cmd = add_rsp_8;
        } else {
                log_error(2, "UNKNOWN PUSH/POP: %d", token->my_cmd);
        }
}

uint8_t insert_add_sub_mul2reg (uint8_t my_cmd, x86_cmd_t *cmds, tokens_t *tokens, size_t position)
{
        assert(cmds);
        assert(tokens);

        int immed = 0;
        uint8_t reg  = 0;

        uint8_t cmd_incode   = 0;

        if (my_cmd == CMD_MY_ADD) {
                cmd_incode = ADD;
        } else if (my_cmd == CMD_MY_SUB) {
                cmd_incode = SUB;
        } else if (my_cmd == CMD_MY_MUL) {
                cmd_incode = MUL;
        }

        if (tokens->tokens[position + 0].mode == MODE_8_BYTE_IN_ADDRESS &&
            tokens->tokens[position + 1].mode == MODE_8_BYTE_IN_ADDRESS) {
                cmds[0].cmd[0]  = ADD_64bit_PREFIX;
                cmds[0].cmd[1]  = cmd_incode | tokens->tokens[position].s;

                cmds[0].cmd[2]  = MODE_REG_ADDRESS << 6;
                cmds[0].cmd[2] |= (uint8_t) tokens->tokens[position + 0].reg << 3;
                cmds[0].cmd[2] |= tokens->tokens[position + 1].reg;

                cmds[0].length = 3;

                x86_cmd_ctor(cmds + 1, tokens->tokens + position);
        } else {
                uint8_t offset = 0;
                uint8_t indent = 0;

                if (tokens->tokens[position].use_immed) {
                        immed = tokens->tokens[position].immed;
                        reg = tokens->tokens[position + 1].reg;
                        offset++;
                } else {
                        immed = tokens->tokens[position + 1].immed;
                        reg = tokens->tokens[position].reg;
                }

                if (reg == RBX)
                        indent = 1;

                cmds[indent].cmd[0] = ADD_64bit_PREFIX;
                cmds[indent].cmd[1] = cmd_incode | (1 << 7) | tokens->tokens[position].s | (tokens->tokens[position].dest << 1);

                cmds[indent].cmd[2]  = MODE_REG_ADDRESS << 6;
                if (reg == RBX) {
                        cmds[0] = mov_rdi_rbx;
                        cmds[indent].cmd[2] |= RDI;

                        tokens->tokens[position + offset].reg = RDI;
                } else {
                        cmds[indent].cmd[2] |= reg;
                }
                memcpy(cmds[indent].cmd + 3, &immed, get_sizeof_number2write((size_t) immed));
                cmds[indent].length = 3 + get_sizeof_number2write((size_t) immed);

                x86_cmd_ctor(cmds + 1 + indent, tokens->tokens + position + offset);

                return indent;
        }

        return 0;
}

// void assemble_mov (x86_cmd_t *cmd, tokens_t *tokens, size_t position)
// {
//         assert(tokens);
//
//         $
// }

uint8_t get_sizeof_number2write (size_t number)
{
        if (number <= ASCII_MAX_SYMBOL)
                return 1;
        else
                return sizeof(int);
}
