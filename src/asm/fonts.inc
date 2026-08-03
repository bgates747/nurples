fonts_load:
    ld ix,font_list ; pointer to font list lookup
    ld b,num_fonts ; loop counter

@load_loop:
    push bc ; save loop counter

; load font into a buffer
    ld hl,(ix+font_list_bufferId)
    ld iy,(ix+font_list_filename)
    push ix ; mos_fread does not preserve the font-list cursor
    call vdu_load_buffer_from_file
    pop ix
    jr nc,@load_failed

; create font from buffer
    ld hl,(ix+font_list_bufferId)
    ld e,(ix+font_list_width)
    ld d,(ix+font_list_height)
    ld a,0 ; flags
    call vdu_font_create

; select font
    ld hl,(ix+font_list_bufferId)
    ld a,1 ; flags
    call vdu_font_select

; Report only a fully uploaded, created, and selected font.
    ld a,'.'
    rst.lil 10h

; ; debug print filename
;     call printNewLine
;     ld hl,(ix+font_list_filename)
;     call printString

; advance font_list pointer to next record
@next_font:
    lea ix,ix+font_list_record_size

; restore loop counter
    pop bc
    djnz @load_loop

; all done: every font completed successfully
    scf
    ret

@load_failed:
; Balance the loop counter saved by this iteration and publish failure.
    pop bc
    or a ; clear carry
    ret
