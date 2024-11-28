    assume adl=1   
    org 0x040000    

    include "mos_api.inc"

    MACRO PROGNAME
    ASCIZ "flower_demo"
    ENDMACRO

    jp start       

_exec_name:
	PROGNAME

    align 64      
    db "MOS"       
    db 00h         
    db 01h

start:              
    push af
    push bc
    push de
    push ix
    push iy

    call init
    call main

exit:

    pop iy 
    pop ix
    pop de
    pop bc
    pop af
    ld hl,0

    ret 

; --- MAIN PROGRAM ---
; APPLICATION INCLUDES
; API includes

; --- INITIALIZATION ---
init:
   ret

; --- MAIN PROGRAM ---
main:

    ret


sqrt24:
    ;; Expects ADL mode
    ;; Inputs: HL
    ;; Outputs: DE is the integer square root
    ;;          HL is the difference inputHL-DE^2
    ;;          C flag reset
    xor a
    ld b, l
    push bc
    ld b, a
    ld d, a
    ld c, a
    ld l, a
    ld e, a

    ; Iteration 1
    add hl, hl
    rl c
    add hl, hl
    rl c
    sub c
    jr nc, $+6
    inc e
    inc e
    cpl
    ld c, a

    ; Iteration 2
    add hl, hl
    rl c
    add hl, hl
    rl c
    rl e
    ld a, e
    sub c
    jr nc, $+6
    inc e
    inc e
    cpl
    ld c, a

    ; Iteration 3
    add hl, hl
    rl c
    add hl, hl
    rl c
    rl e
    ld a, e
    sub c
    jr nc, $+6
    inc e
    inc e
    cpl
    ld c, a

    ; Iteration 4
    add hl, hl
    rl c
    add hl, hl
    rl c
    rl e
    ld a, e
    sub c
    jr nc, $+6
    inc e
    inc e
    cpl
    ld c, a

    ; Iteration 5
    add hl, hl
    rl c
    add hl, hl
    rl c
    rl e
    ld a, e
    sub c
    jr nc, $+6
    inc e
    inc e
    cpl
    ld c, a

    ; Iteration 6
    add hl, hl
    rl c
    add hl, hl
    rl c
    rl e
    ld a, e
    sub c
    jr nc, $+6
    inc e
    inc e
    cpl
    ld c, a

    ; Iteration 7
    add hl, hl
    rl c
    add hl, hl
    rl c
    rl b
    ex de, hl
    add hl, hl
    push hl
    sbc hl, bc
    jr nc, $+8
    ld a, h
    cpl
    ld b, a
    ld a, l
    cpl
    ld c, a
    pop hl
    jr nc, $+4
    inc hl
    inc hl
    ex de, hl

    ; Iteration 8
    add hl, hl
    ld l, c
    ld h, b
    adc hl, hl
    adc hl, hl
    ex de, hl
    add hl, hl
    sbc hl, de
    add hl, de
    ex de, hl
    jr nc, $+6
    sbc hl, de
    inc de
    inc de

    ; Iteration 9
    pop af
    rla
    adc hl, hl
    rla
    adc hl, hl
    ex de, hl
    add hl, hl
    sbc hl, de
    add hl, de
    ex de, hl
    jr nc, $+6
    sbc hl, de
    inc de
    inc de

    ; Iteration 10
    rla
    adc hl, hl
    rla
    adc hl, hl
    ex de, hl
    add hl, hl
    sbc hl, de
    add hl, de
    ex de, hl
    jr nc, $+6
    sbc hl, de
    inc de
    inc de

    ; Iteration 11
    rla
    adc hl, hl
    rla
    adc hl, hl
    ex de, hl
    add hl, hl
    sbc hl, de
    add hl, de
    ex de, hl
    jr nc, $+6
    sbc hl, de
    inc de
    inc de
    rr d
    rr e
    ret