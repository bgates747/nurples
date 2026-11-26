; Eirene RC3 / EMOS UART0 ABI. Packets stay immutable through the callback.
batch_row_mode: equ 0

batch_a: db 23,0,0a0h
    dw earth_first_buffer
    db 5,0c2h
    dw 0,1024
    ds 1024
batch_b: db 23,0,0a0h
    dw earth_first_buffer
    db 5,0c2h
    dw 0,1024
    ds 1024
batch_next: dl 0
batch_current: dl 0
batch_done: db 0
batch_sent: db 0

batch_open:
    xor a
    ld (jit_cached),a
    ld hl,batch_a
    ld (batch_next),hl
batch_read:
    call jit_time_begin
    ld hl,(batch_next)
    ld de,11
    add hl,de
    ex de,hl
    ld bc,1024
    call agnb_read_exact
    ret nz
    ld ix,jit_read_max
    call jit_time_end
    xor a
    ret

batch_submit:
    xor a
    ld (batch_done),a
    inc a
    ld (batch_sent),a
    ld hl,(batch_current)
    .if batch_row_mode
    ; The 11 bytes before this row are either reserved space (first
    ; row) or already-consumed pixels. No 256-byte packing copy.
    push hl
    ex de,hl
    ld hl,batch_header
    ld bc,11
    ldir
    pop hl
    ld bc,267
    .else
    ld bc,1035
    .endif
    ld de,batch_callback
    ld a,025h
    rst.lil 08h
    ret
    .if batch_row_mode
batch_header: db 23,0,0a0h
    dw earth_first_buffer
    db 5,0c2h
    dw 0,256
    .endif
batch_callback:
    ld a,1
    ld (batch_done),a
    ret
batch_join:
    ld a,(batch_done)
    or a
    jr z,batch_join
@drain:
    in0 a,(0c5h)
    bit 6,a
    jr z,@drain
    ret

jit_plot:
    ld a,(jit_cached)
    or a
    jr nz,@row
    call lp_poll
    ld hl,(batch_next)
    ld (batch_current),hl
    ld de,batch_a
    or a
    sbc hl,de
    jr nz,@swap
    ld de,batch_b
@swap:
    ld (batch_next),de
    ld a,4
    ld (jit_cached),a
    xor a
    ld (batch_sent),a
    call jit_time_begin
    ld hl,(jit_start_irq)
    push hl
    ld hl,(jit_start_count)
    push hl
    ld a,(scanout_off)
    bit 0,a
    call z,batch_submit
    ld hl,(jit_rows_left)
    ld de,4
    or a
    sbc hl,de
    call nz,batch_read ; independent SD source, no VDU traffic here
    push af
    ld a,(batch_sent)
    or a
    call nz,batch_join
    pop af
    pop hl
    ld (jit_start_count),hl
    pop hl
    ld (jit_start_irq),hl
    jp nz,jit_fail
    ld ix,jit_send_max
    call jit_time_end
    call lp_poll
@row:
    ld a,(scanout_off)
    bit 0,a
    .if batch_row_mode
    jr nz,@advance
    .else
    jp nz,jit_consume_row
    .endif
    ld a,(batch_sent)
    or a
    jr nz,@draw
    call batch_submit ; background re-enabled partway through a group
    call batch_join
@draw:
    ld hl,earth_first_buffer
    call vdu_buff_select
    .if batch_row_mode
    ld de,0
    .else
    ld hl,1
    ld de,0
    ld a,(jit_cached)
    ld e,a
    or a
    sbc hl,de
    ex de,hl
    .endif
    ld bc,0
    call vdu_plot_bmp
    .if batch_row_mode
@advance:
    ld hl,(batch_current)
    ld de,256
    add hl,de
    ld (batch_current),hl
    xor a
    ld (batch_sent),a
    .endif
    jp jit_consume_row
