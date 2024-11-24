arith24uaf: ds 6
arith24uhl: ds 6
arith24ubc: ds 6
arith24ude: ds 6
arith24uix: ds 6
arith24uiy: ds 6
arith24usp: ds 6
arith24upc: ds 6

; write hlu to a 54-bit output buffer arithmetically shifted up or down a specified number of bits
; uses whole-byte shifts for the initial shift and bit shifts for the remainder
; inputs: hlu = 24-bit number to shift
;         a = signed number of bits to shift
;         ix = output buffer address
; outputs: hlu = shifted 24-bit number, ix = modified pointer aligned with value in hlu
shift_hlu:
    push bc ; preserve
    ld b,a            ; store shift value in b for later

; Initialize DE to zero and clear output buffer
    ld de, 0
    ld (ix-3), de
    ld (ix+3), de

; Get absolute value of a and save its original sign
    or a
    push af ; save sign flag
    jp p,@F ; If a is positive do nothing
    neg ; If a is negative, negate it
@@:

; Write abs(HLU) to the output buffer and save its original sign
    call hlu_abs
    push af ; save sign of HLU
    ld (ix), hl

; Divide bits to shift by 8 to get the whole byte shift and bit remainder
    ld a,b            ; Restore original shift value into 'a'
    srl a             ; a = a / 2 (shift right 1 bit)
    srl a             ; a = a / 4 (shift right another bit)
    srl a             ; a = a / 8 (final shift for division by 8)
    ld c,a            ; c holds the number of whole bytes to shift

; Calculate remainder of a (original shift) mod 8
    and 0x07          ; Mask with 0x07 to get the remainder (bits to shift back up)
    ld b,a            ; 'b' now holds the number of bits to shift back up
    ld de,0           ; Clear deu for computing the offset address later

    ; call dumpRegistersHex ; DEBUG

; Get back the original sign of the byte shift
    pop af            ; Restore sign flag
    jp p,@F           ; If a was positive, we're done
    ld a,c            ; 'a' now holds the number of whole bytes to shift
    neg               ; Negate the number of whole bytes to shift
    dec a             ; Subtract 1 because that's just how this works
    ld c,a            ; 'c' now holds the number of whole bytes to shift
    dec de            ; DE = -1 to make the signed addition below work
    ld a,8            ; Subtract bits to shift from 8 because we're in backwards land
    sub b
    ld b,a            ; b is the number of bits to shift back up

    ; call dumpRegistersHex ; DEBUG

@@:

; Add the byte offset to the base address of output buffer
    ld e,c            ; DEU and D were properly signed above
    add ix,de         ; Add byte-aligned offset (hl = output buffer + l)

    ; call dumpRegistersHex ; DEBUG

; Read the byte-aligned result into HL and shift in the required number of bits
    ld hl,(ix)        ; Byte-aligned result

    call dumpRegistersHex ; DEBUG

; Check whether we're already byte-aligned
    xor a
    or b
    jr z,@end         ; If no bits to shift, we're done

; Otherwise shiften zee bitzen
    ld a,(ix-1)       ; read one byte below for the bits to shift into HLU
@loop:
    rlca              ; One bit to carry

    call dumpFlags ; DEBUG

    adc hl,hl         ; Shift the carry into the result

    call dumpRegistersHex ; DEBUG

    djnz @loop        ; Loop until all bits are shifted up

@end:

    call dumpRegistersHex ; DEBUG

; get back HLU's original sign and negate if necessary
    pop af
    jp p,@F
    call neg_hlu
@@:
; return the result
    ld (ix),hl        ; Store the shifted result
    pop bc              ; Restore BC
    ret

; operation: UHL * UDE --> UHL
; multiply unsigned 24-bit numbers and return a 48-bit intermediate
; with inputs and outputs having an arbitrary number of fractional bits from 0 to 24
; inputs: hl = 24-bit number, de = 24-bit number, 
;         b = hl fractional bits, c = de fractional bits, a = output fractional bits
; outputs: umulfxout = 48-bit intermediate
; destroys: af, hl, bc, de
umulfx:
    push ix ; preserve
    push af ; need later

; do the multiplication
    call umul24x24
    ; DEBUG
    ld hl,umulfxout ; point to output buffer
    ld a,6 ; 6 bytes to print
    call dumpMemoryHex ; print the result
    ; END DEBUG

