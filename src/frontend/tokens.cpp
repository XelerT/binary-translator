#include <stdio.h>
#include <cstdlib>
#include <assert.h>

#include "../include/tokens.h"
#include "../include/tokens2x86.h"

int parse_my_binary_file (tokens_t *tokens, const char *input_file_name)
{
        assert(input_file_name);

        FILE *input_file = fopen(input_file_name, "r");
        if (!input_file) {
                log(2, "Null file pointer for %s file", input_file_name);
                return NULL_FILE_PTR;
        }

        text_t bin_code = {};
        if (get_text(input_file, &bin_code, input_file_name))
                return GET_TEXT_ERROR;

        tokens_ctor(tokens, bin_code.n_chars / 2);
        if (parse_text_for_tokens(&bin_code, tokens))
                return PARSE_TOKENS_ERROR;

        return 0;
}

int tokens_ctor (tokens_t *tokens, size_t capacity)
{
        assert(tokens);

        tokens->tokens = (token_t*) calloc(capacity, sizeof(token_t));
        if (!tokens->tokens) {
                log(1, "Null callo for tokens.");
                return NULL_CALLOC;
        }
        tokens->capacity = capacity;

        return 0;
}

void tokens_dtor (tokens_t *tokens)
{
        assert(tokens);

        free(tokens->tokens);
        tokens->tokens = nullptr;
}

int parse_text_for_tokens (text_t *bin_code, tokens_t *tokens)
{
        assert(bin_code);
        assert(tokens);

        unsigned int cmd = 0;
        unsigned char n_skipped_chars = 0;

        for (size_t i = 0; i < bin_code->n_chars; i++) {
                sscanf(bin_code->buf + i, "%d%hhn", &cmd, &n_skipped_chars);
                tokens->n_parsed_numbers++;
                i += (size_t) n_skipped_chars;

                i += init_token(tokens, bin_code, cmd, i);
        }

        return 0;
}

//CMD(cmd_name_str, number_for_mycpu, real_doesn't_exist, has_arg)
#define CMD(name, number, doesnt_exist, has_arg)                                                                 \
                case number:                                                                                     \
                        if (doesnt_exist)                                                                        \
                                return 0;                                                                        \
                        if (has_arg) {                                                                           \
                                sscanf(bin_code->buf + ip, "%d%hhn", &arg, &n_skipped_chars);                    \
                                tokens->n_parsed_numbers++;                                                      \
                                insert_token_args(tokens, cmd, arg);                                             \
                        } else {                                                                                 \
                                tokens->tokens[tokens->size].my_cmd = number;                                    \
                                if (number == CMD_MY_LABEL)                                                      \
                                        tokens->tokens[tokens->size].offset = (uint32_t) tokens->n_parsed_numbers - 1;          \
                        }                                                                                        \
                        break;

size_t init_token (tokens_t *tokens, text_t *bin_code, unsigned int cmd, size_t ip)
{
        assert(tokens);
        assert(bin_code);

        int arg = 0;
        unsigned char n_skipped_chars = 0;

        switch (cmd & CMD_MASK) {
#include "../include/list.my_cmds"
        default:
                log_error(2, "Unknown command %d!", cmd);
        }

        tokens->size++;

        return n_skipped_chars;
}

void insert_token_args (tokens_t *tokens, unsigned int cmd, int arg)
{
        assert(tokens);

        token_t *token = tokens->tokens + tokens->size;

        token->my_cmd = cmd & CMD_MASK;

        switch (cmd & CMD_MASK) {
        case CMD_MY_PUSH:
                if (cmd & ARG_RAM_MASK) {
                        if (cmd & ARG_REG_MASK) {
                                token->dest = 1;
                                token->mode = 3;

                                token->reg = convert_my_regs2x86_regs(arg);
                                if (arg >= 5)
                                        token->use_r8plus_regs = 1;
                        } else {
                                log_error(2, "SMTH WRONG %d", cmd);
                        }
                } else if (cmd & ARG_REG_MASK) {
                        token->dest = 1;
                        token->mode = 2;

                        token->reg = convert_my_regs2x86_regs(arg);
                        if (arg >= 5)
                                token->use_r8plus_regs = 1;
                } else if (cmd & ARG_IMMED_MASK) {
                        token->immed = arg;
                        token->use_immed = 1;
                } else {
                        log_error(2, "SMTH WRONG %d", cmd);
                }
                token->s = 1;
                break;
        case CMD_MY_POP:
                if (cmd & ARG_RAM_MASK) {
                        if (cmd & ARG_REG_MASK) {
                                token->dest = 0;
                                token->mode = 3;

                                token->reg = convert_my_regs2x86_regs(arg);
                                if (arg >= 5)
                                        token->use_r8plus_regs = 1;
                        } else {
                                log_error(2, "SMTH WRONG %d", cmd);
                        }
                } else if (cmd & ARG_REG_MASK) {
                        token->dest = 0;
                        token->mode = 2;

                        token->reg = convert_my_regs2x86_regs(arg);
                        if (arg >= 5)
                                token->use_r8plus_regs = 1;
                } else if (cmd == CMD_MY_POP) {
                        token->delete_stack_elem = 1;
                } else {
                        log_error(2, "SMTH WRONG %d", cmd);
                }
                break;
        default:                        /*JMPs and call*/
                token->offset = arg;
        }
}


uint8_t convert_my_regs2x86_regs (int my_reg)
{
        const uint8_t my2x86_regs[] = {0, 3, 1, 2, 0, 1, 2, 3};
        return my2x86_regs[my_reg - 1];
}
