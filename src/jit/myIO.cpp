#include <assert.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

#include "../include/config.h"
#include "../include/tokens.h"
#include "../include/jit.h"
#include "../include/tokens2x86.h"

#include "../include/myIO.h"

void print_decimal (size_t val_in_rdi)
{
        printf("%zd\n", val_in_rdi);
}

int scan_decimal ()
{
        int val_in_rax = 0;
        scanf("%d", &val_in_rax);

        return val_in_rax;
}

int paste_io_decimal_function (jit_code_t *jit_code, labels_t *label_table, int invalid_func_address)
{
        assert(jit_code);

        char *file_name = nullptr;
        if (invalid_func_address == INVALID_PRINT_ADDRESS)
                file_name = "print.bin";
        else if (invalid_func_address == INVALID_SCAN_ADDRESS)
                file_name = "scan.bin";

        FILE *print_bin = fopen(file_name, "r");
        if (!print_bin) {
                log_error(2, "Can't open %s file", file_name);
                return NULL_FILE_PTR;
        }

        struct stat file = {};
        if (stat("print.bin", &file) < 0)
                return FILE_ERR;

        size_t func_size = fread(jit_code->buf + jit_code->size + jit_code->print_func_offset, sizeof(uint8_t), file.st_size, print_bin);
        if (!func_size) {
                log_error(2, "File %s is empty!", file_name);
                return EMPTY_FILE;
        }

        if (invalid_func_address == INVALID_PRINT_ADDRESS) {
                jit_code->print_func_size = func_size;

                label_table->labels[label_table->size].my_address  = INVALID_PRINT_ADDRESS;
                label_table->labels[label_table->size++].new_address = jit_code->size + jit_code->print_func_offset;
        } else if (invalid_func_address == INVALID_SCAN_ADDRESS) {
                jit_code->scan_func_size = func_size;

                label_table->labels[label_table->size].my_address  = INVALID_SCAN_ADDRESS;
                label_table->labels[label_table->size++].new_address = jit_code->size + jit_code->scan_func_offset;
        }

        return 0;
}
