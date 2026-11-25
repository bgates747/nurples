; Single-row Earth experiment, included only by --earth-jit.
; AGNB's FIL and read window are free after the resident image/voice loads.
jit_filename: asciz "earth.agnb"
jit_owned: db 0
jit_error: db 0
jit_proven: db 0
jit_cached: db 0
jit_cursor: dl 0
jit_rows_left: dl 0
jit_read_max: dl 0
jit_send_max: dl 0
jit_start_irq: dl 0
jit_start_count: dl 0

jit_open:
    ld de,jit_filename
    call agnb_open
    ret nz
    ld a,1
    ld (jit_owned),a
    call agnb_read_riff_header
    ret nz
    call agnb_read_vers
    ret nz
    call agnb_read_buffer_list ; validates DATA length vs dimensions; leaves FIL at first pixel
    ret nz
    ld hl,(agnb_metadata+agnb_meta_width)
    ld de,256
    or a
    sbc.s hl,de
    jr nz,@bad
    ld hl,(agnb_metadata+agnb_meta_height)
    ld de,14*256
    or a
    sbc.s hl,de
    jr nz,@bad
    ld hl,(agnb_metadata+agnb_meta_bufferId)
    ld de,earth_first_buffer
    or a
    sbc.s hl,de
    jr nz,@bad
    ; No siblings, no padding: this fixed-size DATA must end at physical EOF.
    ld hl,(agnb_state+agnb_state_file_offset)
    ld de,14*65536
    add hl,de
    ld de,(agnb_state+agnb_state_file_size)
    or a
    sbc hl,de
    jr nz,@bad
    ld a,(agnb_state+agnb_state_file_size+3)
    or a
    jr nz,@bad
    ld hl,14*256
    ld (jit_rows_left),hl
    jp jit_refill
@bad:
    ld a,agnb_error_metadata
    or a
    ret

jit_close:
    ld a,(jit_owned)
    or a
    ret z
    call agnb_close
    ret nz
    ld (jit_owned),a
    ret

jit_refill:
    call lp_poll
    call jit_time_begin
    ld bc,1024
    ld de,agnb_io_buffer
    call agnb_read_exact
    ret nz
    ld ix,jit_read_max
    call jit_time_end
    call lp_poll
    ld hl,agnb_io_buffer
    ld (jit_cursor),hl
    ld a,4
    ld (jit_cached),a
    xor a
    ret

jit_plot:
    ld a,(jit_cached)
    or a
    jr nz,@ready
    call jit_refill
    jp nz,jit_fail
@ready:
    ld a,(scanout_off)
    bit 0,a
    jr nz,@consume ; logical travel still consumes exactly one row
    call jit_time_begin
    ld hl,(jit_cursor)
    call jit_upload
    ld bc,0
    ld de,0
    call vdu_plot_bmp
    ld ix,jit_send_max
    call jit_time_end
    call lp_poll
@consume:
    ld hl,(jit_cursor)
    ld de,256
    add hl,de
    ld (jit_cursor),hl
    ld hl,jit_cached
    dec (hl)
    ld hl,(jit_rows_left)
    dec hl
    ld (jit_rows_left),hl
    ld a,h
    or l
    ret nz
    ; The NEXT terrain scroll enters pinned 14 through the control renderer.
    ld hl,earth_first_terrain_line
    ld (terrain_bg_call_operand),hl
    call jit_close
    ret z
jit_fail:
    ld (jit_error),a
    ld a,1
    ld (game_exit_requested),a
    or a ; carry clear
    ret

; HL = 256 bytes. Never pump music inside the VDU command + immediate operands.
jit_upload:
    push hl
    ld hl,@adjust
    ld bc,@end-@adjust
    rst.lil 18h
    pop hl
    ld bc,256
    rst.lil 18h
    ld hl,earth_first_buffer
    jp vdu_buff_select
@adjust: db 23,0,0a0h
    dw earth_first_buffer
    db 5,0c2h ; SET multiple targets from multiple inline operands
    dw 0,256
@end:

; Once per invocation, before the stock bezel redraw. Queue white then red
; using the actual upload/plot path. No fence between them. After both, query
; the framebuffer; failure is reported after returning to the MOS display.
jit_probe:
    ld a,(jit_proven)
    or a
    scf
    ret nz
    ld a,0ffh
    call jit_solid
    ld bc,0
    ld de,0
    call vdu_plot_bmp
    ld a,0c3h
    call jit_solid
    ld bc,0
    ld de,1
    call vdu_plot_bmp
    xor a
    call jit_read_pixel
    jr nc,@timeout
    ld a,(ix+sysvar_scrpixel)
    and (ix+sysvar_scrpixel+1)
    and (ix+sysvar_scrpixel+2)
    jr z,@wrong
    ld a,1
    call jit_read_pixel
    jr nc,@timeout
    ld a,(ix+sysvar_scrpixel+1) ; red and blue channel order immaterial; green must be off
    or a
    jr nz,@wrong
    ld a,(ix+sysvar_scrpixel)
    or (ix+sysvar_scrpixel+2)
    jr z,@wrong
    call vdu_cls
    ld a,1
    ld (jit_proven),a
    scf
    ret
@timeout:
    ld a,0e1h
    jp jit_fail
@wrong:
    ld a,0e2h
    jp jit_fail
jit_solid:
    ld hl,agnb_io_buffer
    ld (hl),a
    ld de,agnb_io_buffer+1
    ld bc,255
    ldir
    ld hl,agnb_io_buffer
    jp jit_upload
jit_read_pixel:
    ld (@py),a
    MOSCALL mos_sysvars
    res 2,(ix+sysvar_vpd_pflags)
    push ix
    ld hl,@cmd
    ld bc,7
    rst.lil 18h
    pop ix
    ld d,vdp_pflag_point
    jp vdu_wait_mode_protocol_flag
@cmd: db 23,0,84h
    dw 3
@py: dw 0

; Sample the existing music PRT; no timer of our own. Ignore intervals when
; M has disabled it. Maxima are submission/SD wall time, not VDP render time.
jit_clock:
    ld bc,(lp_irqs)
    ld de,0
    in0 e,(084h)
    in0 d,(085h)
    ld hl,(lp_irqs)
    or a
    sbc hl,bc
    jr nz,jit_clock
    ret
jit_time_begin:
    ld a,(lp_owned)
    or a
    ret z
    call jit_clock
    ld (jit_start_irq),bc
    ld (jit_start_count),de
    ret
jit_time_end:
    ld a,(lp_owned)
    or a
    ret z
    call jit_clock
    ld hl,(jit_start_count)
    or a
    sbc hl,de
    push hl
    push bc
    pop hl
    ld de,(jit_start_irq)
    or a
    sbc hl,de
    ex de,hl
    pop hl
    ld bc,lp_reload
@period:
    ld a,d
    or e
    jr z,@max
    add hl,bc
    dec de
    jr @period
@max:
    ld de,0
    or a
    sbc hl,de ; sample taken across a pending IRQ before ISR accounting
    ret m
    ld de,(ix+0)
    or a
    sbc hl,de
    ret c
    add hl,de
    ld (ix+0),hl
    ret
jit_report:
    ld a,(jit_error)
    or a
    jr z,@metrics
    push af
    call printInline
    asciz "Earth JIT failed: "
    pop af
    call printHex8
    call printNewLine
@metrics:
    call printInline
    asciz "Earth max SD / row submit (72 counts/ms): "
    ld hl,(jit_read_max)
    call printDec
    ld a,'/'
    rst.lil 10h
    ld hl,(jit_send_max)
    call printDec
    jp printNewLine
