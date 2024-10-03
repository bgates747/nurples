	.assume adl=1   
    .org 0x040000    

    jp start       

    .align 64      
    .db "MOS"       
    .db 00h         
    .db 01h       

start:              
    push af
    push bc
    push de
    push ix
    push iy

	call main

exit:
    pop iy
    pop ix
    pop de
    pop bc
    pop af
    ld hl,0
    ret

; API includes
    include "mos_api.inc"
    include "functions.inc"
	include "files.inc"
    include "fonts.inc"
    include "timer.inc"
    include "vdu.inc"
    include "vdu_fonts.inc"

; Application includes

main:
; 19    1024  768   4     60hz
    ld a,19
    call vdu_set_screen_mode

; inputs: hl = bufferId; iy = pointer to filename
    ld e,Lat2_TerminusBold32x16
    ld d,12 ; bytes per font list record
    mlt de
    ld iy,font_list
    add iy,de
    push iy

    ld iy,(iy+9)

; debug print filename at iy
    call printNewLine
    push iy
    pop hl
    call printString
    call printNewLine

    ld hl,0x4000
    push hl
    call vdu_load_buffer_from_file

; create font from buffer
; inputs: hl = bufferId, e = width, d = height, d = ascent, a = flags
; VDU 23, 0, &95, 1, bufferId; width, height, ascent, flags: Create font from buffer
    pop hl ; bufferId
    pop iy ; pointer to font list record
    push hl
    ld a,(iy+0)
    ld e,a  ; width
    ld a,(iy+3)
    ld d,a  ; height / ascent
    ld a,0 ; flags
    call vdu_font_create

; select font
; inputs: hl = bufferId, a = font flags
; Flags:
; Bit	Description
; 0	Adjust cursor position to ensure text baseline is aligned
;   0: Do not adjust cursor position (best for changing font on a new line)
;   1: Adjust cursor position (best for changing font in the middle of a line)
; 1-7	Reserved for future use
; VDU 23, 0, &95, 0, bufferId; flags: Select font
    pop hl
    ld a,0
    call vdu_font_select

; print test string
    call printNewLine
    ld hl,test_string
    call printString
    call printNewLine

; all done
    ret


test_string: db "The quick brown fox jumps over the lazy dog.",0