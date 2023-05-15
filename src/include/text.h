#ifndef TEXT_H
#define TEXT_H

#include <stdio.h>
#include "config.h"

struct word_t
{
        char *ptr = nullptr;
        size_t length = 0;
};

struct line_t
{
        char *ptr = nullptr;
        size_t length = 0;
};

struct text_t {
        size_t n_words      =       0;
        size_t n_lines      =       0;
        size_t n_chars      =       0;
        char *buf           = nullptr;
        line_t *lines       = nullptr;
        word_t *words       = nullptr;
        char *aligned_words = nullptr;
};

const int MAX_LINE_LENGTH = 512;

enum text_error {
        EMPTY_FILE     = -1,
        NULL_TEXT_PTR  = -2,
        FILE_ERR       = 0xF11E,
        GET_TEXT_ERROR = 0xBAD61
};

int  get_text             (FILE *input, text_t *text, const char *file_name);
void text_dtor            (text_t *text);

void replace_n            (text_t *text);
void delete_punctuations  (text_t *text);
void text_count_words     (text_t *text);

void divide_text_on_lines (text_t *text);
void divide_text_on_words (text_t *text);

char *skip_tabs           (char   *line);

int align_words (text_t *text);

#endif /*TEXT_H*/
