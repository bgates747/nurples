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
    ld e,Lat15_VGA16
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


; test_string: db "01234567890!@#$%^&*()\r\nabcdefghijklmnopqrstuvwxyz\r\nABCDEFGHIJKLMNOPQRSTUVWXYZ",0
test_string:
    db 0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,13,10
    db 0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,13,10
    db 0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,13,10
    db 0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,13,10
    db 0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,13,10
    db 0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,13,10
    db 0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,13,10
    db 0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,13,10
    db 0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,13,10
    db 0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,13,10
    db 0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,13,10
    db 0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,13,10
    db 0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,13,10
    db 0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF,13,10
    db 0x00