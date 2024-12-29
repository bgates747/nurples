
screen_right: equ 511
screen_bottom: equ 383
origin_top: equ 48
origin_left: equ 128
field_top: equ 0
field_bottom: equ screen_bottom-origin_top
field_left: equ 0
field_right: equ 255
sprite_top: equ field_top
sprite_bottom: equ field_bottom-15
sprite_left: equ field_left
sprite_right: equ field_right-15
tile_bottom: equ screen_bottom+1

; set gfx origin and viewport to playing field window
set_gfx_playing_field:
    ld bc,origin_left
    ld de,origin_top
    call vdu_set_gfx_origin
    ld bc,field_left
    ld de,field_top
    ld ix,field_right
    ld iy,field_bottom
    call vdu_set_gfx_viewport
    ret
; end set_gfx_playing_field