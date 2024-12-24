    align 256

; ######## GAME STATE VARIABLES #######
; THESE MUST BE IN THIS ORDER FOR game_initialize TO WORK PROPERLY
player_score: dl 0 ; integer
; player current shields,binary
; when < 0 player splodes
; restores to player_shields_max when new ship spawns
player_shields: dl 64 ; integer
; max player shields,binary
; can increase with power-ups (todo)
player_shields_max: dl 64 ; integer
player_power: dl 64 ; integer
player_power_max: dl 64 ; integer
; when reaches zero,game ends
; can increase based on TODO
player_ships: db 3 ; integer

speed_player: equ 3*256 ; 3 pixels per frame 16.8 fixed
player_lasers_max: equ 4

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
    ld iy,player_begin
    ld (table_pointer),iy
    ld a,(iy+sprite_id)
    call vdu_sprite_select
    call vdu_sprite_clear_frames
    ld hl,BUF_SHIP_0L
    call vdu_sprite_add_buff
    ld hl,BUF_SHIP_1C
    call vdu_sprite_add_buff
    ld hl,BUF_SHIP_2R
    call vdu_sprite_add_buff
    ld bc,0
    ld (iy+sprite_x),bc
    ld de,sprite_bottom*256 ; max sprite y-coordinate
    ld (iy+sprite_y),de
    call vdu_sprite_move_abs168
    call vdu_sprite_show
    ld hl,player_alive
    ld (player_state),hl
    set sprite_alive,(iy+sprite_collisions)
    ret

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

; ###################################################################
; TODO: the below is all stuff from the original code we need to port
; ###################################################################

kill_player:
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
    ret ; and out

; draws the player's shields level
; draw_shields:
; TODO: Agonize this routine
; ; prep the loop to draw the bars
;     ld a,(player_shields) ; snag shields
;     and a 
;     ret z ; don't draw if zero shields
; ; set loop counter and drawing position
;     ld b,a ; loop counter
;     ld hl,#5300+48+12
; ; set color based on bars remaining
;     ld c,103 ; bright green 28fe0a
;     cp 9
;     jp p,draw_shields_loop
;     ld c,74 ; bright yellow eafe5b 
;     cp 3
;     jp p,draw_shields_loop
;     ld c,28 ; bright red fe0a0a 
; draw_shields_loop:
;     push bc ; yup,outta
;     push hl ; registers again
;     ; ld a,#A8 ; ▀,168 
;     ld a,10 ; ▀,168 ; we renumber because we don't use the full charset
;     ; call draw_char
;     call draw_num ; we nuked draw_char for the time being
;     pop hl
;     ld a,8
;     add a,l
;     ld l,a
;     pop bc
;     djnz draw_shields_loop
    ; ret


; draw_lives:
;     ld hl,player_small ; make small yellow ship the active sprite
;     ld (sprite_base_bufferId),hl
;     ; ld a,#80 ; northern orientation
;     ; ld (sprite_orientation),a
;     ld hl,0 ; north
;     ld (sprite_heading),hl
;     xor a
;     ld (sprite_frame),a
;     ld a,#56 ; top of visible screen
;     ld (sprite_y+1),a
;     call vdu_buff_select ; TODO: convert to vdu_buff_select
;     ld a,(player_ships)
;     dec a ; we draw one fewer ships than lives
;     ret z ; nothing to draw here, move along
;     ld b,a ; loop counter
;     ld a,256-16 ; initial x position
; draw_lives_loop:
;     ld (sprite_x+1),a
;     push af
;     push bc
;     call vdu_bmp_draw ; convert to vdu_bmp_plot
;     pop bc
;     pop af
;     sub 10
;     djnz draw_lives_loop
;     ret 


dump_player_record:
    ld ix,player_begin

    call printInline
    asciz "id  : "
    ld a,(ix+sprite_id)
    call printHexA

    call printInline
    asciz "\r\nx   :"
    ld hl,(ix+sprite_x)
    call print_s168_hl

    call printInline
    asciz "\r\ny   :"
    ld hl,(ix+sprite_y)
    call print_s168_hl

    call printInline
    asciz "\r\nxv  :"
    ld hl,(ix+sprite_xvel)
    call print_s168_hl

    call printInline
    asciz "\r\nyv  :"
    ld hl,(ix+sprite_yvel)
    call print_s168_hl

    call printInline
    asciz "\r\nv   :"
    ld hl,(ix+sprite_vel)
    call print_s168_hl

    call printInline
    asciz "\r\nhdg :"
    ld hl,(ix+sprite_heading)
    call print_s168_hl

    call printInline
    asciz "\r\nori :"
    ld hl,(ix+sprite_orientation)
    call print_s168_hl
    
    call printInline
    asciz "\r\ntyp : "
    ld a,(ix+sprite_type)
    call printHexA
    
    call printInline
    asciz "\r\nBId : "
    ld hl,(ix+sprite_base_bufferId)
    call printHex24

    call printInline
    asciz "\r\nmPg : "
    ld hl,(ix+sprite_move_program)
    call printHex24

    call printInline
    asciz "\r\ncol : "
    ld a,(ix+sprite_collisions)
    call printBin8

    ; call printInline
    ; asciz "\r\nszx : "
    ; ld a,(ix+sprite_dim_x)
    ; call printHexA
    
    ; call printInline
    ; asciz "\r\nszy : "
    ; ld a,(ix+sprite_dim_y)
    ; call printHexA

    ; call printInline
    ; asciz "\r\n#ori: "
    ; ld a,(ix+sprite_num_orientations)
    ; call printHexA

    ; call printInline
    ; asciz "\r\n#ani: "
    ; ld a,(ix+sprite_num_animations)
    ; call printHexA

    ; call printInline
    ; asciz "\r\nani : "
    ; ld a,(ix+sprite_animation)
    ; call printHexA

    ; call printInline
    ; asciz "\r\nanit: "
    ; ld a,(ix+sprite_animation_timer)
    ; call printHexA

    ; call printInline
    ; asciz "\r\nmvt : "
    ; ld a,(ix+sprite_move_timer)
    ; call printHexA

    ; call printInline
    ; asciz "\r\nmvs : "
    ; ld a,(ix+sprite_move_step)
    ; call printHexA

    ; call printInline
    ; asciz "\r\npts : "
    ; ld a,(ix+sprite_points)
    ; call printHexA

    ; call printInline
    ; asciz "\r\ndmg : "
    ; ld a,(ix+sprite_shield_damage)
    ; call printHexA

    ; call printInline
    ; asciz "\r\nfrm : "
    ; ld a,(ix+sprite_frame)
    ; call printHexA

    ; call printNewLine
    ; call printNewLine

    ; call waitKeypress
    ret