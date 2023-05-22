section .text
global _print_decimal

_print_decimal:
        pop rax
        call print_sign_decimal

        ret

;------------------------------------------------------------------------------------------------------------------------------------------
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
;       Entry: rax - number to print
;------------------------------------------------------------------------------------------------------------------------------------------

print_sign_decimal:
                test rax, rax                   ; need - before?
                jns .has_no_sign

                mov bl, '-'

                push rsi
                push rdx
                push rdi
                push rax

                push rcx
                mov rcx, rsp
                sub rcx, 20

                mov byte [rsp - 20], bl         ; put char to print before stack
                push rcx

                call print_char                 ; print - before number

                add rsp, 8
                pop rcx
                pop rax
                pop rdi
                pop rdx
                pop rsi

.has_no_sign:
                call print_unsign_decimal

                ret

;------------------------------------------------------------------------------------------------------------------------------------------
;       Prints unsigned decimal number.
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
;       Entry: rax
;       Destroys: rcx
;                 rdx
;                 rdi
;------------------------------------------------------------------------------------------------------------------------------------------

print_unsign_decimal:

                mov rdi, 10             ; base of notation
                ; xor rcx, rcx
                mov rcx, 20

.next_number:
                xor rdx, rdx

                div rdi

                add dl, '0'

                mov byte [DECIMAL_NUMBER_BUF + rcx], dl         ; write number in buffer
                dec rcx

                cmp rax, 0
                jne .next_number

                call print_decimal_from_buf                     ; print decimal from buffer

                ret

;------------------------------------------------------------------------------------------------------------------------------------------
;       Prints decimal from buffer
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
;       Entry:          rcx - length of number
;       Destroys:
;------------------------------------------------------------------------------------------------------------------------------------------

print_decimal_from_buf:

                mov rsi, DECIMAL_NUMBER_BUF             ; buffer offset
                add rsi, rcx
                inc rsi

                mov rdx, 20                             ; calculate length of the line
                sub rdx, rcx

                mov rax, 0x01
                mov rdi, 1
                syscall

                ret

section .data

DECIMAL_NUMBER_BUF: db "____________________"
