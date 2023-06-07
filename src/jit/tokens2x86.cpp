#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../include/config.h"
#include "../include/tokens.h"
#include "../include/jit.h"
#include "../include/translate2x86.h"
#include "../include/tokens2x86.h"
#include "../include/myIO.h"

#include "../include/encode_cmd.h"

#include "../include/consts_x86.cmds"

void insert_nops (jit_code_t *jit_code, size_t amount2insert)
{
        assert(jit_code);

        for (size_t i = jit_code->size; i < amount2insert; i++) {
                jit_code->buf[i] = nop.cmd[0];
                jit_code->size++;
        }
}

size_t find_label (labels_t *label_table, uint32_t my_offset)
{
        assert(label_table);

        for (size_t i = 0; i < label_table->size; i++) {
                if (label_table->labels[i].my_address == my_offset)
                        return i;
        }

        return label_table->size + 1;
}

size_t convert_tokens2nonstack_logic (tokens_t *tokens, size_t n_token, jit_code_t *jit_code)
{
        assert(tokens);
        assert(jit_code);

        size_t prev_jit_code_size = jit_code->size;
        size_t prev_n_token = n_token;

        find_convert_memory_access(jit_code, tokens, n_token);
        if (jit_code->size - prev_jit_code_size)
                return 5;

        token_t *token0 = tokens->tokens + n_token;
        token_t *token1 = tokens->tokens + n_token + 1;
        token_t *token2 = tokens->tokens + n_token + 2;

        if ((token0->use_immed && token1->use_immed) ||
             token0->my_cmd == CMD_MY_POP)
                return 0;

        x86_cmd_t cmds[5] = {};
        cmd_info4encode_t cmd_info = {};

        switch (token0->my_cmd) {
        case CMD_MY_ADD:
        case CMD_MY_SUB:
        case CMD_MY_MUL:
        case CMD_MY_DIV:
                cmd_info.cmd_encode = POP;
                cmd_info.dest_reg = RDI;
                encode_cmd(cmds + 0, &cmd_info);

                cmd_info.dest_reg = RAX;
                encode_cmd(cmds + 1, &cmd_info);

                cmd_info.cmd_encode = get_cmd_from_token(token0);
                if (cmd_info.cmd_encode == MUL || cmd_info.cmd_encode == DIV) {
                        cmd_info.src_reg = INVALID_REG;
                        cmd_info.dest_reg = RDI;
                } else {
                        cmd_info.src_reg  = RDI;
                        cmd_info.dest_reg = RAX;
                }
                encode_cmd(cmds + 2, &cmd_info);

                cmd_info.dest_reg = INVALID_REG;
                cmd_info.src_reg  = RAX;
                cmd_info.cmd_encode = PUSH;
                encode_cmd(cmds + 3, &cmd_info);

                n_token++;
                break;
        default:
                log(1, "");
        }
        if (token0->my_cmd != CMD_MY_PUSH) {
                write_cmds_in_jit_code(jit_code, cmds, 5);
                return n_token - prev_n_token;
        }

        if (!(n_token - prev_n_token)) {
                switch (token1->my_cmd) {
                case CMD_MY_ADD:
                case CMD_MY_SUB:
                case CMD_MY_MUL:
                case CMD_MY_DIV:
                        cmd_info.dest_reg = RAX;
                        cmd_info.cmd_encode = POP;
                        encode_cmd(cmds + 0, &cmd_info);

                        cmd_info.src_reg    = token0->reg;
                        cmd_info.immed_val  = token0->immed;
                        cmd_info.dest_reg   = RAX;
                        cmd_info.cmd_encode = get_cmd_from_token(token1);

                        // if (cmd_info.cmd_encode == MUL || cmd_info.cmd_encode == DIV) {
                        //         cmd_info.dest_reg = INVALID_REG;
                        //         if (cmd_info.src_reg == INVALID_REG)
                        //                 return 0;
                        // }
                        encode_cmd(cmds + 1, &cmd_info);

                        cmd_info.dest_reg = INVALID_REG;
                        cmd_info.src_reg  = RAX;
                        cmd_info.cmd_encode = PUSH;
                        encode_cmd(cmds + 2, &cmd_info);

                        n_token += 2;
                        break;
                default:
                        log(1, "");
                }
        }
        if (token0->my_cmd != CMD_MY_PUSH && token1->my_cmd != CMD_MY_PUSH) {
                write_cmds_in_jit_code(jit_code, cmds, 5);
                return n_token - prev_n_token;
        }

        if (!(n_token - prev_n_token)) {
                switch (token2->my_cmd) {
                case CMD_MY_ADD:
                case CMD_MY_SUB:
                case CMD_MY_MUL:
                case CMD_MY_DIV:
                        cmd_info.dest_reg  = RAX;
                        cmd_info.src_reg   = token0->reg;
                        cmd_info.immed_val = token0->immed;
                        if (token0->mode == MODE_REG_ADDRESS)
                                cmd_info.use_memory4src = (bool) token0->mode;

                        cmd_info.cmd_encode = MOV;
                        encode_cmd(cmds + 0, &cmd_info);

                        cmd_info.use_memory4src = 0;
                        cmd_info.dest_reg   = RAX;
                        cmd_info.src_reg    = token1->reg;
                        cmd_info.immed_val  = token1->immed;
                        if (token0->reg == RBX)
                                cmd_info.immed_val *= 8;
                        cmd_info.cmd_encode = get_cmd_from_token(token2);

                        encode_cmd(cmds + 1, &cmd_info);

                        cmd_info.dest_reg   = INVALID_REG;
                        cmd_info.src_reg    = RAX;
                        cmd_info.cmd_encode = PUSH;
                        encode_cmd(cmds + 2, &cmd_info);                        // encode_pop_push(cmds + 2, &cmd_info);

                        n_token += 3;
                        break;
                default:
                        log(1, "");
                }
        }

        write_cmds_in_jit_code(jit_code, cmds, 5);
        return n_token - prev_n_token;
}

