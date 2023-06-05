#ifndef IR_H
#define IR_H

#include "tokens.h"
#include "configs.cmds"

struct tokens_text_t {
        char *buf       = nullptr;
        size_t capacity = 0;
        size_t size     = 0;
};

int save_tokens_in_text        (tokens_t *tokens, const char *file_name);
void parse_memory_indent       (tokens_t *tokens, size_t *n_token, tokens_text_t *tokens_text);
void parse_memory_access_block (tokens_t *tokens, size_t *n_token, tokens_text_t *tokens_text);
void parse_cmd                 (tokens_t *tokens, size_t *n_token, tokens_text_t *tokens_text);
void write_down_cmd            (token_t *token,   int place_in_cmds_table, tokens_text_t *tokens_text);
void write_down_push_pop_args  (token_t *token,   tokens_text_t *tokens_text);

int  write_buf_in_file         (tokens_text_t *tokens_text, const char *file_name);

#endif /*IR_H*/
