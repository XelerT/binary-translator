#include "src/include/config.h"
#include "src/include/jit.h"
#include "src/include/text.h"
#include "src/include/tokens.h"

int main (int argc, char *argv[])
{
        log_init("jit.html");

        int n_file_name_arg = 1;
        int execution_status = check_arguments(argc, argv, &n_file_name_arg);
        if (execution_status)
                return execution_status;

        tokens_t tokens = {};
        execution_status = parse_my_binary_file(&tokens, argv[n_file_name_arg]);
        if (execution_status)
                goto terminate_process;

        execution_status = jit(&tokens);

terminate_process:
        tokens_dtor(&tokens);
        log_dtor();

        return execution_status;
}
