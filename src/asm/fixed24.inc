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
    ld (@scratch),hl ; 7 cycles
	ld e,a
	ld a,(@scratch+2)
	ld d,a
	mlt de
	ex af,af' ; restore carry
	adc a,e ; add carry
    ld (@scratch+2),a ; 5 cycles
    ld hl,(@scratch) ; 7 cycles
; highest byte
	ld a,0 ; preserve carry flag
	adc a,d ; product highest byte
	pop de ; restore de
	ret
@scratch: ds 3

umul24x24out: blkb 6,0

; unsigned multiplication of two 24-bit numbers giving a 48-bit result
; operation: UHL * UDE --> umul24x24out
umul24x24:
	push ix ; preserve
; point to output buffer and clear it
	ld ix,umul24x24out 
	push bc
	ld bc,0
	ld (ix),bc
	ld (ix+3),bc
	pop bc
; STEP 1: UHL * E
	ld a,e
	push hl
	call umul24x8
	ld (ix+0),hl
	ld (ix+3),a
; STEP 2: UHL * D
	pop hl
	push hl
	ld a,d
	call umul24x8
	call @accumulate
; STEP 3: UHL * DEU
	pop hl
	ld (@de),de
	ld a,(@de+2)
	call umul24x8
	call @accumulate
; all done
	pop ix ; restore
	ret
@accumulate:
	inc ix
; highest byte of product to carry
	ld (ix+3),a
; low byte of product
	ld a,l
	add a,(ix+0)
	ld (ix+0),a
; high byte of product
	ld a,h
	adc a,(ix+1)
	ld (ix+1),a
; uppper byte of product
	ld (@hl),hl
	ld a,(@hl+2)
	adc a,(ix+2)
	ld (ix+2),a
; carry
	ld a,0 ; preserve flags
	adc a,(ix+3)
	ld (ix+3),a
	ret

@ix: dl 0
@hl: dl 0
@de: dl 0

; UH.L = UH.L*UD.E (unsigned)
umul168:
	call umul24x24
	ld hl,(umul24x24out+1)
	ret

; UH.L * UD.E --> UH.L (signed)
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

;------------------------------------------------------------------------
;  arith24.asm 
;  24-bit ez80 arithmetic routines
;  Copyright (c) Shawn Sijnstra 2024
;  MIT license
;
;  This library was created as a tool to help make ez80
;  24-bit native assembly routines for simple mathematical problems
;  more widely available.
;  
;------------------------------------------------------------------------
;
;------------------------------------------------------------------------
; udiv24
; Unsigned 24-bit division
; Divides HLU by DEU. Gives result in DEU (and BC), remainder in HLU.
; 
; Uses AF BC DE HL
; Uses Restoring Division algorithm
;------------------------------------------------------------------------

udiv24:
	push	hl
	pop		bc	;move dividend to BCU
	ld		hl,0	;result
	and		a
	sbc		hl,de	;test for div by 0
	ret		z		;it's zero, carry flag is clear
	add		hl,de	;HL is 0 again
	ld		a,24	;number of loops through.
udiv1:
	push	bc	;complicated way of doing this because of lack of access to top bits
	ex		(sp),hl
	scf
	adc	hl,hl
	ex	(sp),hl
	pop	bc		;we now have bc = (bc * 2) + 1

	adc	hl,hl
	and	a		;is this the bug
	sbc	hl,de
	jr	nc,udiv2
	add	hl,de
;	dec	c
	dec	bc
udiv2:
	dec	a
	jr	nz,udiv1
	scf		;flag used for div0 error
	push	bc
	pop		de	;remainder
	ret