void write_cmds_in_jit_code (jit_code_t *jit_code, x86_cmd_t *cmds, uint8_t max_cmds2write)
{
        assert(jit_code);
        assert(cmds);

        for (uint8_t i = 0; cmds[i].length != 0 && i < max_cmds2write; i++) {
                paste_cmd_in_jit_buf(jit_code, cmds + i);
        }
}

uint8_t get_cmd_from_token (token_t *token)
{
        assert(token);

        switch (token->my_cmd) {
        case CMD_MY_ADD:
                return ADD;
        case CMD_MY_SUB:
                return SUB;
        case CMD_MY_MUL:
                return MUL;
        case CMD_MY_DIV:
                return DIV;
        default:
                log_error(1, "UNKNOWN COMMAND!!");
        }
        return 0;
}

void change_return_value_src (jit_code_t *jit_code, tokens_t *tokens, size_t n_token)
{
        assert(jit_code);
        assert(tokens);

        if (tokens->tokens[n_token + 1].my_cmd == CMD_MY_RET)
                if (tokens->tokens[n_token].my_cmd == CMD_MY_PUSH) {
                        cmd_info4encode_t cmd_info = {
                                .cmd_encode = MOV,
                                .dest_reg   = RAX,
                                .src_reg    = RDX
                        };
                        x86_cmd_t cmd = {};
                        encode_cmd(&cmd, &cmd_info);
                        paste_cmd_in_jit_buf(jit_code, &cmd);
                }
}

void find_convert_memory_access (jit_code_t *jit_code, tokens_t *tokens, size_t n_token)
{
        assert(jit_code);
        assert(tokens);

        token_t *token0 = tokens->tokens + n_token;
        token_t *token1 = tokens->tokens + n_token + 1;
        token_t *token2 = tokens->tokens + n_token + 2;
        token_t *token3 = tokens->tokens + n_token + 3;
        token_t *token4 = tokens->tokens + n_token + 4;

        cmd_info4encode_t mov_rdi_rbx = {
                .cmd_encode = MOV,
                .dest_reg   = RDI,
                .src_reg    = RBX
        };
        cmd_info4encode_t add_rdi = {
                .cmd_encode = ADD,
                .dest_reg = RDI
        };
        cmd_info4encode_t push_rdi = {
                .cmd_encode = PUSH,
                .src_reg = RDI
        };
        cmd_info4encode_t info_pop_rcx = {
                .cmd_encode = POP,
                .dest_reg = RCX
        };
        cmd_info4encode_t cmd_info = {};

        x86_cmd_t cmds[5] = {};

        if (token0->my_cmd == CMD_MY_PUSH && token0->reg == RBX) {
                if (!token1->use_immed && !token2->use_immed)
                        return;
                if ((token3->my_cmd != CMD_MY_PUSH && token3->my_cmd != CMD_MY_POP) || token3->reg != RCX)
                        return;
                if ((token4->my_cmd != CMD_MY_PUSH && token4->my_cmd != CMD_MY_POP) || token4->reg != RCX)
                        return;

                if (token1->my_cmd == CMD_MY_PUSH && token2->my_cmd == CMD_MY_ADD)
                        add_rdi.immed_val = token1->immed * 8;
                else if (token2->my_cmd == CMD_MY_PUSH && token1->my_cmd == CMD_MY_ADD)
                        add_rdi.immed_val = token2->immed * 8;

                encode_cmd(cmds + 0, &mov_rdi_rbx);
                encode_cmd(cmds + 1, &add_rdi);
                encode_cmd(cmds + 2, &push_rdi);
                encode_cmd(cmds + 3, &info_pop_rcx);

                if (token4->my_cmd == CMD_MY_PUSH) {
                        cmd_info.cmd_encode = PUSH;
                        cmd_info.src_reg = RCX;
                        cmd_info.use_memory4src = 1;
                } else {
                        cmd_info.cmd_encode = POP;
                        cmd_info.dest_reg = RCX;
                        cmd_info.use_memory4dest = 1;
                }
                encode_cmd(cmds + 4, &cmd_info);

                for (int i = 0; i < 5; i++)
                        paste_cmd_in_jit_buf(jit_code, cmds + i);
        }

}