; shift the result to the required output precision
    ld a,b
    add a,c ; a is left shift from the multiplication
    pop bc ; b is the output precision (was a)
    sub b ; a is the net shift of the output

; Get absolute value of a and save its original sign
    or a
    push af ; save sign flag
    jp p,@F ; If a is positive do nothing
    neg ; If a is negative, negate it
@@:

; Divide a by 8 to get the whole byte shift and bit remainder
    ld b,a            ; Store 'a' temporarily in 'b' (net shift)
    srl a             ; a = a / 2 (shift right 1 bit)
    srl a             ; a = a / 4 (shift right another bit)
    srl a             ; a = a / 8 (final shift for division by 8)
    ld c,a            ; c holds the number of whole bytes to shift

; Calculate remainder of a (original shift) mod 8
    ld a,b            ; Restore original shift value into 'a'
    and 0x07          ; Mask with 0x07 to get the remainder (bits to shift back up)
    ld b,a            ; 'b' now holds the number of bits to shift back up
    ld de,0           ; Clear deu for computing the offset address later

    ; call dumpRegistersHex ; DEBUG

; Get back the original sign of the byte shift
    pop af            ; Restore sign flag
    jp p,@F           ; If a was positive, we're done
    ld a,c            ; 'a' now holds the number of whole bytes to shift
    neg               ; Negate the number of whole bytes to shift
    dec a             ; Subtract 1 because that's just how this works
    ld c,a            ; 'c' now holds the number of whole bytes to shift
    dec de            ; DE = -1 to make the signed addition below work
    ld a,8            ; Subtract bits to shift from 8 because we're in backwards land
    sub b
    ld b,a            ; b is the number of bits to shift back up

    ; call dumpRegistersHex ; DEBUG

@@:

; Add the byte offset to the base address of umulfxout
    ld e,c            ; DEU and D were properly signed above
    ld ix,umulfxout   ; Load base address of the output buffer
    add ix,de         ; Add byte-aligned offset (hl = umulfxout + l)

    ; call dumpRegistersHex ; DEBUG

; Read the byte-aligned result into HL and shift in the required number of bits
    ld hl,(ix)        ; Byte-aligned result
    ld a,(ix-1)       ; One byte below

; Check whether we're already byte-aligned
    xor a
    or b
    jr z,@end         ; If no bits to shift, we're done

; Otherwise shiften zee bitzen
@loop:

    ; call dumpRegistersHex ; DEBUG

    rlca              ; One bit to carry
    adc hl,hl         ; Shift the carry into the result

    djnz @loop        ; Loop until all bits are shifted up

@end:
; return the result
    pop ix ; restore
    ret

    dl 0 ; padding
umulfxout: blkb 6,0
    dl 0 ; padding

; operation: UHL * UDE --> UHL
; multiply signed 24-bit numbers and return a 48-bit intermediate
; with inputs and outputs having an arbitrary number of fractional bits from 0 to 24
; inputs: hl = 24-bit number, de = 24-bit number, 
;         b = hl fractional bits, c = de fractional bits, a = output fractional bits
; outputs: umulfxout = 48-bit intermediate
; destroys: af, hl, bc, de
smulfx:
; make everything positive and store sign flags
	call hlu_abs
	push af
	ex de,hl
	call hlu_abs
	ex de,hl
	push af
; do the division
    call umulfx ; hl = product
; adjust sign of result
	pop af ; sign de
	jp m,@de_neg
	pop af ; sign hl
	ret p ; both positive, nothing to do
@hl_neg:
    call neg_hlu ; de pos, hl neg, result is negative
    ret
@de_neg:
	pop af
	ret m ; both negative, nothing to do
	call neg_hlu ; result is negative
	ret

; multiply a signed 24-bit number by an unsigned 8-bit number giving a signed 24-bit result
; uses EZ80 MLT instruction for speed
; operation: UHL * A --> UHL
; destroys: AF, HL
smul24x8:
; make hl positive and store sign flag
	call hlu_abs
	push af
; do the division
    call umul24x8 ; hl = product
; adjust sign of result
	pop af ; sign de
	ret p ; hl was positive, nothing to do
	call neg_hlu ; result is negative
	ret

; unsigned multiplication of a 24-bit and 8-bit number giving a 32-bit result
; uses EZ80 MLT instruction for speed
; operation: UHL * A --> AUHL
; destroys: AF, HL
umul24x8:
	push de ; preserve de
