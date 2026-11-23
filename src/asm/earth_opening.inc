; RAM-resident ADL code: bind the existing CALL operand only at level setup
; or at the last Earth seam. Ordinary gameplay executes its original CALL.
earth_first_buffer: equ 1280
earth_next_buffer: dl earth_first_buffer
earth_open_frame: db 0
earth_fraction: db 0
earth_bind_background:
    ld hl,bg_plot
    ld a,(cur_level)
    or a
    jp nz,@bind
    ld hl,earth_first_buffer
    ld (earth_next_buffer),hl
    ld hl,earth_plot
@bind:
    ld (terrain_bg_call_operand),hl
    ret

earth_select:
    ld hl,(earth_next_buffer)
    ld (bg_bufferId),hl
    inc hl
    ld (earth_next_buffer),hl
    ret
earth_plot:
    ld a,(bg_y_plot)
    inc a
    ld (bg_y_plot),a
    jp nz,@plot
    ld hl,(earth_next_buffer)
    ld de,earth_first_buffer+16
    or a
    sbc hl,de
    jp nz,@earth
    ld hl,bg_plot
    ld (terrain_bg_call_operand),hl
    ld hl,-1
    ld (bg_y_plot),hl
    jp bg_plot
@earth:
    call earth_select
@plot:
    ld hl,(bg_bufferId)
    call vdu_buff_select
    ld bc,0
    ld de,(bg_y_plot)
    inc de
    jp vdu_plot_bmp

earth_begin_opening:
    xor a
    ld (earth_open_frame),a
    ld (earth_fraction),a
    ld hl,game_cutscene
    ld (game_state),hl
    ret

earth_opening_step:
    ld hl,earth_open_frame
    ld a,(hl)
    inc (hl)
    cp 40
    jp nc,@reveal
    srl a
    inc a
    cp 17
    jp c,starfield_warp_scroll
    ld a,16
    jp starfield_warp_scroll
@reveal:
    cp 40
    jp nz,@speed
    ld hl,-1
    ld (bg_y_plot),hl
@speed:
    ld b,16
    cp 52
    jp c,@draw
    ld b,12
    cp 58
    jp c,@draw
    ld b,8
    cp 63
    jp c,@draw
    ld b,4
    cp 68
    jp c,@draw
    ld b,2
    cp 72
    jp c,@draw
    ld b,1
    cp 76
    jp c,@draw
    jp z,earth_spawn
    ; 32 frames, speed 8/256 through 256/256. Only takeoff pays for this.
    sub 76
    cp 32
    jp nc,@handoff
    add a,a
    add a,a
    add a,a
    ld hl,earth_fraction
    add a,(hl)
    ld (hl),a
    ret nc
    call tiles_plot
    jp set_gfx_playing_field
@handoff:
    call tiles_plot
    call set_gfx_playing_field
    jp game_publish_playing
@draw:
    ; One or two clipped strips; never plot across a 256-line bitmap seam.
    ld a,b
    ld (earth_remaining),a
@piece:
    ld a,(bg_y_plot)
    cp 255
    call z,earth_select
    ld a,(earth_remaining)
    ld b,a
    ld a,(bg_y_plot)
    inc a
    neg
    jp z,@fits
    cp b
    jp nc,@fits
    ld b,a
@fits:
    ld a,b
    ld (starfield_warp_chunk),a
    ld hl,bg_y_plot
    add a,(hl)
    ld (hl),a
    ld a,(earth_remaining)
    sub b
    ld (earth_remaining),a
    call starfield_warp_render_chunk
    ld a,(earth_remaining)
    or a
    jp nz,@piece
    jp set_gfx_playing_field
earth_remaining: db 0

earth_spawn:
    ; Bottom 21 rows already painted; current map contains no opening markers.
    ld hl,(tiles_row_defs)
    ld de,21*16
    add hl,de
    ld (tiles_row_defs),hl
    ld a,235
    ld (tiles_row),a
    call player_init_respawn
    jp nc,presentation_flow_fail
    ld iy,player_begin
    ld hl,96*256
    ld (iy+sprite_x),hl
    ld hl,288*256
    ld (iy+sprite_y),hl
    ld hl,0
    ld (iy+sprite_yvel),hl
    ld a,player_grace_frames
    ld (iy+player_grace_timer),a
    ld hl,player_grace
    ld (player_state),hl
    jp player_draw_record
