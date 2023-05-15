#ifndef UTILS_H
#define UTILS_H

const int MAX_NAME_LENGTH = 64;
const int DEFAULT_N_VARS  = 16;
const int DEFAULT_N_FUNCS = 16;
const int DEFAULT_N_TABLS =  4;

const int X86_CMD_MAX_LENGTH = 16;

const int m256_BYTE_CAPACITY = 32;

const int PAGE_SIZE        = 4096;
const int ASCII_MAX_SYMBOL =  127;

struct graph_node_atr_t {
        const char *shape     = "rectangle";
        const char *style     =   "rounded";
        const char *fixedsize =     "false";
        const char *fillcolor =   "#00b899";
        int height    =  3;
        int width     =  2;
        int fontsize  = 30;
        int penwidth  =  5;
        int arrowsize =  2;
};

struct edge_t {
        int penwidth      =       5;
        const char *color = "black";
};

struct digraph_t {
        int dpi             =     150;
        const char *splines = "ortho";
};

enum main_errors {
        NULL_CALLOC   = 0xCA110C,
        NULL_FILE_PTR = 0xF11E,
        ALLOC_ERR     = 0xBADA110C,
        REALLOC_ERR   = 0xBADA110C,
        LEX_ERROR     = 0xBAD1E4,
        NULL_SPRINTF  = 0xBAD366,
        TOO_LONG      = 0x100104,
        NULL_FWRITE   = 0xBADF44,
        NO_ELEMENT    = 0xBADE1E,
        NO_INPUT_FILE = 0xBADF11E
};

void set_red_in_terminal      ();
void set_blue_in_terminal     ();
void reset_colour_in_terminal ();
int  check_arguments          (int argc, char *argv[], int *n_file_name_arg);
char check_compilation_flag   (char *flag);

#endif /*UTILS_H*/
