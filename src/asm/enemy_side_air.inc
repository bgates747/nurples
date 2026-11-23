; Stateless pad callbacks: invoked once by do_active_tiles on row admission.
; Never allocate a collidable ground actor. All three share a bounded air pool.
air_max: equ 8
air_hp: equ actor_record_custom
air_visibility: equ air_hp+1

; Install immutable frame lists during sprite initialization, while inactive.
init_air_frames:
    ld iy,air_base
    ld b,air_max
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
    push af
    ld hl,(game_state)
    ld de,game_over
    or a
    sbc hl,de
    jp z,@no_launch
    ld hl,air_defaults
    ld bc,(tiles_x_plot-1)
    ld c,0
    ld de,0 ; top of the admitted row
    ld ix,air_base
    ld a,air_max
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
    call side_spawn_large
    ld hl,BUF_ENEMY_BIG_A1
    jp @frames
@medium:
    ld bc,0x3014 ; sideways 3, down 1.25
    call side_spawn_medium
    ld hl,BUF_ENEMY_MEDIUM_A1
    jp @frames
@small:
    ld bc,0x3818 ; sideways 3.5, down 1.5
    call side_spawn_small
    ld hl,BUF_ENEMY_SMALL_A1
@frames:
    ld (iy+air_visibility),a
    ld (iy+sprite_base_bufferId),hl
    ld de,BUF_ENEMY_BIG_A1
    or a
    sbc hl,de
    ld (iy+sprite_frame),l
    ret
@no_launch:
    pop af
    ret

move_air_enemy:
    call side_to_side
    cp side_exited
    jp z,table_deactivate_sprite
    or a
    jp z,@visible
; Positive campaign VY means a partially exiting actor cannot re-enter.
    ld a,(iy+air_visibility)
    or a
    ret nz
    ld (iy+air_visibility),side_hidden
    ld a,(iy+sprite_id)
    call vdu_sprite_select
    jp vdu_sprite_hide
@visible:
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
