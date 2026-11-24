; One-frame MOS keyboard snapshot for bounded presentation states.
;
; This is the only device-facing half of the presentation input boundary.
; Controllers consume its normalized outputs and never poll, delay or wait.

; outputs: A = 0 when every mapped key is up, 1 when any key is down
;          C = START_CHOICE_NONE/YES/NO from the current ASCII key
; destroys: A,B,HL,IX; preserves DE/IY
presentation_input_poll:
; Capture current ASCII first; mos_getkbmap replaces IX with the held-key map.
    ld a,mos_sysvars
    rst.lil $08
    ld a,(ix+sysvar_keyascii)
    or 020h
    ld c,START_CHOICE_NONE
    cp 'y'
    jp z,@yes
    cp 'n'
    jp nz,@scan
    ld c,START_CHOICE_NO
    jp @scan
@yes:
    ld c,START_CHOICE_YES

@scan:
    MOSCALL mos_getkbmap
    push ix
    pop hl
    .if mus_all50
    ; M is a soundtrack command, not a Start/Continue key.
    ld a,(ix+music_key_byte)
    and 0ffh-[1<<music_key_bit]
    ld b,music_key_byte
@low:
    or (hl)
    inc hl
    djnz @low
    inc hl ; skip byte 12, already accumulated with M removed
    ld b,15-music_key_byte
    .else
    ld b,16
    xor a
    .endif
@byte:
    or (hl)
    inc hl
    djnz @byte
    jp z,@all_up
    ld a,1
    ret
@all_up:
    xor a
    ret
; end presentation_input_poll