; low byte
	ld e,l
	ld d,a
	mlt de
	ld l,e ; product low byte
	ex af,af' ; save multiplier
	ld a,d ; carry
	ex af,af' ; save carry, restore multiplier
; high byte
	ld e,h
	ld d,a
	mlt de
	ex af,af' ; save multiplier, restore carry
	add a,e ; add carry
	ld h,a ; product middle byte
	ld a,d ; carry
	ex af,af' ; save carry, restore multiplier
; upper byte
	push hl
	inc sp
	pop de ; d = hlu
	dec sp
	ld e,a
	mlt de
	ex af,af' ; restore carry
	adc a,e ; add carry
    ld (@scratch),hl ; 7 cycles
    ld (@scratch+2),a ; 5 cycles
    ld hl,(@scratch) ; 7 cycles
; highest byte
	ld a,0 ; preserve carry flag
	adc a,d ; product highest byte
	pop de ; restore de
	ret
@scratch: ds 3

; unsigned multiplication of two 24-bit numbers giving a 48-bit result
; operation: UHL * UDE --> umulfxout
umul24x24:
	ld iy,umulfxout ; point to output buffer
	push bc
	ld bc,0
	ld (iy),bc
	ld (iy+3),bc
	pop bc

; STEP 1: UHL * E
	ld a,e
	push hl
	call umul24x8
	ld (iy+0),hl
	ld (iy+3),a

; STEP 2: UHL * D
	pop hl
	push hl
	ld a,d
	call umul24x8
	call @accumulate

; STEP 3: UHL * DEU
	pop hl
	push de
	inc sp
	pop af
	dec sp
	call umul24x8

@accumulate:
	inc iy
; highest byte of product to carry
	ld (iy+3),a
; low byte of product
	ld a,l
	add a,(iy+0)
	ld (iy+0),a
; high byte of product
	ld a,h
	adc a,(iy+1)
	ld (iy+1),a
; uppper byte of product
	push hl
	inc sp
	pop hl
	dec sp
	ld a,h
	adc a,(iy+2)
	ld (iy+2),a
; carry
	ld a,0 ; preserve flags
	adc a,(iy+3)
	ld (iy+3),a
	ret

; umul168:	UH.L = UH.L*UD.E (unsigned)
umul168:
	call umul24x24
	ld hl,(iy-1)
	ret

; smul168:	UH.L * UD.E --> UH.L (signed)
smul168:
; make everything positive and store sign flags
	call hlu_abs
	push af
	ex de,hl
	call hlu_abs
	ex de,hl
	push af
; do the division
    call umul168 ; hl = product
; adjust sign of result
	pop af ; sign de
	jp m,@de_neg
	pop af ; sign hl
	ret p ; both positive, nothing to do
@hl_neg:
    call neg_hlu ; de pos, hl neg, result is negative
    ret
@de_neg:
	pop af
	ret m ; both negative, nothing to do
	call neg_hlu ; result is negative
	ret

; UH.L / UD.E --> UD.E rem UHL (unsigned)
; perform unsigned division of 16.8 fixed place values
; with an unsigned 16.8 fixed place result and 24-bit remainder
udiv168:
; back up divisor
	ld (@ude),de
; get the 16-bit integer part of the quotient
    call udiv24 ; de = quotient, hl = remainder
; load quotient to upper three bytes of output
    ld (div168_out+1),de
@div256:
; multiply remainder by 256
	hlu_mul256
; skip fractional computation if remainder is zero
    sign_hlu
    jr nz,@div_frac
    xor a
    jr @write_frac
; now divide the shifted remainder by the divisor
@div_frac:
	ld de,(@ude) ; get back divisor
    call udiv24 ; de = quotient, hl = remainder
; load low byte of quotient to low byte of output
    ld a,e
@write_frac:
    ld (div168_out),a
; load de with return value
    ld de,(div168_out)
; load a with any overflow
    ld a,(div168_out+3)
    ret ; ud.e is the 16.8 result
@ude: ds 6
div168_out: ds 4 ; the extra byte is for overflow

; UH.L / UD.E --> UD.E (signed) rem UHL (unsigned)
; perform signed division of 16.8 fixed place values
; with a signed 16.8 fixed place result and unsigned 24-bit remainder
sdiv168:
; make everything positive and store sign flags
	call hlu_abs
	push af
	ex de,hl
	call hlu_abs
	ex de,hl
	push af
