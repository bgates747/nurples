; ##### LASER SPRITE PARAMETERS #####
; uses the same offsets from its table base as the main sprite table:
laser_start_variables: ; label marking beginning of table
laser_id:               db table_max_records+1
laser_type:             db     0x00 ; 1 bytes currently not used
laser_base_bufferId:    dl BUF_LASER_A ; 3 bytes bitmap bufferId
laser_move_program:     dl 0x000000 ; 3 bytes not currently used
laser_collisions:       db     0x00 ; 1 bytes bit 0 set=alive, otherwise dead, bit 1 set=just died
laser_dim_x:            db     0x00 ; 1 bytes sprite width in pixels
laser_dim_y:            db     0x00 ; 1 bytes sprite height in pixels
laser_x:                dl 0x000000 ; 3 bytes 16.8 fractional x position in pixels
laser_y:                dl 0x000000 ; 3 bytes 16.8 fractional y position in pixels
laser_xvel:             dl 0x000000 ; 3 bytes x-component velocity, 16.8 fixed, pixels
laser_yvel:             dl 0xFFF800 ; 3 bytes y-component velocity, 16.8 fixed, pixels
laser_vel:              dl 0x000000 ; 3 bytes not currently used
laser_heading:          dl 0x000000 ; 3 bytes sprite movement direction deg256 16.8 fixed
laser_orientation:      dl 0x000000 ; 3 bytes not currently used
laser_animation:        db     0x00 ; 1 bytes current sprite animation frame
laser_animation_timer:  db     0x00 ; 1 bytes decremented every frame, when zero, advance animation
laser_move_timer:       db     0x00 ; 1 bytes not currently used
laser_move_step:        db     0x00 ; 1 bytes not currently used
laser_points:           db     0x00 ; 1 bytes not currently used
laser_shield_damage:    db     0x00 ; 1 bytes not currently used
laser_end_variables: ; for when we want to traverse this table in reverse

; laser_control:
; ; is laser already active?
;     ld a,(laser_collisions)
;     and %00000001 ; bit zero is lit if laser is active
;     jr nz,laser_move ; move laser if not zero
; ; otherwise check if laser fired
;     in a,(#82) ; keyboard
;     and %00010000 ; bit 4 is lit if space bar pressed
;     ret z ; go back if laser not fired
; ; otherwise,FIRE ZEE LASER!!1111
; ; set laser status to active (set bit 0)
;     ld a,%1
;     ld (laser_collisions),a
; ; initialize laser position
;     ld a,(player_x+1) ; we only need the integer part
;     ; add a,6 ; horizontal center with player sprite
;     ld (laser_x+1),a ; store laser x coordinate
;     ld a,(player_y+1) ; we only need the integer part
;     add a,-6 ; set laser y a few pixels above player
;     ld (laser_y+1),a ; store laser y coordinate
;     ; fall through to laser_move

; laser_move:
; ; begin setting laser to active sprite
;     ld hl,lasers
;     ld (sprite_base_bufferId),hl
;     ld hl,0 ; north
;     ld (sprite_heading),hl
;     xor a ; laser has no animations yet :-(
;     ld (sprite_animation),a
;     ; we set position here for the time being as a default
;     ; in case the laser is flagged for deletion
;     ; load sprite_x with laser x position (we do y further down)
;     ld hl,(laser_x)
;     ld (sprite_x),hl
; ; did laser just die?
;     ld a,(laser_collisions)
;     bit 1,a ; z if laser didn't just die
;     jr z,laser_not_dead_yet
; ; yes laser died
;     call kill_laser
;     ret ; done
; laser_not_dead_yet:
; ; draw it
; ; update laser y position
;     ld hl,(laser_y) ; grab laser y position
;     ld de,(laser_yvel) ; snag laser y velocity
;     add hl,de ; add y velocity to y pos 
;     ld (sprite_y),hl ; update laser y position
;     ld (laser_y),hl ; update laser y position
; ; are we at top of screen?
;     ld a,#51 ; top of visible screen plus a pixel
;     sub h ; no carry if above threshold
;     jr c,finally_draw_the_frikken_laser
;     ; if at top of screen,laser dies
;     call kill_laser
;     ret
; ; otherwise,finally draw the frikken laser
; finally_draw_the_frikken_laser:
;     call vdu_bmp_select
;     call vdu_bmp_draw
; ; all done
;     ret

; kill_laser:
; ; update status to inactive
;     xor a ; zero out a
;     ld (laser_collisions),a
;     ret