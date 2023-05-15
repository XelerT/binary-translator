#include <stdio.h>
#include <assert.h>
#include <sys/mman.h>
#include <cstdlib>

#include "../include/config.h"
#include "../include/tokens.h"
#include "../include/jit.h"
#include "../include/tokens2x86.h"

int jit (tokens_t *tokens)
{
        assert(tokens);

        jit_code_t jit_code {};

        int exec_status = jit_code_ctor(&jit_code, tokens->size * 4);
        if (exec_status)
                return exec_status;

        fill_jit_code_buf(&jit_code, tokens);

        for (size_t i = 0; i < jit_code.size; i++)
                printf("%x ", jit_code.buf[i]);

        exec_status = make_buf_executive((void*) jit_code.buf, jit_code.capacity);

        return exec_status;
}

int jit_code_ctor (jit_code_t *jit_code, size_t capacity)
{
        assert(jit_code);

        jit_code->buf = (uint8_t*) aligned_alloc(PAGE_SIZE, capacity);
        if (!jit_code->buf) {
                log_error(1, "MALLOC RETURNED FOR NULL JIT_BUF.");
                return NULL_CALLOC;
        }

// /*
        *jit_code->buf = 0xCC;
        for (size_t i = 1; i < capacity; i++) {
                jit_code->buf[i] = 0xC3;
        }
        jit_code->size++;
//*/
        // for (size_t i = 0; i < buf_size; i++) {
        //         buf[i] = 0xC3;
        // }
        jit_code->capacity = capacity;

        return 0;
}

void jit_code_dtor (jit_code_t *jit_code)
{
        assert(jit_code);

        if (jit_code->buf) {
                free(jit_code->buf);
                jit_code->buf = nullptr;
        }
        jit_code->size = 0;
        jit_code->capacity = 0;
}

#include <errno.h>
#include <string.h>
extern int errno ;

int make_buf_executive (void *buf, size_t buf_capacity)
{
        assert(buf);

        if (mprotect(buf, buf_capacity, PROT_EXEC)) {
                int errnum;
                log_error(1, "mprotect RETURNED ERROR");

                perror("\nError printed by perror");
                fprintf(stderr, "\nError using mprotect: %s\n", strerror( errnum ));

                return CHANGE_PROTECTION_ERROR;
        }

        return 0;
}
