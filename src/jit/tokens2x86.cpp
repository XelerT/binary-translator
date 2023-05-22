#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../include/config.h"
#include "../include/tokens.h"
#include "../include/jit.h"
#include "../include/tokens2x86.h"
#include "../include/myIO.h"

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

void insert_nops (jit_code_t *jit_code, size_t amount2insert)
{
        assert(jit_code);

        for (size_t i = jit_code->size; i < amount2insert; i++) {
                jit_code->buf[i] = nop.cmd[0];
                jit_code->size++;
        }
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

void incode_mov (x86_cmd_t *cmd, cmd_info4incode_t *info)
{
        assert(cmd);

        uint8_t indent = 0;

        if (info->dest_reg > RDI && info->dest_reg != INVALID_REG) {
                cmd->cmd[0] = USE_R_REGS;
                info->dest_reg -= R8;
        }
        if (info->src_reg > RDI && info->src_reg != INVALID_REG) {
                cmd->cmd[0] = USE_SRC_R_REG;
                info->src_reg -= R8;
        }
        if (cmd->cmd[0])
                indent++;

        if (info->dest_reg != INVALID_REG && info->src_reg != INVALID_REG) {
                cmd->cmd[indent++] = REG_MOV;

                cmd->cmd[indent]  = MODE_REG_ADDRESS << 6;
                if (info->use_memory4src) {
                        uint8_t swap = info->dest_reg;
                        info->dest_reg = info->src_reg;
                        info->src_reg = swap;
                }

                cmd->cmd[indent] |= info->dest_reg;
                cmd->cmd[indent] |= info->src_reg << 3;

                cmd->length = indent + 1;
        } else if (info->dest_reg != INVALID_REG && info->src_reg == INVALID_REG) {
                if (info->immed_val < (uint32_t) -1) {
                        if (info->use_memory4dest)
                                cmd->cmd[indent++] = MEM_IMMED_MOV;
                        else
                                cmd->cmd[indent]  = IMMED_MOV;

                        cmd->cmd[indent] |= info->dest_reg;
                        memcpy(cmd->cmd + 1 + indent, &info->immed_val, sizeof(uint32_t));

                        cmd->length = 1 + indent + (uint8_t) sizeof(uint32_t);
                } else {
                        cmd->cmd[indent++] |= x64bit_PREFIX;

                        // if (info->use_memory4dest)
                        //         cmd->cmd[indent] = MEM_IMMED_MOV;
                        // else
                                cmd->cmd[indent]  = IMMED_MOV;

                        cmd->cmd[indent]   = IMMED_MOV;
                        cmd->cmd[indent]  |= info->dest_reg;
                        memcpy(cmd->cmd + 2, &info->immed_val, sizeof(uint32_t));

                        cmd->length = 2 + sizeof(uint32_t);
                }
        }
        if (info->use_memory4dest && info->use_memory4src) {
                set_red_in_terminal();
                fprintf(stderr, "Can't use memory as destination and source!");
                reset_colour_in_terminal();

                log_error(1, "Can't use memory as destination and source!");

                return;
        } else if (info->use_memory4dest) {
                cmd->cmd[0] |= x64bit_PREFIX;
                // cmd->cmd[indent] = cmd->cmd[indent] & (~MODE_USE_REG >> 2);
        } else if (info->use_memory4src) {
                cmd->cmd[indent - 1] = MEM_REG_MOV;
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

#define INSERT_x86_CMD(cmds, indent, x86_cmd, value_ptr)                                                                    \
                do {                                                                                                        \
                        cmds[indent] = x86_cmd;                                                                             \
                        memcpy(cmds[indent].cmd + cmds[indent].length, value_ptr, sizeof(int));           /*mistake?*/      \
                        cmds[indent].length += 4;                                                                           \
                } while(0)

size_t convert_tokens2nonstack_logic (tokens_t *tokens, size_t n_token, jit_code_t *jit_code, labels_t *label_table)
{
        assert(tokens);
        assert(jit_code);

        size_t indent = n_token;
        uint8_t n_new_cmds = 0;

        x86_cmd_t cmds[10] = {};

        if (tokens->tokens[n_token].my_cmd == CMD_MY_PUSH      && tokens->tokens[n_token + 1].my_cmd == CMD_MY_PUSH &&
            tokens->tokens[n_token].mode   != MODE_REG_ADDRESS && tokens->tokens[n_token + 1].mode   != MODE_REG_ADDRESS) {
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
                        n_new_cmds = insert_add_sub_mul_div2reg(CMD_MY_ADD, cmds, tokens, n_token, label_table);
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
                                n_new_cmds = insert_add_sub_mul_div2reg(CMD_MY_SUB, cmds, tokens, n_token, label_table);
                                n_token += 3;
                                n_new_cmds += 3;
                        }
                        break;
                case CMD_MY_MUL:
                         if (tokens->tokens[n_token + 0].use_immed &&
                             tokens->tokens[n_token + 1].use_immed) {
                                INSERT_x86_CMD(cmds, 0, mov_eax, &tokens->tokens[n_token++].immed);
                                INSERT_x86_CMD(cmds, 1, mov_ebx, &tokens->tokens[n_token++].immed);
/*TODO: add DIV*/
                                cmds[2] = mul_rdi;
                                n_token++;

                                cmds[3] = push_rax;
                                n_token++;
                                n_new_cmds = 3;
                        } else {
                                n_new_cmds = insert_add_sub_mul_div2reg(CMD_MY_MUL, cmds, tokens, n_token, label_table);
                                n_token += 3;
                                n_new_cmds += 3;
                        }
                        break;
                default:
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
                jit_code->buf[jit_code->size] = cmd->cmd[i];
                jit_code->size++;
        }
}

void pre_incode_print_scan_call (x86_cmd_t *cmd, token_t *token, labels_t *label_table)
{
        assert(cmd);
        assert(token);
        assert(label_table);

        if (token->my_cmd == CMD_MY_OUT)
                token->offset = INVALID_PRINT_ADDRESS;
        else
                token->offset = INVALID_SCAN_ADDRESS;

        pre_incode_call(cmd, token);
}

void incode_print (x86_cmd_t *cmds, token_t *token)
{
        assert(cmds);

        cmds[0] = pop_rdi;

        cmd_info4incode_t info = {
                .immed_val = (size_t) print_decimal - (token->space + JMP_LENGTH)
        };
        incode_call(cmds + 1, &info);
}

void incode_scan (x86_cmd_t *cmds, token_t *token)
{
        assert(cmds);

        cmd_info4incode_t info = {
                .immed_val = (size_t) scan_decimal - (token->space + JMP_LENGTH)
        };
        incode_call(cmds, &info);
        cmds[1] = pop_rax;
}

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
                        incode_push_pop(cmds, token, table_position);
                }
        }
}

