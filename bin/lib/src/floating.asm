;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                                                  ;;
;; FLOATING SUPPORT LIBRARY. Written by Dmitry Chuchva aka Mitro (c) 2006 dmitry_chuchva@mail.ru    ;;
;;                                                                                                  ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

type_int equ 100h
type_float equ 200h

float_greater equ 04500h
float_equal equ 4000h
float_lower equ 0100h

[section .data]

ftemp db 0
p1 dd 0
flag db 0
flag_minus db 0

[section .text]

GLOBAL __f__add,__f__sub,__f__mul,__f__div,__f__pow,__f__int2float,__f__float2int
GLOBAL __f__eq,__f__neq,__f__gt,__f__lw,__f__ge,__f__le

;; prepares stack for evaluating
;; also converts values
;; used as inline, so it is not standard function, just like jmp in/out
;; (this code is same for all 2 ops functions)

prepare_stack:
        cmp dword [ebp + 8],type_int
        fild dword [ebp + 12]
        je .next1
        fistp dword [ebp + 12]
        fld dword [ebp + 12]
.next1:
        cmp dword [ebp + 16],type_int
        fild dword [ebp + 20]
        je .next2
        fistp dword [ebp + 20]
        fld dword [ebp + 20]
.next2:
        ret

;; gets an evaluated value in eax
;; is also like jmp in/out

clear_and_return:
        cmp dword [ebp + 24],type_int
        je .int
        fstp dword [ftemp]
        jmp .ret
.int:
        fistp dword [ftemp]
.ret:
        ffree st0
        mov eax,[ftemp]
        ret

;; clear_and_return for logical operations with floats
;; always returns int val (1 or 0)

clear_and_return_bool:
        fistp dword [ftemp]
        mov eax,[ftemp]
        ret



;; add values
 ; stack:
 ;      type2
 ;      arg2
 ;      type1
 ;      arg1
 ;      rettype
 ; return in eax

__f__add:
        enter 0,0
        call prepare_stack

        fadd st0,st1

        call clear_and_return
        leave
        ret 20

;; function int2float - convert int to float
 ; stack:
 ;      intval
 ; return in eax

__f__int2float:
        enter 0,0
        fild dword [ebp + 8]
        fstp dword [ebp + 8]
        mov eax,[ebp + 8]
        leave
        ret 4

;; function float2int - convert int to float
 ; stack:
 ;      floatval
 ; return in eax

__f__float2int:
        enter 0,0
        fld dword [ebp + 8]
        fistp dword [ebp + 8]
        mov eax,[ebp + 8]
        leave
        ret 4


;; substract values
 ; stack:
 ;      type2
 ;      arg2
 ;      type1
 ;      arg1
 ;      rettype
 ; return in eax

__f__sub:
        enter 0,0
        call prepare_stack

        fsub st0,st1

        call clear_and_return       
        leave
        ret 20


;; mult values
 ; stack:
 ;      type2
 ;      arg2
 ;      type1
 ;      arg1
 ;      rettype
 ; return in eax

__f__mul:
        enter 0,0
        call prepare_stack

        fmul st0,st1

        call clear_and_return
        leave
        ret 20

;; div values
 ; stack:
 ;      type2
 ;      arg2
 ;      type1
 ;      arg1
 ;      rettype
 ; return in eax

__f__div:
        enter 0,0
        call prepare_stack

        fdiv st0,st1

        call clear_and_return
        leave
        ret 20

;; pow values
 ; stack:
 ;      type2
 ;      arg2
 ;      type1
 ;      arg1
 ;      rettype
 ; return in eax

__f__pow:
        enter 0,0
        call prepare_stack

        ; x^y = 2^(y * log2(x))

;        fyl2x           ; st1 = st1 * log2(st0); pop st0;
;        f2xm1           ; st0 = 2^(st0) - 1;
;        fld1
;        fadd st0,st1    ; st0 = st0 + 1

        mov byte [flag],0
        mov byte [flag_minus],0
        xor ecx,ecx

        ;; set flag if y is lower than 0 so in end we will do 1/res
        fxch
        ftst
        fstsw ax
        sahf
        jnc .m1                 ; y >= 0
        inc byte [flag]
        fabs
.m1:    
        fxch
        ;; ---------------------------------------------------------

        ;; set flag if x is lower than 0 so we need to check that y is integer (i.e. float with
        ;;      fractional part eq to 0) and check parity of y - if isnt parity, return -res
        ftst
        fstsw ax
        sahf
        jnc .m11                ; x >= 0
        fabs                    ; |x|
        fxch
        fist dword [p1]         ; save y as int to temp
        fild dword [p1]         ; and load temp - if they are not equal then y has fractional part
                                ; so check it
        fcomp st1               ; y and temp
        fstsw ax
        sahf
        jnz .before_m11         ; y != temp : skiping
        test dword [p1],1       ; check for parity of y
        jz .before_m11          ; isnt while last bit is 0
        inc byte [flag_minus]
