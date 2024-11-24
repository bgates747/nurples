; add two 48-bit unsigned integers with carry
; and write a 56-bit result
; inputs: debc is the first 48-bit integer
;         iyix is the second 48-bit integer
; output: abcde is the 56-bit result
;         which is also held in RESULT_ADDR
; destroys: all registers
adc_48:
; write input parameters to scratch memory
    ld (OP_A_ADDR),bc
    ld (OP_A_ADDR+3),de
    ld (OP_B_ADDR),ix
    ld (OP_B_ADDR+3),iy
; set up pointers
    ld hl, OP_A_ADDR   ; HL points to the start of Operand A
    ld ix, OP_B_ADDR   ; IX points to the start of Operand B
    ld iy, RESULT_ADDR ; IY points to the result storage
    ld b,6             ; Counter for 6 bytes
@loop:
    ld a,(hl)         ; Load byte from Operand A
    adc a,(ix)        ; Add with carry byte from Operand B
    ld (iy),a         ; Store result byte
    inc hl             ; Move to next byte of Operand A
    inc ix             ; Move to next byte of Operand B
    inc iy             ; Move to next result storage location
    djnz @loop      ; Decrement counter and loop if not zero
; write overflow byte to result if any
    jr nc,@no_overflow
    ld a,1
    jr @write_a
@no_overflow:
    xor a ; clear carry and zero a
@write_a:
; load oveflow byte with value of a
    ld (iy),a
; load other return registers with result
    ld bc,(RESULT_ADDR)
    ld de,(RESULT_ADDR+3)
    ret                ; Procedure complete
OP_A_ADDR: ds 6
OP_B_ADDR: ds 6
RESULT_ADDR: ds 7

; subtract two 48-bit unsigned integers with carry
; inputs: debc is the first 48-bit integer
;         iyix is the second 48-bit integer
; output: bcde is the 48-bit result
;         which is also held in RESULT_ADDR
; destroys: all registers
sbc_48:
; write input parameters to scratch memory
    ld (OP_A_ADDR),bc
    ld (OP_A_ADDR+3),de
    ld (OP_B_ADDR),ix
    ld (OP_B_ADDR+3),iy
; set up pointers
    ld hl, OP_A_ADDR   ; HL points to the start of Operand A
    ld ix, OP_B_ADDR   ; IX points to the start of Operand B
    ld iy, RESULT_ADDR ; IY points to the result storage
    ld b,6             ; Counter for 6 bytes
@loop:
    ld a,(hl)         ; Load byte from Operand A
    sbc a,(ix)        ; subtract with carry byte from Operand B
    ld (iy),a         ; Store result byte
    inc hl             ; Move to next byte of Operand A
    inc ix             ; Move to next byte of Operand B
    inc iy             ; Move to next result storage location
    djnz @loop      ; Decrement counter and loop if not zero
; write overflow byte to result if any
    jr nc,@no_overflow
    ld a,1
    jr @write_a
@no_overflow:
    xor a ; clear carry and zero a
@write_a:
; load oveflow byte with value of a
    ld (iy),a
; load other return registers with result
    ld bc,(RESULT_ADDR)
    ld de,(RESULT_ADDR+3)
    ret                ; Procedure complete

; divide two 48-bit unsigned integers
; and get a 48-bit quotient and a 48-bit remainder
; inputs: debc is the 48-bit dividend
;         iyix is the 48-bit divisor
; output: debc is the 48-bit quotient
;         iyix is the 48-bit remainder
div_48:
; write input parameters to scratch memory
    ld (div_48_dividend),bc
    ld (div_48_dividend+3),de
    ld (div_48_divisor),ix
    ld (div_48_divisor+3),iy

    ld hl,0
    ld a,(div_48_dividend+5)
    ld b,8
@loop0:
    rla

div_48_dividend: ds 6
div_48_divisor: ds 6
div_48_quotient: ds 6
div_48_remainder: ds 6