#ifndef MYIO_H
#define MYIO_H

void print_decimal (size_t val_in_rdi);
int  scan_decimal  ();
int  paste_io_decimal_function (jit_code_t *jit_code, labels_t *label_table, int invalid_func_address);

#endif /*MYIO_H*/
