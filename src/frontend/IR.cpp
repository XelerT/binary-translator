#include <stdio.h>
#include <cstdlib>
#include <assert.h>

#include "../include/IR.h"

int save_tokens_in_text (tokens_t *tokens, const char *file_name)
{
        assert(tokens);
        assert(file_name);

        tokens_text_t tokens_text = {};
        size_t n_token = 0;

        tokens_text.capacity = tokens->size * AVERAGE_SIZE_OF_TOKEN_IN_CHARS;
        tokens_text.buf = (char*) calloc(tokens_text.capacity, sizeof(char));
        if (!tokens_text.buf) {
                log_error(1, "CALLOC RETURN NULL!");
                return NULL_CALLOC;
        }
        while (n_token < tokens->size) {
                parse_memory_indent(tokens, &n_token, &tokens_text);
                parse_memory_access_block(tokens, &n_token, &tokens_text);
                parse_cmd(tokens, &n_token, &tokens_text);
        }

        return write_buf_in_file(&tokens_text, file_name);
}

void parse_memory_indent (tokens_t *tokens, size_t *n_token, tokens_text_t *tokens_text)
{
        assert(tokens);
        assert(tokens_text);

        token_t *token0 = tokens->tokens + *n_token;
        token_t *token1 = tokens->tokens + *n_token + 1;

        size_t n_written_bytes = 0;

        if (token0->my_cmd == CMD_MY_PUSH && token0->use_immed) {
                if (token1->my_cmd == CMD_MY_POP && token1->reg == RBX) {
                        sprintf(tokens_text->buf + tokens_text->size,
                                "%%setting = memory_indent_rbx(%d)\n\n%ln", token0->immed, &n_written_bytes);
                        tokens_text->size += n_written_bytes;

                        *n_token += 2;
                }
        }
}

void parse_memory_access_block (tokens_t *tokens, size_t *n_token, tokens_text_t *tokens_text)
{
        assert(tokens);
        assert(tokens_text);

        token_t *block_tokens[5] = { tokens->tokens + *n_token,
                                     tokens->tokens + *n_token + 1,
                                     tokens->tokens + *n_token + 2,
                                     tokens->tokens + *n_token + 3,
                                     tokens->tokens + *n_token + 4
                                   };
        size_t n_written_bytes = 0;

        if ( block_tokens[0]->my_cmd == CMD_MY_PUSH &&
             block_tokens[1]->my_cmd == CMD_MY_PUSH &&
             block_tokens[3]->my_cmd == CMD_MY_POP  &&
            (block_tokens[4]->my_cmd == CMD_MY_PUSH ||
             block_tokens[4]->my_cmd == CMD_MY_POP  )) {

                switch (block_tokens[2]->my_cmd) {
                case CMD_MY_ADD:
                case CMD_MY_SUB:
                case CMD_MY_MUL:
                case CMD_MY_DIV:
                        log(1, "May be the block of push/pop [rbx + N]");
                        break;
                default:
                        log(1, "Not the block of push/pop [rbx + N]");
                        return;
                }
        }

        if (block_tokens[0]->reg  == RBX   &&
            block_tokens[3]->reg  == RCX   &&
            block_tokens[4]->reg  == RCX   &&
            block_tokens[1]->use_immed     &&
            block_tokens[4]->mode == MODE_REG_ADDRESS) {

                sprintf(tokens_text->buf + tokens_text->size,
                        "\n%%mem_block = %s(rbx,m,%d)\n\n%ln",
                        (block_tokens[4]->my_cmd == CMD_MY_POP) ? "pop": "push",
                         block_tokens[1]->immed, &n_written_bytes);
                tokens_text->size += n_written_bytes;

                *n_token += 5;
        }
}

void parse_cmd (tokens_t *tokens, size_t *n_token, tokens_text_t *tokens_text)
{
        assert(tokens);
        assert(n_token);
        assert(tokens_text);

        for (int i = 0; i < N_COMMANDS; i++) {
                if (tokens->tokens[*n_token].my_cmd == CMDS_TABLE[i].my_encode) {
                        write_down_cmd(tokens->tokens + *n_token, i, tokens_text);
                        ++*n_token;
                        return;
                }
        }
}

