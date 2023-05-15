#ifndef DEBUG_H
#define DEBUG_H

#include "../include/config.h"


#ifdef DEBUG

#define $ fprintf(stderr, "I'm here File: %s, Line: %d\n", __FILE__, __LINE__);

#define $c(arg) fprintf(stderr, "Var: \"%s\", Char: %c, File: %s, Line: %d\n", #arg, arg, __FILE__, __LINE__);

#define $s(arg) fprintf(stderr, "String in File: %s, Line: %d\n%s\n", __FILE__, __LINE__, arg);

#define $d(arg) fprintf(stderr, "Decimal %s: %d, File: %s, Line: %d\n", #arg, arg, __FILE__, __LINE__);
#define $ld(arg) fprintf(stderr, "Var: \"%s\", Size_t: %ld, File: %s, Line: %d\n", #arg, arg, __FILE__, __LINE__);

#define $dbl(arg) fprintf(stderr, "Double: %llg, File: %s, Line: %d\n", arg, __FILE__, __LINE__);

#define $p(arg) fprintf(stderr, "Pointer: %p, File: %s, Line: %d\n", arg, __FILE__, __LINE__);

#else
        #define $
        #define $c(arg)
        #define $s(arg)
        #define $d(arg)
        #define $lld(arg)
        #define $dbl(arg)
        #define $p(arg)
#endif /*DEBUG*/

#endif /*DEBUG_H*/
