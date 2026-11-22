fireball_custom_begin: equ actor_record_custom
fireball_ground_vy: equ fireball_custom_begin ; immutable signed 16.8
fireball_custom_end: equ fireball_ground_vy+3
    .if fireball_custom_end-fireball_custom_begin-3
        dl fireball_custom_span_mismatch
    .endif

speed_fireball: equ 4*256 ; 6 pixels per frame 16.8 fixed
fireballs_max: equ 4

fireball_defaults:
    db enemy_projectile     ; type
    dl BUF_FIREBALL_2_000   ; base_bufferId
    dl move_enemy_fireball  ; move_program
    db %01000000            ; collisions bit 6 signals sprite alive
    db 16                   ; dim_x
    db 16                   ; dim_y
    db 32                   ; num_orientations
    db 2                    ; num_animations
    db 0                    ; animation
    db 8                    ; animation_timer
    db 0                    ; move_timer
    db 0                    ; move_step
    db 0                    ; points
    db -2                   ; shield_damage
    db 0                    ; frame

enemy_spawn_fireball:
    ld hl,fireball_defaults
    ld ix,fireballs_base
    ld b,fireballs_max
    call spawn_sprite_from_tile
    ret nc ; no carry means no free sprite slots, so we go home
; other specific initialization
; ------------------------------------------------
; Copy the final projectile heading and component velocities from the targeting
; computer. Bearing_t points directly at the player; Heading_p includes lead.
    ld hl,(Heading_p)
    ld de,128*256 ; rotate 128 degrees
    add hl,de
    ld (ix+sprite_heading),hl
    ld hl,(Vp_x)
    ld (ix+sprite_xvel),hl
    ld hl,(Vp_y)
    ld (ix+fireball_ground_vy),hl
    call fireball_to_screen_vy
    ld (ix+sprite_yvel),hl
; move projectile one frame's distance so it isn't drawn right over turret
    ld hl,(ix+sprite_x)
    ld de,(ix+sprite_xvel)
    add hl,de
    ld (ix+sprite_x),hl
    ld hl,(ix+sprite_y)
    ld de,(ix+sprite_yvel)
    add hl,de
    ld (ix+sprite_y),hl
; select the heading-derived artwork immediately rather than exposing frame
; zero for the first complete eight-tick animation interval
    push ix
    pop iy
    call sprite_frame_heading_animation
    ld a,(iy+sprite_id)
    call vdu_sprite_select
    ld a,(iy+sprite_frame)
    call vdu_sprite_select_frame
    .if mus_combat_cues
    call cues_queue_launch ; slot allocated, projectile initialized and visible
    .endif
    ret
; end enemy_spawn_fireball

; HL = ground velocity + this frame's actual terrain displacement in 16.8.
; Do the full ADL add: an upward component can cross zero. Never accumulate
; last frame's camera motion or take speed from the decorative starfield.
fireball_to_screen_vy:
    ld de,0
    ld a,(terrain_step_pixels)
    ld d,a
    add hl,de
    ret

move_enemy_fireball:
    ld iy,(table_pointer)
    ld hl,(iy+fireball_ground_vy)
    call fireball_to_screen_vy
    ld (iy+sprite_yvel),hl
    call move_sprite
    ld a,(sprite_screen_edge)
    or a ; a will be non-zero if have scrolled off screen
    jp nz,table_deactivate_sprite
    ld ix,player_begin
    bit sprite_alive,(ix+sprite_collisions)
    jp z,@animate ; player is dead so don't check for collisions
    call check_enemy_collision_player
    jp nz,@animate ; no collision so do animation
    call update_shields
    jp vox_contact_explosion
@animate:
; check the animation timer
    dec (iy+sprite_animation_timer)
    ret nz ; not time to animate
; reset the animation timer and bump animation
    ld a,8
    ld (iy+sprite_animation_timer),a
    ld a,(iy+sprite_animation)
    inc a
    and %00000001 ; modulo 2
    ld (iy+sprite_animation),a
    call sprite_frame_heading_animation ; a, sprite_frame set to heading-based frame
    call vdu_sprite_select_frame
    ret
; end move_enemy_fireball

init_enemy_fireballs:
; set pointer to seeker sprite table
    ld iy,fireballs_base
    ld a,(iy+sprite_id) ; sprite id
    ld e,fireballs_max ; sprite loop counter
; define seeker sprites
@sprites_loop:
    push af
    push de
    call vdu_sprite_select
    call vdu_sprite_clear_frames
    ld hl,BUF_FIREBALL_2_000
    ld b,2*32 ; 2 animations * 32 orientations
@frames_loop:
    push bc
    push hl
    call vdu_sprite_add_buff
    pop hl
    inc hl
    pop bc
    djnz @frames_loop
    pop de ; sprite loop counter
    pop af ; sprite id
    inc a
    dec e
    jp nz,@sprites_loop
    ret
; end init_enemy_fireballs
