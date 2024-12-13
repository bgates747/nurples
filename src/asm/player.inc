    align 256

; ######## GAME STATE VARIABLES #######
; THESE MUST BE IN THIS ORDER FOR game_initialize TO WORK PROPERLY
player_score: dl 0 ; integer
; player current shields,binary
; when < 0 player splodes
; restores to player_max_shields when new ship spawns
player_shields: dl 16 ; integer
; max player shields,binary
; can increase with power-ups (todo)
player_max_shields: dl 16 ; integer
; when reaches zero,game ends
; can increase based on TODO
player_ships: db 3 ; integer

speed_player: equ 3*256 ; 3 pixels per frame 16.8 fixed

player_weapons_count: db 0
player_weapons_max: equ 4

player_defaults:
    db player               ; type
    dl BUF_SHIP_0L          ; base_bufferId
    dl do_player            ; move_program
    db 0                    ; collisions
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

; process player keyboard input, set player bitmap
; velocities and draw player bitmap at updated coordinates
; Inputs: player_x/y set at desired position
; Returns: player bitmap drawn at updated position
; Destroys: probably everything except maybe iy
; NOTE: in mode 9 we draw the ship as a sprite, not a bitmap
; TODO: requires sprite implementation
player_input:
    ld iy,player_begin
    ld (table_pointer),iy
; ; did we die last frame?
;     ld a,(player_collisions)
;     and %00000010 ; zero flag will be set if not dead
;     jr z,player_not_dead
; ; yes we died
;     call kill_player  
;     ret ; done
player_not_dead: ; yay we didn't die
; reset player component velocities to zero as the default
    ld hl,0
    ld (iy+sprite_xvel),hl
    ld (iy+sprite_yvel),hl
; make ship the active sprite
    ld a,(iy+sprite_id)
    call vdu_sprite_select
; check for keypresses and branch accordingly
; for how this works,see: https://github.com/breakintoprogram/agon-docs/wiki/MOS-API-%E2%80%90-Virtual-Keyboard
    MOSCALL    mos_getkbmap ;ix = pointer to MOS virtual keys table
; we test all four arrow keys and add/subract velocities accordingly
; this handles the case where two opposing movement keys
; are down simultaneously (velocities will net to zero)
; and allows diagonal movement when a vertical and horizontal key are down
; it also allows movement and action keys to be detected simultaneously
; so we can walk and chew gum at the same time
    ld a,1 ; set ship's default animation to center
        ; if left and right are both down a will net to 

; 99 Space: fire main weapon
    bit 2,(ix+12)
    push af ; save flags for after player movement processed
; 26 Left: move player left
    bit 1,(ix+3)
    jp z,@F
    ld hl,(iy+sprite_xvel)
    ld bc,-speed_player
    add hl,bc
    ld (iy+sprite_xvel),hl
    dec a ; set ship's animation to left
@@:
; 122 Right: move player right
    bit 1,(ix+15)
    jp z,@F
    ld hl,(iy+sprite_xvel)
    ld bc,speed_player
    add hl,bc
    ld (iy+sprite_xvel),hl
    inc a ; set ship's animation to right
@@:
; 58 Up: move player up
    bit 1,(ix+7)
    jp z,@F
    ld hl,(iy+sprite_yvel)
    ld bc,-speed_player
    add hl,bc
    ld (iy+sprite_yvel),hl
@@:
; 42 Down: move player down
    bit 1,(ix+5)
    jp z,@F
    ld hl,(iy+sprite_yvel)
    ld bc,speed_player
    add hl,bc
    ld (iy+sprite_yvel),hl
@@:
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
    pop af ; restore flags from spacebar check
    jp nz,player_spawn_laser ; will return to caller from there
    ret
; end player_input

score_x: equ 48
score_y: equ 1
; prints the player's score
print_score:
    ld c,score_x
    ld b,score_y
    call vdu_move_cursor
; TODO: make this part of the UI setup and only print as needed
    call printInline
    asciz "Score:"
; END TODO
    ld hl,(player_score)
    call printDec
    ret
; end print_score

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