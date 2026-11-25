; RAM-resident ADL code: bind the existing CALL operand only at level setup
; or at the last Earth seam. Ordinary gameplay executes its original CALL.
earth_first_buffer: equ 1280
earth_next_buffer: dl earth_first_buffer
earth_bind_background:
    ld hl,bg_plot
    ld a,(cur_baseBufferId+1)
    cp 6 ; Earth artwork bank 0600h, not an assumption about level order
    jp nz,@bind
    ld hl,earth_first_buffer
    ld (earth_next_buffer),hl
    ld hl,earth_first_terrain_line
@bind:
    ld (terrain_bg_call_operand),hl
    ret

earth_select:
    ld hl,(earth_next_buffer)
    ld (bg_bufferId),hl
    inc hl
    ld (earth_next_buffer),hl
    ret

; Extended row events: solitary seeker (either tileset), Earth march S/M/L.
earth_marker_callbacks:
    dl activate_null_tile,earth_launch_seeker,earth_launch_small,earth_launch_medium,earth_launch_large
earth_launch_seeker:
    ld bc,(tiles_x_plot-1)
    ld c,0
    ld de,0
    ld hl,seeker_defaults
    ld ix,seekers_base
    ld a,seekers_max
    call spawn_sprite_from_coords
    ret nc
    call sprite_center_on_tile
    ld (ix+seeker_spawn_pad_id),SEEKER_PAD_NONE
    xor a
    ld (ix+seeker_spawn_pad_generation),a
    ld (ix+seeker_completed_orbits),a
    ld hl,speed_seeker
    ld (ix+sprite_vel),hl
    push ix
    pop iy
    jp seeker_enter_live_launch
; Warp advances the shared starfield cursor. Reset it only on the first
; ordinary terrain scroll, then leave the normal Earth plot path bound.
earth_first_terrain_line:
    ld hl,earth_plot
    ld (terrain_bg_call_operand),hl
    ld hl,-1
    ld (bg_y_plot),hl
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