void pre_encode_print_scan_call (x86_cmd_t *cmd, token_t *token, labels_t *label_table)
{
        assert(cmd);
        assert(token);
        assert(label_table);

        if (token->my_cmd == CMD_MY_OUT)
                token->offset = INVALID_PRINT_ADDRESS;
        else
                token->offset = INVALID_SCAN_ADDRESS;

        pre_encode_call(cmd, token);
}

void encode_print (x86_cmd_t *cmds, token_t *token)
{
        assert(cmds);

        cmd_info4encode_t pop_rdi_info = {
                .cmd_encode = POP,
                .dest_reg = RDI
        };
        encode_cmd(cmds + 0, &pop_rdi_info);

        cmd_info4encode_t info = {
                .cmd_encode = CALL,
                .immed_val = (int) ((size_t) print_decimal - (token->space + JMP_LENGTH))
        };
        encode_cmd(cmds + 1, &info);
}

void encode_scan (x86_cmd_t *cmds, token_t *token)
{
        assert(cmds);

        cmd_info4encode_t info = {
                .cmd_encode = CALL,
                .immed_val = (int) ((size_t) scan_decimal - (token->space + JMP_LENGTH))
        };
        encode_cmd(cmds + 0, &info);

        cmd_info4encode_t pop_rax_info = {
                .cmd_encode = POP,
                .dest_reg = RAX
        };
        encode_cmd(cmds + 1, &pop_rax_info);
}

void insert_label (jit_code_t *jit_code, token_t *token, labels_t *label_table)
{
        assert(jit_code);
        assert(token);
        assert(label_table);

        label_table->labels[label_table->size].my_address  = token->offset;
        label_table->labels[label_table->size].new_address = jit_code->size;

        label_table->size++;
}

void encode_token2push_pop (x86_cmd_t *cmd, token_t *token, size_t table_position)
{
        assert(cmd);
        assert(token);

        uint8_t offset = 0;

        if (token->use_r8plus_regs) {
                cmd->cmd[offset] = USE_DEST_R_REGS;
                offset++;
        }

        if (token->use_immed) {
                cmd->cmd[offset] = IMMED_PUSH;                       /*0000 0110*/
                cmd->cmd[offset] = cmd->cmd[0] << 4;                 /*0110 0000*/

                cmd->cmd[offset] |= IMMED_PUSH_MRR_MASK;             /*0110 1010*/
                if (token->immed <= ASCII_MAX_SYMBOL)
                        cmd->cmd[offset] |= 2;

                memcpy(cmd->cmd + 1, &(token->immed), get_sizeof_number2write((size_t) token->immed));

                cmd->length = 1 + get_sizeof_number2write((size_t) token->immed);                  /* 1 byte for cmd encode and 4 for immed number */
        } else if (token->mode == MODE_REG_ADDRESS) {
                if (CMDS_TABLE[table_position].code1 == IMMED_PUSH) {
                        cmd->cmd[offset] = MEM_REG_PUSH;
                        cmd->cmd[offset + 1]  = IMMED_PUSH << 3;
                } else {
                        cmd->cmd[offset] = MEM_REG_POP;
                }
                cmd->cmd[offset + 1] |= token->reg;

                cmd->length = 2 + offset;                        /* 1 for MEM_REG_PUSH, 1 for other incoding*/
        } else if (token->mode == MODE_8_BYTE_IN_ADDRESS) {
                cmd->cmd[offset] = REG_PUSH_POP << 5;           /*010 00 000*/
                if (CMDS_TABLE[table_position].code1 == IMMED_PUSH)
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

#undef INSERT_x86_CMD