void incode_ret (x86_cmd_t *cmd)
{
        assert(cmd);

        cmd->cmd[0] = RET;
        cmd->length = 1;
}

uint8_t incode_test (x86_cmd_t *cmds)
{
        assert(cmds);

        cmds[0] = pop_rcx;
        cmds[1] = pop_rdx;
        cmds[2] = test_rcx_rdx;

        return 3;
}

uint8_t incode_cmp (x86_cmd_t *cmds)
{
        assert(cmds);

        cmds[0] = pop_rcx;
        cmds[1] = pop_rdx;
        cmds[2] = cmp_rcx_rdx;

        return 3;
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

void incode_add_sub_mul_div (x86_cmd_t *cmd, cmd_info4incode_t *info)
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

        if (info->use_memory4src) {
                uint8_t swap   = info->dest_reg;
                info->dest_reg = info->src_reg;
                info->src_reg  = swap;
        }
        cmd->cmd[indent++] = info->cmd_incode;

        if (info->dest_reg != INVALID_REG && info->src_reg != INVALID_REG) {
                cmd->cmd[indent]  = (MODE_REG_ADDRESS << 6);
                cmd->cmd[indent] |= (info->src_reg << 3);
                cmd->cmd[indent] |= info->dest_reg;

                cmd->length = indent + 1;
        } else if (info->dest_reg != INVALID_REG && info->src_reg == INVALID_REG) {
                cmd->cmd[indent]  = (MODE_REG_ADDRESS << 6);
                if (info->cmd_incode == SUB || info->cmd_incode == ADD)
                        cmd->cmd[indent - 1] = ADD_SUB_IMMED;
                if (info->cmd_incode == SUB)
                        cmd->cmd[indent] |= (IMMED_SUB_MASK << 3);
                cmd->cmd[indent] |= info->dest_reg;

                if (get_sizeof_number2write(info->immed_val) == 1)
                        cmd->cmd[indent - 1] |= 2;
                memcpy(cmd->cmd + indent + 1, &info->immed_val, get_sizeof_number2write(info->immed_val));

                cmd->length = get_sizeof_number2write(info->immed_val) + indent + 1;
        }

        if (info->use_memory4dest && info->use_memory4src) {
                set_red_in_terminal();
                fprintf(stderr, "Can't use memory as destination and source!");
                reset_colour_in_terminal();

                log_error(1, "Can't use memory as destination and source!");

                return;
        } else if (info->use_memory4dest) {
                cmd->cmd[indent - 1] |= 0x0;
        } else if (info->use_memory4src) {
                cmd->cmd[indent - 1] |= 0x2;    /* xxxxxx|ds <= d = 1, from memory to register*/
        }
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
                if (cmds_table[table_position].code2)
                        cmd->cmd[indent - 1] |= 0xF0;
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

uint8_t insert_add_sub_mul_div2reg (uint8_t my_cmd, x86_cmd_t *cmds, tokens_t *tokens, size_t position, labels_t *label_table)
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
        } else if (my_cmd == CMD_MY_MUL || my_cmd == CMD_MY_DIV) {
                cmd_incode = MUL;
        }

        if (tokens->tokens[position + 0].mode == MODE_8_BYTE_IN_ADDRESS &&
            tokens->tokens[position + 1].mode == MODE_8_BYTE_IN_ADDRESS) {
                cmds[0].cmd[0]  = x64bit_PREFIX;
                cmds[0].cmd[1]  = cmd_incode | tokens->tokens[position].s;

                cmds[0].cmd[2]  = MODE_REG_ADDRESS << 6;
                cmds[0].cmd[2] |= (uint8_t) tokens->tokens[position + 0].reg << 3;
                cmds[0].cmd[2] |= tokens->tokens[position + 1].reg;

                cmds[0].length = 3;

                x86_cmd_ctor(cmds + 1, tokens->tokens + position, label_table);
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

                cmds[indent].cmd[0] = x64bit_PREFIX;
                cmds[indent].cmd[1] = cmd_incode | tokens->tokens[position].s | (tokens->tokens[position].dest << 1);

                if (my_cmd == CMD_MY_DIV)
                        cmds[indent].cmd[1] |= 0xFF;
                else if (my_cmd == CMD_MY_ADD)
                        cmds[indent].cmd[1] |= ADD_REG_REG_MASK;

                cmds[indent].cmd[2]  = MODE_REG_ADDRESS << 6;
                if (reg == RBX) {
                        cmds[0] = mov_rdi_rbx;
                        cmds[indent].cmd[2] |= RDI;

                        tokens->tokens[position + offset].reg = RDI;
                        immed *= sizeof(size_t);
                } else {
                        cmds[indent].cmd[2] |= reg;
                }
                memcpy(cmds[indent].cmd + 3, &immed, get_sizeof_number2write((size_t) immed));
                cmds[indent].length = 3 + get_sizeof_number2write((size_t) immed);

                x86_cmd_ctor(cmds + 1 + indent, tokens->tokens + position + offset, label_table);

                return indent;
        }

        return 0;
}

uint8_t get_sizeof_number2write (size_t number)
{
        if (number <= ASCII_MAX_SYMBOL)
                return 1;
        else
                return sizeof(int);
}

#undef INSERT_x86_CMD