.before_m11:
        fxch
.m11:

        fyl2x           ; z = y * log2(x)
        fabs
        fld1
        fcom st1
        fstsw ax
        sahf
        jp near .exit    ;операнды не сравнимы
        jnc .m2          ;если |z|<1, то переход на m2
;если |z|>1, то приводим к формуле z=z1+z2, где z1-целое, z2-дробное и z2<1:
.m12:   inc     cx
        fsub    st1, st0    
        fcom    st1
        fstsw   ax
        sahf    
        jp      near .exit    ;операнды не сравнимы
        jz      .m12     ;если |z|=1, то переход на m12
        jnc     .m2      ;если |z|<1, то переход на m2
        jmp     .m12     ;если |z|>1, то переход на m12
.m2:    mov     [p1], ecx
        fxch
        f2xm1
        faddp   st1,st0         ;компенсируем 1
        fild    dword [p1]      ;показатель степени для fscale
        fld1
        fscale
        fxch
        ffree st0
        fincstp
        fmul    st0,st1
;проверка на отрицательную степень
        cmp     byte [flag], 1
        jnz     .check_flag2
        fld1
        fxch st1
        fdivp    st1,st0
.check_flag2:
        cmp byte [flag_minus],1
        jnz .exit
        fchs
.exit:
        call clear_and_return
        leave
        ret 20

;; gt(>) values
 ; stack:
 ;      type2
 ;      arg2
 ;      type1
 ;      arg1
 ;      rettype  ;; not used in fact, always returns int
 ; return in eax

__f__gt:
        enter 0,0
        call prepare_stack

        fcompp
        fstsw ax
        test ax,float_greater
        fld1
        jz .next
        ffree st0
        fincstp
        fldz
.next:
        call clear_and_return_bool
        leave
        ret 20


;; lw(<) values
 ; stack:
 ;      type2
 ;      arg2
 ;      type1
 ;      arg1
 ;      rettype  ;; not used in fact, always returns int
 ; return in eax

__f__lw:
        enter 0,0
        call prepare_stack

        fcompp
        fstsw ax
        test ax,float_lower
        fld1
        jnz .next
        ffree st0
        fincstp
        fldz
.next:
        call clear_and_return_bool
        leave
        ret 20


;; ge(>=) values
 ; stack:
 ;      type2
 ;      arg2
 ;      type1
 ;      arg1
 ;      rettype ;; not used in fact, always returns int
 ; return in eax

__f__ge:
        enter 0,0
        call prepare_stack

        fcompp
        fstsw ax
        sahf
        fld1
        jp .next        ; not allowed for comparsions
        jc .next        ; <
        jz .done        ; =
        jmp .done       ; >
.next:
        ffree st0
        fincstp
        fldz
.done:
        call clear_and_return_bool
        leave
        ret 20

;; le(<=) values
 ; stack:
 ;      type2
 ;      arg2
 ;      type1
 ;      arg1
 ;      rettype ;; not used in fact, always returns int
 ; return in eax

__f__le:
        enter 0,0
        call prepare_stack

        fcompp
        fstsw ax
        sahf
        fld1
        jp .next        ; not allowed for comparsions
        jc .done        ; <
        jz .done        ; =
.next:
        ffree st0
        fincstp
        fldz
.done:
        call clear_and_return_bool
        leave
        ret 20

;; eq(==) values
 ; stack:
 ;      type2
 ;      arg2
 ;      type1
 ;      arg1
 ;      rettype  ;; not used in fact, always returns int
 ; return in eax

__f__eq:
        enter 0,0
        call prepare_stack

        fcompp
        fstsw ax
        test ax,float_equal
        fld1
        jnz .next
        ffree st0
        fincstp
        fldz
.next:
        call clear_and_return_bool
        leave
        ret 20

;; neq(!=) values
 ; stack:
 ;      type2
 ;      arg2
 ;      type1
 ;      arg1
 ;      rettype  ;; not used in fact, always returns int
 ; return in eax

__f__neq:
        enter 0,0
        call prepare_stack

        fcompp
        fstsw ax
        test ax,float_equal
        fld1
        jz .next                ; !!!, not jnz
        ffree st0
        fincstp
        fldz
.next:
        call clear_and_return_bool
        leave
        ret 20


