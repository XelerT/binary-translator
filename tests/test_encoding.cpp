#include <stdio.h>
#include <cstdlib>
#include <assert.h>

#include "../src/include/config.h"
#include "../src/include/text.h"
#include "../src/include/jit.h"
#include "../src/include/encode_cmd.h"
#include "../src/include/translate2x86.h"

#include "cmd_array.cmds"

void     check_cmds             (x86_cmd_t *cmds, uint8_t *correct_bytes);
uint8_t* get_correct_cmds_array (const char *input_file_name);

static const int TEXT_SECTION_START  = 0x1000;
static const int TEXT_SECTION_LENGTH = 0x2A;

int main ()
{
        log_init("test.html");
        x86_cmd_t *cmds = (x86_cmd_t*) calloc(CMDS_ARRAY_LENGTH, sizeof(x86_cmd_t));

        for (int i = 0; i < CMDS_ARRAY_LENGTH; i++) {
                encode_cmd(cmds + i, cmds_array + i);
                // printf("%d ", i);
                // for (int j = 0; j < cmds[i].length; j++)
                //         printf("%x ", cmds[i].cmd[j]);
                // printf(" \n");
        }

        uint8_t *correct_bytes = get_correct_cmds_array("tests/standart");
        check_cmds(cmds, correct_bytes);

        free(correct_bytes);
        free(cmds);
        log_dtor();
        return 0;
}

void check_cmds (x86_cmd_t *cmds, uint8_t *correct_bytes)
{
        assert(cmds);
        assert(correct_bytes);

        size_t bytes_counter = 0;

        for (int i = 0; i < CMDS_ARRAY_LENGTH; i++) {
                set_green_in_terminal();
                for (uint8_t j = 0; j < cmds[i].length; j++) {
                        if (cmds[i].cmd[j] != correct_bytes[bytes_counter++]) {
                                set_red_in_terminal();
                                fprintf(stderr, "TEST %d NOT PASSED!\n", i);
                                reset_colour_in_terminal();
                                return;
                        }
                }
                fprintf(stderr, "TEST %d PASSED!\n", i);
        }
        reset_colour_in_terminal();
}

uint8_t* get_correct_cmds_array (const char *input_file_name)
{
        assert(input_file_name);

        FILE *input_file = fopen(input_file_name, "r");
        if (!input_file)
                fprintf(stderr, "NULL FILE PTR!\n");
        text_t correct_code = {};

        get_text(input_file, &correct_code, input_file_name);
        uint8_t *correct_bytes = (uint8_t*) calloc(TEXT_SECTION_LENGTH, sizeof(uint8_t));

        for (int j = 0, i = TEXT_SECTION_START; j < TEXT_SECTION_LENGTH; j++) {
                correct_bytes[j] = correct_code.buf[i++];
                // printf("%x ", correct_bytes[j]);
        }
        text_dtor(&correct_code);
        return correct_bytes;
}

