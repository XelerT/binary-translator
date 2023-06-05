#include <stdio.h>
#include <assert.h>

#include "../include/conditional_cmds.h"
#include "../include/mem_cmds.h"

#include "../include/consts_x86.cmds"

uint8_t encode_test (x86_cmd_t *cmds)
{
        assert(cmds);

        cmd_info4encode_t info = {
                .dest_reg = RCX
        };
        encode_pop_push(cmds + 0, &info);

        info.dest_reg = RDX;
        encode_pop_push(cmds + 1, &info);

        cmds[2] = test_rcx_rdx;

        return 3;
}

uint8_t encode_cmp (x86_cmd_t *cmds)
{
        assert(cmds);

        cmd_info4encode_t info = {
                .dest_reg = RCX
        };
        encode_pop_push(cmds + 0, &info);

        info.dest_reg = RDX;
        encode_pop_push(cmds + 1, &info);
        cmds[2] = cmp_rcx_rdx;

        return 3;
}
