#include <stdio.h>
#include <assert.h>

#include "../include/utils.h"

static const char *HELP_MESSAGE = "Available flags:\n"
                                  "\t\t-h - prints help message.\n";

void set_red_in_terminal ()
{
        fprintf(stderr, "\033[1;31m");
}

void set_blue_in_terminal ()
{
        fprintf(stderr, "\033[0;34m");
}

void reset_colour_in_terminal ()
{
        fprintf(stderr, "\033[0m");
}

int check_arguments (int argc, char *argv[], int *n_file_name_arg)
{
        assert(argv);

        if (argc == 1) {
                set_red_in_terminal();
                fprintf(stderr, "Fatal error:");
                reset_colour_in_terminal();
                fprintf(stderr, "No input files!");

                return NO_INPUT_FILE;
        }

        char compilation_flags = 0;
        for (int i = 1; i < argc; i++) {
                if (*argv[i] == '-') {
                        compilation_flags |= check_compilation_flag(argv[i]);
                } else {
                        *n_file_name_arg = i;
                }
        }
        return compilation_flags;
}

#pragma GCC diagnostic ignored "-Wformat-security"

char check_compilation_flag (char *flag)
{
        assert(flag);

        if (flag[1] == 'h') {
                fprintf(stderr, HELP_MESSAGE);
                return 0;
        } else {
                set_red_in_terminal();
                fprintf(stderr, "Fatal error:");
                reset_colour_in_terminal();
                fprintf(stderr, "Unknown compilation flag!\n");
        }

        return 0;
}

uint8_t get_sizeof_number2write (size_t number)
{
        if (number <= ASCII_MAX_SYMBOL)
                return 1;
        else
                return sizeof(int32_t);
}
