; Injected as the main call in a song-free, otherwise normally linked game.
; Write a four-byte MOS receipt: "M2", variant, checkpoint (42 = pass).
electrode_review:
    call tile_table_init
    xor a
    ld (tile_fault_flags),a
    ld hl,32
    ld (tiles_y_plot),hl
    ld hl,review_row
    ld (tiles_row_defs),hl
    ld a,1
    ld (tiles_cols),a
    call do_active_tiles                  ; register slot 15, actual ROM table
    ld a,(num_active_tiles)
    cp review_alias
    jp nz,review_save
    .if 1-review_alias
    ld a,11                              ; known positive control after no-op
    ld (review_row),a
    call do_active_tiles
    .endif
    ld a,2
    ld (review_step),a
    ld a,(num_active_tiles)
    cp 1
    jp nz,review_save
    ld iy,tile_table_base
    ld a,(iy+tile_type)
    cp tile_electrode+128
    jp nz,review_save
    call move_tiles                      ; normal admission clears just-spawned
    ld iy,tile_table_base
    ld hl,(iy+tile_move_program)
    call tile_move_program_valid
    jp nc,review_save
    ld a,3
    ld (review_step),a
    ld a,tile_lightning
    ld (review_row),a
    call do_active_tiles                 ; matching Y
    ld hl,64
    ld (tiles_y_plot),hl
    call do_active_tiles                 ; another independent barrier
    ld a,(num_active_tiles)
    cp 3
    jp nz,review_save
    ; Admit the two lightning records without simulating an unrelated play frame.
    ld ix,tile_table_base+tile_table_record_size
    res 7,(ix+tile_type)
    ld ix,tile_table_base+[2*tile_table_record_size]
    res 7,(ix+tile_type)
    ld iy,tile_table_base
    call kill_electrode
    ld a,4
    ld (review_step),a
    ld a,(num_active_tiles)
    cp 2
    jp nz,review_save
    ld iy,tile_table_base
    ld a,(iy+tile_type)
    cp tile_crater
    jp nz,review_save
    ld hl,(iy+tile_move_program)
    call tile_move_program_valid
    jp nc,review_save
    ld iy,tile_table_base+tile_table_record_size
    bit sprite_alive,(iy+tile_collisions)
    jp nz,review_save
    ld iy,(tile_stack+3)
    push iy
    pop hl
    ld de,tile_table_base+[2*tile_table_record_size]
    or a
    sbc hl,de
    jp nz,review_save
    ld a,(iy+tile_type)
    cp tile_lightning
    jp nz,review_save
    bit sprite_alive,(iy+tile_collisions)
    jp z,review_save
    ld a,(tile_fault_flags)
    or a
    jp nz,review_save
    ld a,42
    ld (review_step),a
review_save:
    ld hl,review_filename
    ld de,review_receipt
    ld bc,4
    MOSCALL mos_save
    ret
review_filename:
    .if review_alias
    asciz "ALIAS.OUT"
    .else
    asciz "CONTROL.OUT"
    .endif
review_row: db 15
review_receipt: db "M2",review_alias
review_step: db 1
