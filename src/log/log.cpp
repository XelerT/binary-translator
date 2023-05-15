#include <stdio.h>
#include <string.h>
#include <stdarg.h>
// #include <assert.h>
#include "log.h"

static FILE *LOG_FILE = nullptr;
static const int MAX_LINE_LENGTH = 512;
static char LOG_LINE[MAX_LINE_LENGTH] = {'\0'};

int log_init (const char *log_file_name)
{
        assert_ptr(log_file_name);

        LOG_FILE = fopen(log_file_name, "w");
        if (setvbuf(LOG_FILE, nullptr, _IONBF, 0))
                return VBUF_ERR;
        fprintf(LOG_FILE, "<pre>\n");
        fprintf(LOG_FILE, "<meta charset=\"utf-8\">\n");
        fprintf(LOG_FILE, "<body style=\"background-color:#B4D1CC;\">\n");

        return 0;
}

int mlog (const char *str, const char *file, int line, const char *color, int font_size)
{
        assert_ptr(str);
        assert_ptr(LOG_FILE);
        assert_ptr(file);

        fprintf(LOG_FILE, "<span style = \"color: gray; font-size:14px;\">{File: %s, Line: %d}"
        " <span style = \"color: %s; font-size:%dpx;\">\t%s.</span></span>\n", file, line, color, font_size, str);

        return 0;
}

int log_pict (const char *str, const char *file, int line)
{
        assert_ptr(str);
        assert_ptr(LOG_FILE);
        assert_ptr(file);

        fprintf(LOG_FILE, "<p style = \"color: gray; font-size:14px;\">{File: %s, Line: %d}</p>\n"
                          " %s\n", file, line, str);

        return 0;
}

int my_assert (const void *ptr, const char *var, const char *file, int line)
{
        if (ptr == nullptr && LOG_FILE != nullptr) {
                fprintf(LOG_FILE, "<p style = \"color: red; font-size:20px;\"> Variable \"%s\""
                                  " caused assert in %s file, on %d line.</p>\n",
                                   var, file, line);
                log_dtor();
                return ASSERTION;
        } else if (ptr == nullptr && LOG_FILE == nullptr) {
                fprintf(stderr, "Variable \"%s\" caused assert in %s file, on %d line.\n",
                                 var, file, line);
                log_dtor();
                return ASSERTION;
        }

        return 0;
}

// void log(const char* format, ...)
//         {
//         va_list args{};
//         va_start(args, format);
//
//         vprintf(format, args);
//         va_end(args);
//
//         return;
//         }
//
char* make_line (int argc, ...)
{
        va_list valist = {};
        va_start(valist, argc);

        char temp_str[MAX_LINE_LENGTH] = {'\0'};
        fill_null(LOG_LINE);
        strcpy(temp_str, va_arg(valist, const char*));
        size_t line_length = strlen((char*) temp_str);

        for (size_t i = 0, j = 0; i < line_length; i++) {
                if (temp_str[i] == '\%') {
                        i++;
                        if (temp_str[i] == 's') {
                                strcpy(LOG_LINE + j, va_arg(valist, const char*));
                                j += strlen(LOG_LINE + j);
                        } else if (temp_str[i] == 'l' && temp_str[i + 1] == 'd') {
                                sprintf(LOG_LINE + j, "%ld", va_arg(valist, size_t));
                                j += strlen(LOG_LINE + j);
                                i += 2;
                        } else if (temp_str[i] == 'l' && temp_str[i + 1] == 'g') {
                                sprintf(LOG_LINE + j, "%lg", va_arg(valist, double));
                                j += strlen(LOG_LINE + j);
                                i+= 1;
                        } else if (temp_str[i] == 'c') {
                                sprintf(LOG_LINE + j, "%c", va_arg(valist, int));
                                j += strlen(LOG_LINE + j);
                        } else if (temp_str[i] == 'd') {
                                sprintf(LOG_LINE + j, "%d", va_arg(valist, int));
                                j += strlen(LOG_LINE + j);
                        } else {
                                sprintf(LOG_LINE + j, "/unknown sign/");
                                j += strlen(LOG_LINE + j);
                                // i++;
                        }
                } else {
                        LOG_LINE[j++] = temp_str[i];
                }
        }

        va_end(valist);
        return LOG_LINE;
}

void fill_null(char *str)
{
        assert_ptr(str);

        for (int i = 0; str[i] != '\0'; i++) {
                str[i] = '\0';
        }
}

int log_dtor ()
{
        if (LOG_FILE == nullptr) {
                fprintf(stderr, "Log file is already closed.\n");
                return CLOSED_LOG;
        }
        fprintf(LOG_FILE, "</pre>\n");
        fprintf(LOG_FILE, "</body>\n");

        fclose(LOG_FILE);
        return 0;
}
