    align 256

seekers_count: db 0
seekers_max: equ 3
speed_seeker: equ 0x000280 ; 2.5 pixels per frame

seeker_defaults:
    db enemy_air         ; type
    dl BUF_SEEKER_000        ; base_bufferId
    dl move_enemy_seeker    ; move_program
    db collides_with_laser | collides_with_player ; collisions
    db 16                    ; dim_x
    db 16                   ; dim_y
    db 32                   ; num_orientations
    db 1                    ; num_animations
    db 0                    ; animation
    db 0                    ; animation_timer
    db 1                    ; move_timer
    db 0                    ; move_step
    db 10                    ; points
    db 2                    ; shield_damage
    db 0                    ; frame

enemy_spawn_seeker:
    ld hl,seeker_defaults
    call spawn_sprite_from_sprite
    ret nc ; no carry means no free sprite slots, so we go home
; other specific initialization
    ld hl,speed_seeker
    ld (ix+sprite_vel),hl
    ret
; end enemy_spawn_seeker

move_enemy_seeker:
; run the movement program for the seeker enemy
    call en_nav_seek_zag
    jp check_enemy_collisions
    ret
; end move_enemy_seeker

; contains the logic for how to move the enemy
; and then does the moving
; inputs: a fully-populated active sprite table
;         player position variables
; destroys: everything except index registers
; outputs: moving enemies
en_nav_seek_zag:
; set velocity and orientation by player's relative location
; move enemies y-axis
; where is player relative to us?
    call orientation_to_player ; uh.l angle to player, ub.c, ud.e = dx, dy
; is player above or below us?
    ld (ude),de ; dy
    ld a,(ude+2) ; deu
    rla ; shift sign bit into carry
    jp nc,@zig_zag ; player is below,evade
; player is even or above,so home in on current heading
    ld (iy+sprite_heading),hl ; save sprite heading
; fall through to @compute_velocities
@compute_velocities:
    call sprite_compute_velocities
; change the animation frame to match heading
    call sprite_frame_heading ; a, sprite_frame set to heading-based frame
    call move_sprite ; move the sprite
    or a ; a will be non-zero if have collided with screen edge
    jp nz,@zag_switch ; switch direction
    ret ; didn't hit edge, nothing else to do
@zig_zag_start:
    ld iy,(table_pointer) 
    call rand_8
    and %00111111 ; limit it to 64
    set 3,a ; make sure it's at least 8
    ld (iy+sprite_move_timer),a ; store it
    ; fall through to @zig_zag
@zig_zag:
    ld a,(iy+sprite_move_timer)
    dec a
    ld (iy+sprite_move_timer),a
    jp nz,@zig_zag_no_switch
@zag_switch:
    ; otherwise flip direction and restart timer
    ld a,(iy+sprite_move_step)
    xor %1 ; flips bit zero
    ld (iy+sprite_move_step),a ; store it
    jp nz,@F ; zag right
;otherwise zag left
    ld hl,160*256; southwest heading
    ld (iy+sprite_heading),hl ; save sprite heading
    jp @zig_zag_start
@@: ; zag right
    ld hl,96*256; southeast heading
    ld (iy+sprite_heading),hl ; save sprite heading
    jp @zig_zag_start
@zig_zag_no_switch:
    ; ld a,(sprite_orientation)
    ld hl,(iy+sprite_heading)
    jp @compute_velocities
; end en_nav_seek_zag
