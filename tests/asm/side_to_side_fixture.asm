; Isolated CPU fixture ABI: record at 0x2000, B/C inputs at 0x2100/1.
    assume adl=1
    org 0
    ld sp,0xf000
    ld iy,0x2000
    ld a,(0x2100)
    ld b,a
    ld a,(0x2101)
    ld c,a
    ld a,(0x2102)
    or a
    jp z,@tick
    dec a
    jp z,@small
    dec a
    jp z,@medium
    dec a
    jp z,@large
    call side_to_side_init
    jp @done
@tick:
    call side_to_side
    jp @done
@small:
    call side_spawn_small
    jp @done
@medium:
    call side_spawn_medium
    jp @done
@large:
    call side_spawn_large
@done:
    inc a
    ld (0x2103),a
    halt
sprite_x: equ 1
sprite_y: equ 4
sprite_xvel: equ 7
sprite_yvel: equ 10
sprite_dim_x: equ 30
sprite_dim_y: equ 31
field_right: equ 255
field_bottom: equ 335
neg_hlu:
    push de
    ex de,hl
    ld hl,0
    or a
    sbc hl,de
    pop de
    ret
    include "../../src/asm/side_to_side.inc"