#define WRITE_IN_TOKENS_BUF(format_line, ...)           \
                sprintf(tokens_text->buf + tokens_text->size, format_line, ##__VA_ARGS__)

void write_down_cmd (token_t *token, int place_in_cmds_table, tokens_text_t *tokens_text)
{
        assert(token);
        assert(tokens_text);

        char cmd_str[] = {"%cmd       ="};
        size_t n_written_bytes = 0;

        switch (CMDS_TABLE[place_in_cmds_table].my_encode) {
        case CMD_MY_PUSH:
        case CMD_MY_POP:
                WRITE_IN_TOKENS_BUF("%s %s(%ln",
                                    cmd_str,
                                    CMDS_TABLE[place_in_cmds_table].my_name,
                                    &n_written_bytes);
                tokens_text->size += n_written_bytes;

                write_down_push_pop_args(token, tokens_text);
                WRITE_IN_TOKENS_BUF(")\n%ln", &n_written_bytes);
                tokens_text->size += n_written_bytes;
                break;
        case CMD_MY_ADD:
        case CMD_MY_SUB:
        case CMD_MY_MUL:
        case CMD_MY_DIV:
        case CMD_MY_HLT:
        case CMD_MY_OUT:
        case CMD_MY_RET:
        case CMD_MY_IN:
        case CMD_MY_SQRT:
        case CMD_MY_LABEL:
                WRITE_IN_TOKENS_BUF("%s %s\n%ln", cmd_str,
                                                 CMDS_TABLE[place_in_cmds_table].my_name,
                                                 &n_written_bytes);
                tokens_text->size += n_written_bytes;
                break;
        default:
                WRITE_IN_TOKENS_BUF("%s %s(%d)\n%ln", cmd_str, CMDS_TABLE[place_in_cmds_table].my_name,
                                                      token->offset,
                                                      &n_written_bytes);
                tokens_text->size += n_written_bytes;
        }
}

void write_down_push_pop_args (token_t *token, tokens_text_t *tokens_text)
{
        assert(token);
        assert(tokens_text);

        size_t n_written_bytes = 0;
        uint8_t reg = token->reg;

        if (reg != INVALID_REG) {
                if (token->use_r8plus_regs)
                        WRITE_IN_TOKENS_BUF("r+%d,%ln", reg, &n_written_bytes);
                else
                        WRITE_IN_TOKENS_BUF("r%d,%ln", reg, &n_written_bytes);
                tokens_text->size += n_written_bytes;

                if (token->mode == MODE_REG_ADDRESS) {
                        WRITE_IN_TOKENS_BUF("m,%ln", &n_written_bytes);
                        tokens_text->size += n_written_bytes;
                }
        } else if (token->use_immed) {
                WRITE_IN_TOKENS_BUF(",,%d%ln", token->immed, &n_written_bytes);
                tokens_text->size += n_written_bytes;
        }
}

#undef WRITE_IN_TOKENS_BUF

int write_buf_in_file (tokens_text_t *tokens_text, const char *file_name)
{
        assert(tokens_text);
        assert(file_name);

        FILE *output_file = fopen(file_name, "w");
        if (!output_file) {
                log_error(2, "Can't open file %s for IR", file_name);
                return NULL_FILE_PTR;
        }

        size_t n_written_bytes = fwrite(tokens_text->buf, sizeof(char), tokens_text->size, output_file);
        if (n_written_bytes != tokens_text->size) {
                log_error(1, "FWRITE DIDN'T WRITE ALL ELEMENT OF IR!");
                return NULL_FWRITE;
        }
        fclose(output_file);

        return 0;
}

/*

%setting   = memory_indent_rbx(100)
%mem_block = pop(r,m,12)
%cmd       = push(,,)
%cmd       = jmp(16)
%cmd       = label(123)

%cmd       = hlt
%cmd       = add
%cmd       = sqrt
%cmd       = in/out

*/
