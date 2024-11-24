; integer operations		
iand:	EQU 58	; AND (INTEGER)
ibdiv:	EQU 59	; DIV
ieor:	EQU 60	; EOR
imod:	EQU 61	; MOD
ior:	EQU 62	; OR
ile:	EQU 63	; <=
ine:	EQU 64	; <>
ige:	EQU 65	; >=
ilt:	EQU 66	; <
ieq:	EQU 67	; =
imul:	EQU 68	; *
iadd:	EQU 69	; +
igt:	EQU 70	; >
isub:	EQU 71	; -
ipow:	EQU 72	; ^
idiv:	EQU 73	; /
;		
; floating point functions		
absv:	EQU 16	; ABS
acs:	EQU 17	; ACS
asn:	EQU 18	; ASN
atn:	EQU 19	; ATN
cos:	EQU 20	; COS
deg:	EQU 21	; DEG
exp:	EQU 22	; EXP
int_:	EQU 23	; INT
ln:	    EQU 24	; LN
log:	EQU 25	; LOG
notk:	EQU 26	; NOT
rad:	EQU 27	; RAD
sgn:	EQU 28	; SGN
sin:	EQU 29	; SIN
sqr:	EQU 30	; SQR
tan:	EQU 31	; TAN
zero:	EQU 32	; ZERO
fone:	EQU 33	; FONE
true:	EQU 34	; TRUE
pi:	    EQU 35	; PI
val:	EQU 36	; VAL
str:	EQU 37	; STR$
sfix:	EQU 38	; FIX
sfloat:	EQU 39	; FLOAT
ftest:	EQU 40	; TEST
fcomp:	EQU 41	; COMPARE
;		
; floating point operations		
fand:	EQU  0	; AND (FLOATING-POINT)
fbdiv:	EQU  1	; DIV
feor:	EQU  2	; EOR
fmod:	EQU  3	; MOD
ffor:	EQU  4	; OR
fle:	EQU  5	; <=
fne:	EQU  6	; <>
fge:	EQU  7	; >=
flt:	EQU  8	; <
feq:	EQU  9	; =
fmul:	EQU 10	; *
fadd:	EQU 11	; +
fgt:	EQU 12	; >
fsub:	EQU 13	; -
fpow:	EQU 14	; ^
fdiv:	EQU 15	; /

    MACRO LOAD_FLOAT ARG
    ld ix,$+11
    call val_fp ; convert the string to a float
    jp (ix)
    asciz ARG
    ENDMACRO

; --- originally in eval.asm ---
;SWAP - Swap arguments
;Exchanges DE,HL D'E',H'L' and B,C
;Destroys: A,B,C,D,E,H,L,D',E',H',L'
;
SWAP:			LD      A,C
			LD      C,B
			LD      B,A
			EX      DE,HL
			EXX
			EX      DE,HL
			EXX
			RET

; same as VAL_FP in fpp.asm, but preserves any float stored in DED'E'B
val_fp:
    push iy ; preserve
    ld iy,@val
    call store_float_iy_alt
    ld a,val
    call FPP ; string converted to float in HLH'L'C
    ld iy,@val
    call fetch_float_iy_alt
    pop iy ; restore
    ret
@val: ds 5

; same as INT_FP_ in fpp.asm but preserves B, which keeps any float stored in DED'E'B intact
; af will also return differently from the original
;INT - Floor function
;Result is integer numeric.
;
int_fp_:		
    push bc ; preserve b
    ld a,int_
    call FPP
    pop af ; restore b to a
    ld b,a ; restore b
    RET

; convert floating point number to integer and store it in HLU
; inputs: HLH'L'C = floating point number
; outputs: HLU = integer part of the number
; destroys: all except DEU and U'D'E'B, index registers
int2hlu:
    call int_fp_
    ld a,l
    push af
    exx
    push hl
    exx
    pop hl
    pop af
    call A_TO_HLU
    ret

; convert polar coordinates to cartesian coordinates as deltas from the origin
; inputs: HLH'L'C = angle in radians
;         DED'E'B = radius
; outputs: HLH'L'C = dx
;          DED'E'B = dy
polar_to_cartesian_fpp:
; store input parameters in scratch
    ld iy,@angle
    call store_float_iy_nor
    ld iy,@radius
    call store_float_iy_alt
; compute dy = sin(angle) * radius
    ld a,sin
    call FPP ; HLH'L'C = sin(angle)
    ld iy,@radius
    call fetch_float_iy_alt ; DED'E'B = radius
    ld a,fmul
    call FPP ; HLH'L'C = sin(angle) * radius
    ld iy,@scratch
    call store_float_iy_nor ; @scratch = dy
; compute dx = cos(angle) * radius
    ld iy,@angle
    call fetch_float_iy_nor
    ld a,cos
    call FPP ; HLH'L'C = cos(angle)
    ld iy,@radius
    call fetch_float_iy_alt ; DED'E'B = radius
    ld a,fmul
    call FPP ; HLH'L'C = cos(angle) * radius
; restore dy from @scratch
    ld iy,@scratch
    call fetch_float_iy_alt
    ret
@angle: ds 5
@radius: ds 5
@scratch: ds 5

; store HLH'L'C floating point number in a 40-bit buffer
; inputs: HLH'L'C = floating point number
;         ix = buffer address
; outputs: buffer filled with floating point number
; destroys: nothing
store_float_nor:
    ld (ix+0),c
    ld (ix+3),l
    ld (ix+4),h
    exx
    ld (ix+1),l
    ld (ix+2),h
    exx
    ret

