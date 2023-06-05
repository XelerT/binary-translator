#include <stdio.h>
#include <assert.h>
#include <sys/mman.h>
#include <cstdlib>

#include "../include/config.h"
#include "../include/tokens.h"
#include "../include/jit.h"
#include "../include/translate2x86.h"

int jit (tokens_t *tokens)
{
        assert(tokens);

        jit_code_t jit_code {};

        int exec_status = jit_code_ctor(&jit_code, tokens->size * 4);
        if (exec_status)
                return exec_status;

        fill_jit_code_buf(&jit_code, tokens);
        exec_status = make_buf_executable((void*) jit_code.buf, jit_code.size);
        exec_status = make_buf_writable((void*) jit_code.exec_memory2use, jit_code.exec_memory_capacity);
        run_jit_buffer((void(*)()) jit_code.buf);

        return exec_status;
}

int jit_code_ctor (jit_code_t *jit_code, size_t capacity)
{
        assert(jit_code);

        jit_code->buf = (uint8_t*) aligned_alloc(PAGE_SIZE, capacity);
        if (!jit_code->buf) {
                log_error(1, "ALIGNED_ALLOC RETURNED FOR NULL JIT_BUF.");
                return NULL_CALLOC;
        }

        jit_code->exec_memory2use = (uint8_t*) aligned_alloc(PAGE_SIZE, capacity);
        if (!jit_code->exec_memory2use) {
                log_error(1, "ALIGNED_ALLOC RETURNED FOR NULL JIT_BUF.");
                return NULL_CALLOC;
        }
        jit_code->exec_memory_capacity = capacity;
        // for (size_t i = 0; i < capacity; i++) {
        //         jit_code->buf[i] = 0;
        // }

// /*
        *jit_code->buf = 0xCC;
        jit_code->size++;
        for (size_t i = jit_code->size; i < capacity; i++) {
                jit_code->buf[i] = 0xC3;
        }
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

        if (jit_code->exec_memory2use) {
                free(jit_code->exec_memory2use);
                jit_code->exec_memory2use = nullptr;
        }
        jit_code->exec_memory_capacity = 0;
}

#include <errno.h>
#include <string.h>
extern int errno;

int make_buf_executable (void *buf, size_t buf_capacity)
{
        assert(buf);

        if (mprotect(buf, buf_capacity, PROT_WRITE | PROT_EXEC | PROT_READ)) {
                int errnum = 0;
                log_error(1, "mprotect RETURNED ERROR");

                perror("\nError printed by perror");
                fprintf(stderr, "\nError using mprotect: %s\n", strerror( errnum ));

                return CHANGE_PROTECTION_ERROR;
        }

        return 0;
}

int make_buf_writable (void *buf, size_t buf_capacity)
{
        assert(buf);

        if (mprotect(buf, buf_capacity, PROT_WRITE | PROT_EXEC | PROT_READ)) {
                int errnum = 0;
                log_error(1, "mprotect RETURNED ERROR");

                perror("\nError printed by perror");
                fprintf(stderr, "\nError using mprotect: %s\n", strerror( errnum ));

                return CHANGE_PROTECTION_ERROR;
        }

        return 0;
}

void run_jit_buffer (void (*buf_func) ())
{
        buf_func();
}
