; ######## GAME STATE VARIABLES #######
; THESE MUST BE IN THIS ORDER FOR new_game TO WORK PROPERLY
player_score: db 0x00,#00,#00 ; bcd
; player current shields,binary
; when < 0 player splodes
; restores to player_max_shields when new ship spawns
player_shields: db 16 ; binary
; max player shields,binary
; can increase with power-ups (todo)
player_max_shields: db 16 ; binary
; when reaches zero,game ends
; can increase based on TODO
player_ships: db 0x03 ; binary

speed_player: equ 0x000300 ; 3 pixels per frame

; ######### PLAYER SPRITE PARAMETERS ##########
; uses the same offsets from its table base as the main sprite table:
player_start_variables: ; label marking beginning of table
player_id:               db table_max_records
player_type:             db     0x00 ; 1 bytes currently not used
player_base_bufferId:    dl BUF_SHIP_0L ; 3 bytes bitmap bufferId
player_move_program:     dl 0x000000 ; 3 bytes not currently used
player_collisions:       db     0x00 ; 1 bytes bit 0 set=alive, otherwise dead, bit 1 set=just died
player_dim_x:            db     0x00 ; 1 bytes sprite width in pixels
player_dim_y:            db     0x00 ; 1 bytes sprite height in pixels
player_x:                dl 0x000000 ; 3 bytes 16.8 fractional x position in pixels
player_y:                dl 0x000000 ; 3 bytes 16.8 fractional y position in pixels
player_xvel:             dl 0x000000 ; 3 bytes x-component velocity, 16.8 fixed, pixels
player_yvel:             dl 0x000000 ; 3 bytes y-component velocity, 16.8 fixed, pixels
player_vel:              dl 0x000000 ; 3 bytes velocity px/frame (16.8 fixed)
player_heading:          dl 0x000000 ; 3 bytes sprite movement direction deg256 16.8 fixed
player_orientation:      dl 0x000000 ; 3 bytes not currently used
player_animation:        db     0x00 ; 1 bytes not currently used
player_animation_timer:  db     0x00 ; 1 bytes not currently used
player_move_timer:       db     0x00 ; 1 bytes not currently used
player_move_step:        db     0x00 ; 1 bytes not currently used
player_points:           db     0x00 ; 1 bytes not currently used
player_shield_damage:    db     0x00 ; 1 bytes not currently used
player_end_variables: ; for when we want to traverse this table in reverse

; set initial player position
; inputs: none,everything is hardcoded
; outputs: player_x/y set to bottom-left corner of screen
; destroys: a
player_init:
	ld a,(player_id)
	call vdu_sprite_select
    call vdu_sprite_clear_frames
    ld hl,BUF_SHIP_0L
    call vdu_sprite_add_buff
    ld hl,BUF_SHIP_1C
    call vdu_sprite_add_buff
    ld hl,BUF_SHIP_2R
    call vdu_sprite_add_buff
    ld bc,0
    ld (player_x),bc
    ld de,0x00DF00
    ld (player_y),de
    call vdu_sprite_move_abs168
    call vdu_sprite_show
    ret

; process player keyboard input, set player bitmap
; velocities and draw player bitmap at updated coordinates
; Inputs: player_x/y set at desired position
; Returns: player bitmap drawn at updated position
; Destroys: probably everything except maybe iy
; NOTE: in mode 9 we draw the ship as a sprite, not a bitmap
; TODO: requires sprite implementation
player_input:
; reset player component velocities to zero as the default
	ld hl,0
	ld (player_xvel),hl
	ld (player_yvel),hl
; make ship the active sprite
    ld a,(player_id)
    call vdu_sprite_select
; check for keypresses and branch accordingly
; for how this works,see: https://github.com/breakintoprogram/agon-docs/wiki/MOS-API-%E2%80%90-Virtual-Keyboard
    MOSCALL	mos_getkbmap ;ix = pointer to MOS virtual keys table