; do the division
    call udiv168 ; de = quotient, hl = remainder
; adjust sign of result
	pop af ; sign de
	jp m,@de_neg
	pop af ; sign hl
	ret p ; both positive, nothing to do
@hl_neg:
    ex de,hl ; hl = quotient, de = remainder
    call neg_hlu ; de pos, hl neg, result is negative
    ex de,hl ; de = negated quotient, hl = remainder
    ret
@de_neg:
	pop af
	ret m ; both negative, nothing to do
    ex de,hl ; hl = quotient, de = remainder
	call neg_hlu ; result is negative
    ex de,hl ; de = negated quotient, hl = remainder
	ret
; ========== FROM maths24.inc ==========

; Expects  ADL mode
; Inputs:  UH.L
; Outputs: UH.L is the 16.8 square root
;          UD.E is the difference inputHL-DE^2
;          c flag reset  
sqrt168:
    call sqrt24
    ex de,hl
    add hl,hl
    add hl,hl
    add hl,hl
    add hl,hl
    ret

; Expects  ADL mode
; Inputs:  HL
; Outputs: DE is the integer square root
;          HL is the difference inputHL-DE^2
;          c flag reset 
sqrt24:
    xor a
    ld b,l
    push bc
    ld b,a
    ld d,a
    ld c,a
    ld l,a
    ld e,a

    ;Iteration 1
    add hl,hl
    rl c
    add hl,hl
    rl c
    sub c
    jr nc,$+6
    inc e
    inc e
    cpl
    ld c,a

    ;Iteration 2
    add hl,hl
    rl c
    add hl,hl
    rl c
    rl e
    ld a,e
    sub c
    jr nc,$+6
    inc e
    inc e
    cpl
    ld c,a

    ;Iteration 3
    add hl,hl
    rl c
    add hl,hl
    rl c
    rl e
    ld a,e
    sub c
    jr nc,$+6
    inc e
    inc e
    cpl
    ld c,a

    ;Iteration 4
    add hl,hl
    rl c
    add hl,hl
    rl c
    rl e
    ld a,e
    sub c
    jr nc,$+6
    inc e
    inc e
    cpl
    ld c,a

    ;Iteration 5
    add hl,hl
    rl c
    add hl,hl
    rl c
    rl e
    ld a,e
    sub c
    jr nc,$+6
    inc e
    inc e
    cpl
    ld c,a

    ;Iteration 6
    add hl,hl
    rl c
    add hl,hl
    rl c
    rl e
    ld a,e
    sub c
    jr nc,$+6
    inc e
    inc e
    cpl
    ld c,a

    ;Iteration 7
    add hl,hl
    rl c
    add hl,hl
    rl c
    rl b
    ex de,hl
    add hl,hl
    push hl
    sbc hl,bc
    jr nc,$+8
    ld a,h
    cpl
    ld b,a
    ld a,l
    cpl
    ld c,a
    pop hl
    jr nc,$+4
    inc hl
    inc hl
    ex de,hl

    ;Iteration 8
    add hl,hl
    ld l,c
    ld h,b
    adc hl,hl
    adc hl,hl
    ex de,hl
    add hl,hl
    sbc hl,de
    add hl,de
    ex de,hl
    jr nc,$+6
    sbc hl,de
    inc de
    inc de

    ;Iteration 9
    pop af
    rla
    adc hl,hl
    rla
    adc hl,hl
    ex de,hl
    add hl,hl
    sbc hl,de
    add hl,de
    ex de,hl
    jr nc,$+6
    sbc hl,de
    inc de
    inc de

    ;Iteration 10
    rla
    adc hl,hl
    rla
    adc hl,hl
    ex de,hl
    add hl,hl
    sbc hl,de
    add hl,de
    ex de,hl
    jr nc,$+6
    sbc hl,de
    inc de
    inc de

    ;Iteration 11
    rla
    adc hl,hl
    rla
    adc hl,hl
    ex de,hl
    add hl,hl
    sbc hl,de
    add hl,de
    ex de,hl
    jr nc,$+6
    sbc hl,de
    inc de
    inc de

    ;Iteration 11
    rla
    adc hl,hl
    rla
    adc hl,hl
    ex de,hl
    add hl,hl
    sbc hl,de
    add hl,de
    ex de,hl
    jr nc,$+6
    sbc hl,de
    inc de
    inc de

    rr d
    rr e
    ret