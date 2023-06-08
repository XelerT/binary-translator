# Binary translator

## Overview

## Introduction

This work was made for studying processes of binary translation and JIT compilation.

Our binary translator is a tool to convert binary code for [my stack virtual cpu](https://github.com/XelerT/cpu). For creating programs for our cpu you can use our C-like [language](https://github.com/XelerT/language).

---

## Install & Use
Install and compile:

        $ git clone https://github.com/XelerT/binary-translator.git
        $ make

To compile with $int 3$ at the start of exec buffer for debugger:

        $ make run_debug

Run:

        $ ./jit bit_code.txt

Run tests:

        $ make test

## x86-64 encode
We will tell you about some information with was used for creating commands auto encoding.

Command in x86-64 can consists of 5 parts (first line is size in bytes):

        ------------------------------------------------------------
        |  0 - 4  |    1 - 2      |         1        |    1      | 0 - 4        |
        | Prefix | Opcode    | M.R.R/M  | S.I.B  | Data        |
        -------------------------------------------------------------
- Prefix part is used to encode 64 bit information, such as need to use 8 byte registers, need to use r8-r15 in source or destination.

- In opcode command is encoded: (xxxxxx|ds). First 6 bits of this byte encode command, s-bit is 1 if we use 64-bit mode of command, d-bit contains information about source and destination:

        register  -> memory   =>      d = 0
        memory -> register    =>      d = 1

- Mode-Register-Register/Memory (M.R.R/M): (xx|xxx|xxx). First 2 bits show which arguments has command, 11 - if 2 operands are registers, 00 - if destination is memory, 10 - if destination is register and source is immediate number.

- Scale-Index-Base (S.I.B): (xx|xxx|xxx). First 2 bits indicates the scaling factor of index, next 3 bits is the index register to use, last three is the base register to use.

- Data field can contain immediate number or address.

## Translation
For technical details see [documentation]().
### Parsing binary file
During parsing process array of tokens is created. Each token contains information about each command: command encode for our cpu, (repeating x86 encode rules) mode, need of using r8-r15 registers etc. After this process simple IR is created.

### Creation of executed buffer
After parsing, array of tokens is translated in x86 commands. Each token is going through some layers of check, folding stack constructions into non-stack logic, for example:

        push 5
        push rax        =>      add rax, 5
        add                           push rax
        ───────────────────────────────────────
        push 5
        pop rbx         =>      mov rbx, 5
We obliged to save stack logic in order to avoid destruction of program logic therefor after math functions we push register into stack.
<details>

<summary>More encoding details.</summary>

Jumps' and call's addresses are fulfilled using 2 step passage.

---
Our cpu have 2 stacks, the first is for working with numbers and the second contains returning addresses. For solving this problem we emulate our second stack. Register r15 is used as second rsp, at the beginning of buffer $mov r15, (end of data section address)$.
Every call is changed for "push" in the second stack and jump to the address:

        mov qword [r15], (return address)
        sub r15, 8
        jmp (function)

Before return we need to "pop" return address:

        add r15, 8
        push qword [r15]
        ret

---
For more details see [documentation]()

</details>

### Execution
After creation of execution buffer we change rights for "text section" and "data section" for execution, read, write using [mprotect](https://man7.org/linux/man-pages/man2/mprotect.2.html). Then we cast execution buffer pointer to function pointer and call this "function".

<!-- ## Speed check -->

