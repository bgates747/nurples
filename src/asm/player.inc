    align 256

; ######## GAME STATE VARIABLES #######
player_state:  dl player_alive ; Player state variable
player_score: dl 0 ; integer
player_ships: db 3 ; integer
speed_player: equ 3*256 ; 3 pixels per frame 16.8 fixed
player_lasers_max: equ 4

do_player:
    ld hl,(player_state)
    CALL_HL                ; Call the corresponding player state
    ret

player_alive:
    call player_input     ; Handle player input
    call player_move_weapons    ; Handle laser movement and collision detection
    ret

player_dead:
    call player_move_weapons    ; Handle laser movement and collision detection
    ret

player_defaults:
    db player               ; type
    dl BUF_SHIP_0L          ; base_bufferId
    dl do_player            ; move_program
    db %01000000            ; collisions bit 6 signals sprite alive
    db 16                   ; dim_x
    db 16                   ; dim_y
    db 1                    ; num_orientations
    db 1                    ; num_animations
    db 0                    ; animation
    db 0                    ; animation_timer
    db 0                    ; move_timer
    db 0                    ; move_step
    db 0                    ; points
    db 0                    ; shield_damage
    db 0                    ; frame


; set initial player position
; inputs: none,everything is hardcoded
; outputs: player_x/y set to bottom-left corner of screen
; destroys: a
player_init:
    ld a,(player_shields_max)
    ld (player_shields),a
    ld a,(laser_power_max)
    ld (laser_power),a
    ld a,(player_fuel_max)
    ld (player_fuel),a
    ld hl,0
    ld (player_score),hl
    xor a
    ld (laser_power_last),a ; set to zero so draw_laser will update
    ld (player_shields_last),a ; set to zero so draw_shields will update
    ld (player_fuel_last),a ; set to zero so draw_fuel will update    
    ld a,3
    ld (player_ships),a

; set player sprite record with defaults
    ld ix,player_begin
    ld hl,player_defaults
    ld bc,0*256
    ld de,sprite_bottom*256
    ld a,1
    call spawn_sprite_from_coords
    jp nc,@draw ; player sprite record already exists so skip to draw
    res 7,(ix+sprite_type) ; clear just spawned flag

; load sprite frames
    ld a,(ix+sprite_id)
    call vdu_sprite_select
    call vdu_sprite_clear_frames
    ld hl,BUF_SHIP_0L
    call vdu_sprite_add_buff
    ld hl,BUF_SHIP_1C
    call vdu_sprite_add_buff
    ld hl,BUF_SHIP_2R
    call vdu_sprite_add_buff

@draw:
    ld bc,0*256
    ld de,sprite_bottom*256
    call vdu_sprite_move_abs168
    call vdu_sprite_show
    ld hl,player_alive
    ld (player_state),hl
    set sprite_alive,(ix+sprite_collisions)
    ret
; end player_init

; UART ports for reading joystick input
; bit tests: 0 means actuated, 1 means not actuated
portC: 	EQU 	$9E ; reads dpad input
joy1_u: equ 1
joy1_d: equ 3
joy1_l: equ 5
joy1_r: equ 7
portD: 	EQU 	$A2 ; reads buttons 
btn1_a: equ 5

; process player keyboard input, set player bitmap
; velocities and draw player bitmap at updated coordinates
; Inputs: player_x/y set at desired position
; Returns: player bitmap drawn at updated position
; Destroys: probably everything except maybe iy
; NOTE: in mode 9 we draw the ship as a sprite, not a bitmap
; TODO: requires sprite implementation
player_input:
; point to player sprite record
    ld iy,player_begin
    ld (table_pointer),iy
; reset player component velocities to zero as the default
    ld hl,0
    ld (iy+sprite_xvel),hl
    ld (iy+sprite_yvel),hl
; make ship the active sprite
    ld a,(iy+sprite_id)
    call vdu_sprite_select
; check for keypresses and branch accordingly
    MOSCALL    mos_getkbmap ;ix = pointer to MOS virtual keys table
; do player directional input
    xor a ; clear flags
    in a,(portC) ; read dpad input
    ld d,a ; save dpad input
    ld a,1 ; set ship's default animation to center
; 26 Left: move player left
    bit 1,(ix+3)
    jp nz,@F
; test dpad left
    bit joy1_l,d
    jp z,@F
    jp @right
@@:
    ld hl,(iy+sprite_xvel)
    ld bc,-speed_player
    add hl,bc
    ld (iy+sprite_xvel),hl
    dec a ; set ship's animation to left
@right:
; 122 Right: move player right
    bit 1,(ix+15)
    jp nz,@F
; test dpad right
    bit joy1_r,d
    jp z,@F
    jp @up
@@:
    ld hl,(iy+sprite_xvel)
    ld bc,speed_player
    add hl,bc
    ld (iy+sprite_xvel),hl
    inc a ; set ship's animation to right
@up:
; 58 Up: move player up
    bit 1,(ix+7)
    jp nz,@F
; test dpad up
    bit joy1_u,d
    jp z,@F
    jp @dn
@@:
    ld hl,(iy+sprite_yvel)
    ld bc,-speed_player
    add hl,bc
    ld (iy+sprite_yvel),hl
@dn:
; 42 Down: move player down
    bit 1,(ix+5)
    jp nz,@F
; test dpad down
    bit joy1_d,d
    jp z,@F
    jp @frame
@@:
    ld hl,(iy+sprite_yvel)
    ld bc,speed_player
    add hl,bc
    ld (iy+sprite_yvel),hl
@frame:
; a should land here loaded with the correct frame
    call vdu_sprite_select_frame
; draw player at updated position
    ld iy,player_begin
    ld (table_pointer),iy
    call move_sprite
    ld bc,(iy+sprite_x)
    ld de,(iy+sprite_y)
    call vdu_sprite_move_abs168

; process player laser fire
; do player fire button input
; read joystick fire button
    xor a ; clear flags
    in a,(portD) ; read joypad fire button input
    bit btn1_a,a
    jp z,player_spawn_laser
; 99 Space: fire main weapon
    bit 2,(ix+12)
    jp nz,player_spawn_laser
; no fire button pressed so go home
    ret
; end player_input

player_move_weapons:
; initialize pointers and loop counter
    ld iy,player_weapons_begin ; set iy to first record in table
    ld b,player_lasers_max ; loop counter
    call move_sprite_group
    ret
; end player_move_weapons

kill_player:
; DEBUG
    ld hl,game_init
    ld (game_state),hl
    ret
; END DEBUG

    ld hl,enemies_despawning
    ld (enemies_state),hl
    ld hl,player_dying
    ld (player_state),hl
    ret 
; end kill_player

player_dying:

    ret
; end player_dying