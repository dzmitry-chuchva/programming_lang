;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                                                  ;;
;; SCREEN IO FUNCTIONS LIB. Written by Dmitry Chuchva aka Mitro (c) 2006 dmitry_chuchva@mail.ru     ;;
;;                                                                                                  ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;




EXTERN __imp_malloc, __imp_free
EXTERN __imp_printf, __imp_scanf

GLOBAL _my_print__000, _my_read__001, _my_print_str__002, _my_println__003

[section .data]

format_str_ptr dd 0
format_str_str db '%s',0
crlf db 0dh,0ah,0
type_int equ 100h
type_float equ 200h
double resq 1
spcorrection dd 0

[section .text]

;;;;;;;;;;;Function my_print;;;;;;;;;;;;;;;;;;;
; stack is:
; count_of_args
; type_of_argN
; argN
; ...
; type_of_arg1
; arg1

_my_print__000:
        enter 0,0
        mov eax,[ebp + 8]       ; count_of_args in eax
        mov [spcorrection],eax
        mov edx,3
        mul dx                  ; 3 * count_of_args in eax = length of format string for printf
        push eax
        call [__imp_malloc]     ; malloc
        mov [format_str_ptr],eax
        mov ecx,[ebp + 8]       ; loop count
        mov esi,8               ; init esi

.push_loop:
        add esi,8
        cmp dword [ebp + esi - 4],type_float
        je .float2double
        push dword [ebp + esi]
        jmp .push_loop_check
.float2double:
        fld dword [ebp + esi]
        fstp qword [double]
        push dword [double + 4]
        push dword [double]        
        inc dword [spcorrection]
.push_loop_check:
        loop .push_loop

        mov edi,[format_str_ptr]
        mov ecx,[ebp + 8]
.form_str: ; forming format string for printf
        mov byte [edi],'%'
        mov byte [edi + 1],'d'
        cmp dword [ebp + esi - 4],type_int
        je .is_int
        mov byte [edi + 1],'f'
.is_int:
        mov byte [edi + 2],' '
        add edi,3
        sub esi,8
        loop .form_str

        mov byte [edi - 1],0      ; append NULL

        push dword [format_str_ptr]
        call [__imp_printf]

        mov eax,[spcorrection]
        shl eax,2
        add eax,4
        add esp,eax             ; clear args after _cdecl function

        push dword [format_str_ptr]
        call [__imp_free]
        
        leave        
        ret

;;;;;;;;;;;Function my_read;;;;;;;;;;;;;;;;;;;
; stack is:
; count_of_args
; type_of_argN
; addr_argN
; ...
; type_of_arg1
; addr_arg1

_my_read__001:
        enter 0,0
        mov eax,[ebp + 8]       ; count_of_args in eax
        mov [spcorrection],eax
        mov edx,3
        mul dx                  ; 3 * count_of_args in eax = length of format string for printf
        push eax
        call [__imp_malloc]     ; malloc
        mov [format_str_ptr],eax
        mov ecx,[ebp + 8]       ; loop count
        mov esi,8               ; init esi

.push_loop:
        add esi,8
;        cmp dword [ebp + esi - 4],type_float
;        je .float2double
        push dword [ebp + esi]
;        jmp .push_loop_check
;.float2double:
;        fld dword [ebp + esi]
;        fstp qword [double]
;        push dword [double + 4]
;        push dword [double]        
;        inc dword [spcorrection]
;.push_loop_check:
        loop .push_loop

        mov edi,[format_str_ptr]
        mov ecx,[ebp + 8]
.form_str: ; forming format string for printf
        mov byte [edi],'%'
        mov byte [edi + 1],'d'
        cmp dword [ebp + esi - 4],type_int
        je .is_int
        mov byte [edi + 1],'f'
.is_int:
        mov byte [edi + 2],' '
        add edi,3
        sub esi,8
        loop .form_str

        mov byte [edi - 1],0      ; append NULL

        push dword [format_str_ptr]
        call [__imp_scanf]

        mov eax,[spcorrection]
        shl eax,2
        add eax,4
        add esp,eax             ; clear args after _cdecl function

        push dword [format_str_ptr]
        call [__imp_free]
        
        leave        
        ret

;;;;;;;;;;;Function my_print_str;;;;;;;;;;;;;;;;;;;
; stack is:
; str

_my_print_str__002:
        enter 0,0
        push dword [ebp + 8]
        push dword format_str_str
        call [__imp_printf]
        pop eax
        pop eax
        leave
        ret 4

;;;;;;;;;;Function my_println;;;;;;;;;;;;;;;;;;;;;;
; stack is:
; <no>

_my_println__003:
        enter 0,0
        push dword crlf
        call [__imp_printf]
        pop eax
        leave
        ret