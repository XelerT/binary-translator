#ifndef TOKENS_H
#define TOKENS_H

#include "config.h"
#include "text.h"

#include "configs.cmds"

struct token_t {
        int  my_cmd = 0;                  /* original code of command for my cpu */
        bool   dest = 0;                  /* 1 - register => reg/memory, 0 - mem => reg */
        bool      s = 1;                  /* Using 64 bit */

        char    my_reg = 0;                  /* number of my reg */
        int      immed = 0;                  /* immediate number in cmd */
        bool use_immed = 0;               /*1 - When immediate number is given*/

        int8_t reg = INVALID_REG;
        bool use_r8plus_regs = 0;         /* if we need to use regs r8+ */

        uint8_t mode = 0;                 /* 0 - use register, 2 - 8 byte in address(FYI: "push rax"), 3 - reg address mode*/

        uint32_t offset = 0;

        bool delete_stack_elem = 0;

        size_t space = 0;
};

struct tokens_t {
        token_t *tokens         = nullptr;
        size_t capacity         = 0;
        size_t size             = 0;
        size_t n_parsed_numbers = 0;
};

enum my_masks {
        ARG_IMMED_MASK = 1 << 5, //0010 0000
        ARG_REG_MASK   = 1 << 6, //0100 0000
        ARG_RAM_MASK   = 1 << 7, //1000 0000

        CMD_MASK  = 0x1F
};

enum tokens_errors {
        PARSE_TOKENS_ERROR = 0xBAD104
};

int parse_my_binary_file (tokens_t *tokens, const char *input_file_name);

int  tokens_ctor (tokens_t *tokens, size_t capacity);
void tokens_dtor (tokens_t *tokens);

int    parse_text_for_tokens (text_t *bin_code, tokens_t *tokens);
size_t init_token            (tokens_t *tokens, text_t *bin_code, unsigned int cmd, size_t ip);
void   insert_token_args     (tokens_t *tokens, unsigned int cmd, int arg);

uint8_t convert_my_regs2x86_regs (int my_reg);

#endif /*TOKENS_H*/
