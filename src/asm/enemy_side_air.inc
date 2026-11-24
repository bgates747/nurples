; Stateless pad callbacks: invoked once by do_active_tiles on row admission.
; Never allocate a collidable ground actor. All three share a bounded air pool.
air_max: equ 8
formation_max: equ 40
formation_live: db 0
air_hp: equ actor_record_custom
air_visibility: equ air_hp+1
earth_first_leg: equ air_visibility+1
earth_full_leg: equ air_visibility+2
earth_direction: equ air_visibility+3

; Install immutable frame lists during sprite initialization, while inactive.
init_air_frames:
    ld iy,formation_base
    ld b,formation_max
    call init_air_frame_group
    ld iy,air_base
    ld b,air_max
init_air_frame_group:
@sprite:
    push bc
    push iy
    ld a,(iy+sprite_id)
    call vdu_sprite_select
    call vdu_sprite_clear_frames
    ld hl,BUF_ENEMY_BIG_A1
    ld b,6 ; big A1/A2, medium A1/A2, small A1/A2
@frame:
    push bc
    push hl
    call vdu_sprite_add_buff
    pop hl
    pop bc
    inc hl
    djnz @frame
    pop iy
    lea iy,iy+table_record_size
    pop bc
    djnz @sprite
    ret

air_defaults:
    db enemy_air
    dl BUF_ENEMY_SMALL_A1,move_air_enemy
    db %01000000,16,16,1,2,0,8,0,0,10,-4,0

activate_pad_small:
    ld a,1
    jp air_launch
activate_pad_medium:
    ld a,2
    jp air_launch
activate_pad_large:
    ld a,3
air_launch:
    ld ix,air_base
    ld b,air_max
air_launch_pool:
    push af
    ld hl,(game_state)
    ld de,game_over
    or a
    sbc hl,de
    jp z,@no_launch
    ld hl,air_defaults
    ld a,b
    ld bc,(tiles_x_plot-1)
    ld c,0
    ld de,0 ; top of the admitted row
    call spawn_sprite_from_coords
    jp nc,@no_launch
    push ix
    pop iy
    pop af
    ld (iy+air_hp),a
    cp 1
    jp z,@small
    cp 2
    jp z,@medium
    ld bc,0x180D ; sideways 1.5, down 13/16 (nearest to 0.8)
    ld de,air_size_large
    ld hl,BUF_ENEMY_BIG_A1
    jp @frames
@medium:
    ld bc,0x3014 ; sideways 3, down 1.25
    ld de,air_size_medium
    ld hl,BUF_ENEMY_MEDIUM_A1
    jp @frames
@small:
    ld bc,0x3818 ; sideways 3.5, down 1.5
    ld de,air_size_small
    ld hl,BUF_ENEMY_SMALL_A1
@frames:
    push hl ; selected bitmap
    ld (ix+sprite_dim_x),d
    ld (ix+sprite_dim_y),e
    push de
    call sprite_center_on_tile
    pop de
    ld hl,(ix+sprite_x)
    push hl ; constructor's travel clamp must not change the spawn anchor
    call side_spawn_inward
    pop de
    ld hl,(iy+sprite_x)
    or a
    sbc hl,de
    jr z,@centered
    ld (iy+sprite_x),de
    ld a,side_hidden
@centered:
    pop hl
    ld (iy+air_visibility),a
    ld (iy+sprite_base_bufferId),hl
    ld de,BUF_ENEMY_BIG_A1
    or a
    sbc hl,de
    ld (iy+sprite_frame),l
    scf ; successful constructor: IY is the allocated air record
    ret
@no_launch:
    pop af
    or a ; do not let an Earth marker customize an unallocated record
    ret

earth_launch_small:
    ld a,1
    jp earth_launch_march
earth_launch_medium:
    ld a,2
    jp earth_launch_march
earth_launch_large:
    ld a,3
earth_launch_march:
    push af
    ld a,(tiles_row)
    ld hl,earth_rank_programs
    ld b,earth_rank_count
    inc b
    dec b
    jr z,@absent
@find:
    cp (hl)
    jr z,@found
    ld de,5
    add hl,de
    djnz @find
@absent:
    pop af
    or a
    ret
@found:
    pop af
    push hl
    ld ix,formation_base
    ld b,formation_max
    call air_launch_pool
    pop hl
    ret nc
    inc hl
    ld a,(hl)
    ld (iy+sprite_move_timer),a
    inc hl
    ld a,(hl)
    ld (iy+earth_first_leg),a
    inc hl
    ld a,(hl)
    ld (iy+earth_full_leg),a
    inc hl
    ld a,(hl)
    ld (iy+earth_direction),a
    ld hl,formation_live
    inc (hl)
    ld hl,move_earth_air
    ld (iy+sprite_move_program),hl
    ld hl,0
    ld (iy+sprite_xvel),hl
    ld hl,128 ; no initial wait: begin at march speed
    ld (iy+sprite_move_step),0
    ld a,(iy+sprite_move_timer)
    or a
    jr z,@wait
    ld hl,256 ; during the wait, follow each terrain scanline exactly
@wait:
    ld (iy+sprite_yvel),hl
    ret

move_earth_air:
    call earth_march_tick
    jp air_after_motion

    include "earth_march.inc"

move_air_enemy:
    call side_to_side
air_after_motion:
    cp side_exited
    jp z,table_deactivate_sprite
    or a
    jp z,@visible
; A centered launch above the top remains hidden until fully inside.
    ld a,(iy+air_visibility)
    or a
    ret nz
    ld (iy+air_visibility),side_hidden
    ld a,(iy+sprite_id)
    call vdu_sprite_select
    jp vdu_sprite_hide
@visible:
    ld a,(iy+air_visibility)
    or a
    jr z,@animate
    call sprite_footprint_within_playfield
    ret nc ; also guard a wide actor centered on a side-edge tile
    ld (iy+air_visibility),side_visible
    ; Use the normal frame/position/show publication in move_sprite_group.
    set actor_type_just_spawned_bit,(iy+sprite_type)
@animate:
    dec (iy+sprite_animation_timer)
    jp nz,@contacts
    ld (iy+sprite_animation_timer),8
    ld a,(iy+sprite_frame)
    xor 1
    ld (iy+sprite_frame),a
@contacts:
    ld ix,player_begin
    bit sprite_alive,(ix+sprite_collisions)
    jp z,@laser
    call check_enemy_collision_player
    jp nz,@laser
    call update_shields
    jp vox_contact_explosion
@laser:
    call check_enemy_collision_laser
    ret nz
    dec (iy+air_hp)
    jp z,@killed
    res sprite_just_died,(iy+sprite_collisions) ; shot consumed, target survives
    ret
@killed:
    call update_score
    jp spawn_explosion