; we test all four arrow keys and add/subract velocities accordingly
; this handles the case where two opposing movement keys
; are down simultaneously (velocities will net to zero)
; and allows diagonal movement when a vertical and horizontal key are down
; it also allows movement and action keys to be detected simultaneously
; so we can walk and chew gum at the same time
    ld a,1 ; set ship's default animation to center
        ; if left and right are both down a will net to 

@left:
    bit 1,(ix+3) ; keycode 26
    jr z,@right
    ld hl,(player_xvel)
    ld bc,-speed_player
    add hl,bc
    ld (player_xvel),hl
    dec a ; set ship's animation to left
@right:
    bit 1,(ix+15) ; keycode 122
	jr z,@up
    ld hl,(player_xvel)
    ld bc,speed_player
    add hl,bc
    ld (player_xvel),hl
    inc a ; set ship's animation to right
@up:
    bit 1,(ix+7) ; keycode 58
	jr z,@down
    ld hl,(player_yvel)
    ld bc,-speed_player
    add hl,bc
    ld (player_yvel),hl
@down:
    bit 1,(ix+5) ; keycode 42
	jr z,@done_keyboard
    ld hl,(player_yvel)
    ld bc,speed_player
    add hl,bc
    ld (player_yvel),hl
@done_keyboard:
; a should land here loaded with the correct frame
    call vdu_sprite_select_frame
; draw player at updated position
    ld iy,player_start_variables
    ld (table_pointer),iy
    call move_sprite
    ld bc,(player_x)
    ld de,(player_y)
	call vdu_sprite_move_abs168
; end player_input
	ret

; ###################################################################
; TODO: the below is all stuff from the original code we need to port
; ###################################################################

; kill_player:
; ; set player status to dead
;     xor a; sets all player flags to zero
;     ld (player_collisions),a
; ; deduct a ship from the inventory
;     ld a,(player_ships)
;     dec a
;     ld (player_ships),a
; ; are we out of ships?
;     jp z,game_over
; ; wait a few ticks
;     ld a,32 ; 32-cycle timer ~1/2 second at 60fps
;     ld (player_move_timer),a
; kill_player_loop:
;     call vdu_vblank
;     ld a,(player_move_timer)
;     dec a
;     ld (player_move_timer),a
;     jr nz,kill_player_loop 
;     call player_init ; player respawn if timer zero
;     ret ; and out


; player_move:
; ; begin setting player to active sprite
;     ld hl,player
;     ld (sprite_base_bufferId),hl
;     ld hl,0 ; north
;     ld (sprite_heading),hl
;     ld a,#01 ; animation 1 is center,which we set here as a default
;     ld (sprite_animation),a
;     ; we set position here for the time being as a default
;     ; in case the player doesn't move,or is flagged for deletion
;     ld hl,(player_x)
;     ld (sprite_x),hl
;     ld hl,(player_y)
;     ld (sprite_y),hl
; ; did we just die?
;     ld a,(player_collisions)
;     and %00000010 ; zero flag will be set if not dead
;     jr z,player_not_dead
; ; yes we died
;     call kill_player  
;     ret ; done
; ; yay we didn't die
; player_not_dead:
; ; set player movements to zero by default
;     ld hl,0
;     ld (player_xvel),hl
;     ld (player_yvel),hl
; ; do we move it?
;     in a,(#82) ; keyboard
;     or a ; if zero,don't move
;     jr z,player_draw
; ; move it
;     call player_move_calc
; player_draw:
;     call vdu_bmp_select ; TODO: convert to vdu_buff_select
;     call vdu_bmp_draw ; convert to vdu_bmp_plot
; player_move_done:
;     ; write updated x,y coordinates back to player table
;     ld hl,(sprite_x)
;     ld (player_x),hl
;     ld hl,(sprite_y)
;     ld (player_y),hl
;     ret