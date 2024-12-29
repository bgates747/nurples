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

player_state:  dl player_alive
player_ships: db 0
player_ships_max: db 3
player_speed: equ 3*256 ; pixels per frame 16.8 fixed

draw_player_ships:
; activate mfd
    ld ix,mfd_bg_r
    call mfd_activate
; blank out previously drawn ships
    xor a ; color mode 0
    ld c,c_black
    call vdu_gcol
    ld bc,0
    ld de,0
    ld ix,63
    ld iy,15
    call vdu_plot_rf
; draw the ships remaining
    ld hl,BUF_SHIP_SMALL
    call vdu_buff_select
    ld a,(player_ships) ; loop counter
    ld bc,4 ; starting x-coordinate
@loop:
    push af ; back up loop counter
    push bc ; save x-coordinate
    ld de,4 ; y-coordinate
    call vdu_plot_bmp
    pop bc ; restore x-coordinate
    ld hl,12 ; bump x-coords
    add hl,bc
    push hl
    pop bc ; bc = new x-coordinate
    pop af
    dec a
    jp nz,@loop    
    ret
; end draw_player_ships

do_player:
    ld hl,(player_state)
    CALL_HL
    ret
; end do_player

; set initial player position
; inputs: none,everything is hardcoded
; outputs: player_x/y set to bottom-left corner of screen
; destroys: pretty much everything
player_init:
    ld a,(player_shields_max)
    ld (player_shields),a
    ld a,(laser_power_max)
    ld (laser_power),a
    ld a,(player_fuel_max)
    ld (player_fuel),a
; set all to zero so they will update
    xor a
    ld (laser_power_last),a
    ld (player_shields_last),a
    ld (player_fuel_last),a
; set player sprite record with defaults
    ld ix,player_begin
    ld hl,player_defaults
    ld bc,0*256
    ld de,sprite_bottom*256
    ld a,1
    call spawn_sprite_from_coords
    jp nc,@draw
    res 7,(ix+sprite_type)
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

player_alive:
    call player_input
    call player_move_weapons
    ret
; end player_alive

player_dying:
; DEBUG
; decrement lives
    ld a,(player_ships)
    dec a
    jp z,game_over
    ld (player_ships),a
    call draw_player_ships
; reset the sprite table
    call table_init
    ld hl,table_base
    ld (table_pointer),hl
; initialize sprites and active tiles table
    call sprites_init
    call table_init
    call tile_table_init
; reset player score to what it was at the start of the level
    ld hl,(player_starting_score)
    ld (player_score),hl

; set gfx origin and viewport to playing field window
    call set_gfx_playing_field
    call vdu_clg

; print get ready message
    ld c,27 ; x
    ld b,16 ; y
    call vdu_move_cursor
    call printInline
    asciz "Get Ready!"

; countdown to restart
    ld c,23 ; x
    ld b,17 ; y
    call vdu_move_cursor
    call printInline
    asciz "Restarting In..."
    ld b,6 ; loop counter
@waitLoop:
    push bc
    ld a,b
    dec a
    call printHexA
    ld c,39 ; x
    ld b,17 ; y
    call vdu_move_cursor
    ld a,%10000000 ; bit 7 changes once per second
    call multiPurposeDelay
    pop bc
    djnz @waitLoop

; restart the level
    call vdu_clg
    ld hl,cur_level
    dec (hl) ; decrement level so it will be incremented back to the same level
    call next_level
; spawn player sprite at starting position
    call player_init
; set the game state to playing
    ld hl,game_playing
    ld (game_state), hl
; set game playing substates
    ld hl,player_alive
    ld (player_state), hl
    ld hl,enemies_alive
    ld (enemies_state), hl
    ret
; END DEBUG

    ld hl,enemies_despawning
    ld (enemies_state),hl
    ret
; end player_dying

get_ready: asciz "Get Ready!"

player_dead:
    call player_move_weapons
    ret
; end player_dead


kill_player:
    ld hl,player_dying
    ld (player_state),hl
    ret
; end kill_player

