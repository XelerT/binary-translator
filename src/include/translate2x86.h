#ifndef TRANSLATE2x86_H
#define TRANSLATE2x86_H

#include "../include/config.h"
#include "../include/tokens.h"
#include "../include/tokens2x86.h"
#include "../include/jit.h"

#include "../include/mem_cmds.h"
#include "../include/math_cmds.h"
#include "../include/jmp_cmds.h"
#include "../include/conditional_cmds.h"

#include "../include/encode_cmd.h"

/**
 * @brief Translate tokens into x86 commands, writing them into jit_code_t buffer.
 *
 * @param jit_code pointer to jit_code_t
 * @param tokens pointer to tokens_t
 * @return int errors
 */
int  fill_jit_code_buf    (jit_code_t *jit_code, tokens_t *tokens);

/**
 * @brief Paste x86 command in jit_code_t buffer
 *
 * @param jit_code pointer to jit_code_t
 * @param cmd encoded x86 command
 */
void paste_cmd_in_jit_buf (jit_code_t *jit_code, x86_cmd_t *cmd);

/**
 * @brief Finds token command in command table(CMD_TABLE in configs.cmds file) and call encode functions
 * Call encode print or scan functions or assemble_cmd function.
 * @param cmds
 * @param token
 * @param label_table
 * @return int
 */
int  x86_cmd_ctor (x86_cmd_t *cmds, token_t *token, labels_t *label_table);

/**
 * @brief Call encode functions
 *
 * @param cmds array of x86_cmd_t to encode
 * @param token pointer to token to encode
 * @param table_position position of command in CMD_TABLE from configs.cmds file
 * @param label_table pointer to table of labels for jumps and calls
 */
void assemble_cmd (x86_cmd_t *cmds, token_t *token, size_t table_position, labels_t *label_table);

/**
 * @brief Change return value of program from stack to rax
 *
 * @param jit_code
 */
void change_return_value_src2rax (jit_code_t *jit_code);

/**
 * @brief Change memory offset which contains in rbx (as pointer to data section of
 * soft-cpu RAM) to new data section
 *
 * @param jit_code
 */
void change_memory_offset        (jit_code_t *jit_code);

/**
 * @brief Encode imitation of return using r15
 * Encode add r15, 8
 *        push qword [r15]
 *        ret
 * @param cmds
 */
void encode_imitation_of_ret     (x86_cmd_t  *cmds);

#endif /*TRANSLATE2x86_H*/
