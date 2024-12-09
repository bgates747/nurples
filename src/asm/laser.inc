    align 256

; ##### LASER SPRITE PARAMETERS #####
; uses the same offsets from its table base as the main sprite table:
laser_start_variables: ; label marking beginning of table
laser_id:                db table_max_records+1 ; 1 bytes unique spriteId, zero-based
; sprite state variables
laser_x:                 dl                   0 ; 3 bytes 16.8 fractional x position in pixels
laser_y:                 dl                   0 ; 3 bytes 16.8 fractional y position in pixels
laser_xvel:              dl                   0 ; 3 bytes x-component velocity, 16.8 fixed, pixels
laser_yvel:              dl              -6*256 ; 3 bytes y-component velocity, 16.8 fixed, pixels
laser_vel:               dl              -6*256 ; 3 bytes velocity px/frame (16.8 fixed)
laser_heading:           dl                   0 ; 3 bytes sprite movement direction deg256 16.8 fixed
laser_orientation:       dl                   0 ; 3 bytes orientation bits
; sprite definition variables
laser_type:              db                   0 ; 1 bytes not currently used
laser_base_bufferId:     dl         BUF_LASER_A ; 3 bytes bitmap bufferId
laser_move_program:      dl                   0 ; 3 bytes not currently used
laser_collisions:        db                   0 ; 1 bytes see collisions.inc constants for bit definitions
laser_dim_x:             db                  16 ; 1 bytes sprite width in pixels
laser_dim_y:             db                  16 ; 1 bytes sprite height in pixels
laser_num_orientations:  db                   1 ; 1 bytes number of orientations for this sprite
laser_num_animations:    db                   2 ; 1 bytes number of animations for this sprite
laser_animation:         db                   1 ; 1 bytes current animation index, zero-based
laser_animation_timer:   db                   0 ; 1 bytes when hits zero, draw next animation
laser_move_timer:        db                   0 ; 1 bytes when zero, go to next move program, or step
laser_move_step:         db                   0 ; 1 bytes stage in a move program sequence, varies
laser_points:            db                   0 ; 1 bytes points awarded for killing this sprite type
laser_shield_damage:     db                   0 ; 1 bytes shield points deducted for collision
laser_frame:             db                   0 ; 1 bytes current animation frame
laser_end_variables: ; for when we want to traverse this table in reverse
laser_speed: equ -6*256 ; 6 pixels per frame 16.8 fixed
laser_animation_timer_reset: equ 2 ; animate every two frames

laser_control:
; point to laser sprite variables
    ld iy,laser_start_variables
    ld (table_pointer),iy
; did laser just die?
    bit sprite_just_died,(iy+sprite_collisions)
    jp nz,kill_laser
; select laser sprite
    ld a,(laser_id)
    call vdu_sprite_select
; update laser position
    call move_sprite
; are we at top of screen?
    ld a,(sprite_screen_edge)
    and collide_top
    jp nz,kill_laser
; check animation timer
    dec (iy+sprite_animation_timer)
    jp nz,@F
    ld a,laser_animation_timer_reset
    ld (iy+sprite_animation_timer),a
    call vdu_sprite_next_frame
@@: ; draw the sprite
    ld bc,(iy+sprite_x)
    ld de,(iy+sprite_y)
    call vdu_sprite_move_abs168
; return to caller
    ret
; end laser_control

fire_laser:
; point to laser sprite variables
    ld iy,laser_start_variables
    ld (table_pointer),iy
; is laser already active?
    bit sprite_alive,(iy+sprite_collisions)
    ret nz ; laser already active
; otherwise,FIRE ZEE LASER!!1111
; set laser status to active
    set sprite_alive,(iy+sprite_collisions)
; initialize laser sprite parameters
; x-coordinate
    ld ix,player_start_variables
    ld hl,(ix+sprite_x)
    ld de,5*256 ; horizontal center with player sprite
    add hl,de
    ld (iy+sprite_x),hl
; y-coordinate
    ld hl,(ix+sprite_y)
    ld de,-6*256 ; set laser y a few pixels above player
    ld (iy+sprite_y),hl
; set laser velocity
    ld hl,laser_speed
    ld (iy+sprite_yvel),hl
; set laser animation frame and timer
    xor a ; first frame
    ld (iy+sprite_frame),a
    ld a,laser_animation_timer_reset
    ld (iy+sprite_animation_timer),a
; plot the sprite and make it visible
    ld a,(laser_id)
    call vdu_sprite_select
    ld bc,(iy+sprite_x)
    ld de,(iy+sprite_y)
    call vdu_sprite_move_abs168
    call vdu_sprite_show
; return to caller
    ret
; end fire_laser

kill_laser:
; update status to inactive
    xor a ; zero out a
    ld (laser_collisions),a
; hide the sprite
    ld a,(laser_id)
    call vdu_sprite_select
    call vdu_sprite_hide
    ret
; end kill_laser