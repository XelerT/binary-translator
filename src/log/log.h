#ifndef LOG_H
#define LOG_H

#include "../include/config.h"

#ifdef ASSERT_ON
        #define assert_ptr(ptr) my_assert(ptr, #ptr, __FILE__, __LINE__)
#else
        #define assert_ptr()
#endif /*ASSERT_ON*/

#ifdef LOG_ON
        #define log(num,...)       mlog((const char*)     make_line(num,__VA_ARGS__), __FILE__, __LINE__, "black", 14)
        #define log_pic(num,...)   log_pict((const char*) make_line(num,__VA_ARGS__), __FILE__, __LINE__)
        #define log_error(num,...) mlog((const char*)     make_line(num,__VA_ARGS__), __FILE__, __LINE__, "red", 16)

#else
        #define log(num,...)
        #define log_pic(num,...)
#endif /*LOG_ON*/

#include "../debug/debug.h"

enum log_errors {
        ASSERTION  = 0xACCE,
        CLOSED_LOG = 0xBAD106,
        VBUF_ERR   = 0xBADB0F
};

int   my_assert (const void *ptr, const char *var, const char *file, int line);
int    log_init (const char *log_file_name);
int        mlog (const char *str, const char *file, int line, const char *color, int font_size);
int    log_pict (const char *str, const char *file, int line);
char* make_line (int argc, ...);
void  fill_null (char *str);
int    log_dtor ();

#endif /*LOG_H*/
