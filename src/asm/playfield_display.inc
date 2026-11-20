; Display checkpoints identify a failed handoff after restoring the MOS screen.
PLAYFIELD_MODE: equ 20
PLAYFIELD_COLOURS: equ 64
PLAYFIELD_CHECK_LOAD: equ 1
PLAYFIELD_CHECK_ATTRACT: equ 2
PLAYFIELD_CHECK_START: equ 3
PLAYFIELD_CHECK_GAME_OVER: equ 4

PLAYFIELD_DISPLAY_MISMATCH: equ 4
playfield_display_status: db 0 ; zero = no failure, otherwise protocol error or mismatch
playfield_display_checkpoint: db 0
playfield_display_mode: db 0
playfield_display_colours: db 0
playfield_display_width: dl 0
playfield_display_height: dl 0

; Reuse a matching live mode. Carry set only for the expected physical surface.
playfield_display_select:
    ld a,PLAYFIELD_CHECK_LOAD
    ld (playfield_display_checkpoint),a
    call vdu_query_mode
    jp nc,playfield_display_failed
    ld a,(ix+sysvar_scrMode)
    cp PLAYFIELD_MODE
    jp z,playfield_display_validate
    ld a,PLAYFIELD_MODE
    call vdu_set_mode_confirm
    jp nc,playfield_display_failed
    jp playfield_display_validate
; end playfield_display_select

playfield_display_verify:
    ld (playfield_display_checkpoint),a
    call vdu_query_mode
    jp nc,playfield_display_failed
; end playfield_display_verify

playfield_display_validate:
    ld a,(ix+sysvar_scrMode)
    ld (playfield_display_mode),a
    ld a,(ix+sysvar_scrColours)
    ld (playfield_display_colours),a
    ld hl,0
    ld l,(ix+sysvar_scrWidth)
    ld h,(ix+sysvar_scrWidth+1)
    ld (playfield_display_width),hl
    ld hl,0
    ld l,(ix+sysvar_scrHeight)
    ld h,(ix+sysvar_scrHeight+1)
    ld (playfield_display_height),hl
    ld de,screen_bottom+1
    or a
    sbc hl,de
    jp nz,@mismatch
    ld hl,(playfield_display_width)
    ld de,screen_right+1
    or a
    sbc hl,de
    jp nz,@mismatch
    ld a,(playfield_display_mode)
    cp PLAYFIELD_MODE
    jp nz,@mismatch
    ld a,(playfield_display_colours)
    cp PLAYFIELD_COLOURS
    jp nz,@mismatch
    scf
    ret
@mismatch:
    ld a,PLAYFIELD_DISPLAY_MISMATCH
playfield_display_failed:
    ld (playfield_display_status),a
    or a
    ret
; end playfield_display_validate

; Called after the previous MOS mode and legible system text are restored.
playfield_display_report:
    ld a,(playfield_display_status)
    or a
    jp nz,@failure
    .if GAME_OVER_REVIEW_TRIGGER
    ld a,(playfield_display_checkpoint)
    cp PLAYFIELD_CHECK_GAME_OVER
    ret nz
    call printInline
    asciz "Game Over display: "
    jp @mode
    .else
    ret
    .endif
@failure:
    call printInline
    asciz "Playfield display failed at check "
    ld hl,0
    ld a,(playfield_display_checkpoint)
    ld l,a
    call printDec
    call printInline
    asciz ": "
    ld a,(playfield_display_status)
    cp VDU_MODE_FENCE_TIMEOUT
    jp z,@fence_timeout
    cp VDU_MODE_ACK_TIMEOUT
    jp z,@ack_timeout
    cp VDU_MODE_QUERY_TIMEOUT
    jp nz,@mode
    call printInline
    asciz "VDP mode query timed out.\r\n"
    ret
@fence_timeout:
    call printInline
    asciz "VDP cursor fence timed out.\r\n"
    ret
@ack_timeout:
    call printInline
    asciz "MODE 20 acknowledgement timed out.\r\n"
    ret
@mode:
    call printInline
    asciz "mode "
    ld hl,0
    ld a,(playfield_display_mode)
    ld l,a
    call printDec
    call printInline
    asciz ", "
    ld hl,(playfield_display_width)
    call printDec
    ld a,'x'
    rst.lil 10h
    ld hl,(playfield_display_height)
    call printDec
    call printInline
    asciz ", colours "
    ld hl,0
    ld a,(playfield_display_colours)
    ld l,a
    call printDec
    jp printNewLine
; end playfield_display_report
