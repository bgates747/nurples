    MACRO div_24_BC_DE
        call div_24
    ENDMACRO

; this is an adaptation of Div16 extended to 24 bits
; from https://map.grauw.nl/articles/mult_div_shifts.php
; it works by shifting each byte of the dividend left into carry 8 times
; and adding the dividend into hl if the carry is set
; thus hl accumulates a remainder depending on the result of each iteration
; ---------------------------------------------------------
; Divide 24-bit unsigned values 
;   with 24-bit unsigned result
;   and 24-bit remainder
; In: Divide ubc by ude
; Out: ubc = result, uhl = remainder
; Destroys: a,hl,bc
div_24:
    ld hl,0     ; Clear accumulator for remainder
; put dividend in scratch so we can get at all its bytes
    ld (ubc),bc ; scratch ubc also accumulates the quotient
    ld a,(ubc+2); grab the upper byte of the dividend
    ld b,8      ; loop counter for 8 bits in a byte
@loop0:
    rla         ; shift the next bit of dividend into the carry flag
    adc hl,hl   ; shift the remainder left one bit and add carry if any
    sbc hl,de   ; subtract divisor from remainder
    jr nc,@noadd0   ; if no carry,remainder is <= divisor
                ; meaning remainder is divisible by divisor
    add hl,de   ; otherwise add divisor back to remainder
                ; reversing the previous subtraction
@noadd0:
    djnz @loop0 ; repeat for all 8 bits
    rla         ; now we shift a left one more time
    cpl         ; then flip its bits for some reason
    ld (ubc+2),a; magically this is the upper byte of the quotient
    ld a,(ubc+1); now we pick up the middle byte of the dividend
    ld b,8      ; set up the next loop and do it all again ...
@loop1:
    rla
    adc hl,hl
    sbc hl,de
    jr nc,@noadd1
    add hl,de
@noadd1:
    djnz @loop1
    rla
    cpl
    ld (ubc+1),a ; writing the middle byte of quotient
    ld a,(ubc)
    ld b,8
@loop2:          ; compute low byte of quotient
    rla
    adc hl,hl
    sbc hl,de
    jr nc,@noadd2
    add hl,de
@noadd2:
    djnz @loop2
    rla
    cpl
    ld (ubc),a  ; ... write low byte of quotient
    ld bc,(ubc) ; load quotient into bc for return
    ret         ; hl already contains remainder so we're done