; fetch HLH'L'C floating point number from a 40-bit buffer
; inputs: ix = buffer address
; outputs: HLH'L'C = floating point number
; destroys: HLH'L'C
fetch_float_nor:
    ld c,(ix+0)
    ld l,(ix+3)
    ld h,(ix+4)
    exx
    ld l,(ix+1)
    ld h,(ix+2)
    exx
    ret

; store DED'E'B floating point number in a 40-bit buffer
; inputs: DED'E'B = floating point number
;         ix = buffer address
; outputs: buffer filled with floating point number
; destroys: nothing
store_float_alt:
    ld (ix+0),b
    ld (ix+3),e
    ld (ix+4),d
    exx
    ld (ix+1),e
    ld (ix+2),d
    exx
    ret

; fetch DED'E'B floating point number from a 40-bit buffer
; inputs: ix = buffer address
; outputs: DED'E'B = floating point number
; destroys: DED'E'B
fetch_float_alt:
    ld b,(ix+0)
    ld e,(ix+3)
    ld d,(ix+4)
    exx
    ld e,(ix+1)
    ld d,(ix+2)
    exx
    ret


; store HLH'L'C floating point number in a 40-bit buffer
; inputs: HLH'L'C = floating point number
;         iy = buffer address
; outputs: buffer filled with floating point number
; destroys: nothing
store_float_iy_nor:
    ld (iy+0),c
    ld (iy+3),l
    ld (iy+4),h
    exx
    ld (iy+1),l
    ld (iy+2),h
    exx
    ret

; fetch HLH'L'C floating point number from a 40-bit buffer
; inputs: iy = buffer address
; outputs: HLH'L'C = floating point number
; destroys: HLH'L'C
fetch_float_iy_nor:
    ld c,(iy+0)
    ld l,(iy+3)
    ld h,(iy+4)
    exx
    ld l,(iy+1)
    ld h,(iy+2)
    exx
    ret

; store DED'E'B floating point number in a 40-bit buffer
; inputs: DED'E'B = floating point number
;         iy = buffer address
; outputs: buffer filled with floating point number
; destroys: nothing
store_float_iy_alt:
    ld (iy+0),b
    ld (iy+3),e
    ld (iy+4),d
    exx
    ld (iy+1),e
    ld (iy+2),d
    exx
    ret

; fetch DED'E'B floating point number from a 40-bit buffer
; inputs: iy = buffer address
; outputs: DED'E'B = floating point number
; destroys: DED'E'B
fetch_float_iy_alt:
    ld b,(iy+0)
    ld e,(iy+3)
    ld d,(iy+4)
    exx
    ld e,(iy+1)
    ld d,(iy+2)
    exx
    ret

; print HLH'L'C floating point number in hex format
; inputs: HLH'L'C = floating point number
print_float_hex_nor:
    PUSH_ALL
    push bc ; preserve c
    call printHex16
    ld a,'.'
    rst.lil 10h
    exx 
    call printHex16
    exx
    ld a,'^'
    rst.lil 10h
    pop bc ; restore c
    ld a,c
    call printHex8
    POP_ALL
    ret

; print DED'E'B floating point number in hex format
; inputs: DED'E'B = floating point number
print_float_hex_alt:
    PUSH_ALL
    push bc ; preserve b
    ex de,hl
    call printHex16
    ex de,hl
    ld a,'.'
    rst.lil 10h
    exx 
    ex de,hl
    call printHex16
    ex de,hl
    exx
    ld a,'^'
    rst.lil 10h
    pop af ; restore b to a
    call printHex8
    POP_ALL
    ret

print_floats_hex:
    call print_float_hex_nor
    ld a,' '
    rst.lil 10h
    call print_float_hex_alt
    ret

; print a floating point number in decimal format
; inputs: HL'H'L'C is the number to print
; outputs: number printed to screen
;          ACCS null-terminated string representation of the number
; destroys: everything except ix
print_float_dec:
print_float_dec_nor:
    push ix             ; preserve

; back up floats in normal and alternate registers
    ld ix,@float_nor   
    call store_float_nor
    ld ix,@float_alt
    call store_float_alt

; convert the number to a string
    ld de,ACCS          ; point to the string accumulator
    ld ix,@G9-1         ; get the format code for the number
    call STR_FP         ; convert the number to a string
    ex de,hl            ; point to end of the string
    ld (hl),0           ; null-terminate the string
    ld hl,ACCS          ; point to the string accumulator
    call printString

; restore floats in normal and alternate registers
    ld ix,@float_nor
    call fetch_float_nor
    ld ix,@float_alt
    call fetch_float_alt

; cleanup and go home
    pop ix              ; restore
    ret
@G9:			DW    9
@float_nor: ds 5
@float_alt: ds 5

print_float_dec_alt:
    CALL SWAP
    CALL print_float_dec_nor
    CALL SWAP
    ret

;PI - Return PI (3.14159265)
;Result is floating-point numeric.
;
pi_alt:			LD      DE,0x490F
        		EXX
        		LD      DE,0xDAA2
        		EXX
        		LD      B,0x81
        		XOR     A               ;NUMERIC MARKER
        		RET

;PI - Return PI * 2 (6.28318531)
;Result is floating-point numeric.
;
pi2_alt:		LD      DE,0x490F
        		EXX
        		LD      DE,0xDAA3
        		EXX
        		LD      B,0x82
        		XOR     A               ;NUMERIC MARKER
        		RET