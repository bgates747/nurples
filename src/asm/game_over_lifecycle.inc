; Bounded GAME OVER lifecycle.
;
; The global state owns orchestration only. Seekers and turrets continue their
; actor-owned retreat/search behavior. This boundary removes the player and
; projectiles; overlay and scenery animation continue on the frozen field.

game_over_lifecycle_active: db 0
game_over_dismiss_requested: db 0

game_over_lifecycle_process_init:
    xor a
    ld (game_over_lifecycle_active),a
    ld (game_over_dismiss_requested),a
    ret
; end game_over_lifecycle_process_init

; inputs: IY = first record in an exact compile-time sprite pool
;         B = pool count
; outputs: every record hidden and collision-disabled; carry set
; destroys: A,B,DE,HL,IY
game_over_deactivate_sprite_pool:
@loop:
    push bc
    call table_deactivate_sprite
    lea iy,iy+table_record_size
    pop bc
    djnz @loop
    scf
    ret
; end game_over_deactivate_sprite_pool

; Enter once after the final player-dead event. Overlay acquisition happens
; before actor mutation so a failed hardware gate leaves the old world intact.
; outputs: carry set on active entry; carry clear on renderer/setup failure
game_over_lifecycle_enter:
    ld a,(game_over_lifecycle_active)
    or a
    jp z,@acquire
    scf
    ret
@acquire:
    ld a,PLAYFIELD_CHECK_GAME_OVER
    call playfield_display_verify
    ret nc
    ld a,PRESENTATION_GAME_OVER
    call presentation_overlay_enter
    ret nc
    call presentation_renderer_enter
    jp c,@remove_hostiles
    call presentation_overlay_exit
    xor a
    ret

@remove_hostiles:
    ld iy,player_begin
    call table_deactivate_sprite
    ld iy,player_lasers_begin
    ld b,player_lasers_max
    call game_over_deactivate_sprite_pool
    ld iy,fireballs_base
    ld b,fireballs_max
    call game_over_deactivate_sprite_pool
    call seeker_group_enter_game_over_retreat
    xor a
    ld (game_over_dismiss_requested),a
    inc a
    ld (game_over_lifecycle_active),a
    scf
    ret
; end game_over_lifecycle_enter

; Advance exactly one outer frame. The main loop remains the sole VBlank owner.
; outputs: carry set when a release-gated fresh key requests continuation
; destroys: all volatile application registers
game_over_lifecycle_update:
    ld a,(game_over_lifecycle_active)
    or a
    jp nz,@world
    xor a
    ret
@world:
; The one-scanline comparison uses the same travel policy for passive terrain
; and active tiles. The production path leaves both sets of coordinates fixed.
    .if GAME_OVER_REVIEW_SCROLL
    call tiles_plot
    .endif
    call set_gfx_playing_field
    call do_enemies
    call move_explosions
    call move_tiles

; Preserve the ordinary fixed cockpit gauges over the moving world. Charging a
; hidden weapon cannot spawn or score, and clean-session init resets its state.
    ld bc,0
    ld de,0
    call vdu_set_gfx_origin
    call draw_shields
    call draw_laser
    call draw_fuel

    call presentation_input_poll
    call presentation_overlay_update
    ld a,0
    adc a,0
    ld (game_over_dismiss_requested),a
    call presentation_renderer_update
    ld a,(game_over_dismiss_requested)
    or a
    jp z,@continue
    scf
    ret
@continue:
    xor a
    ret
; end game_over_lifecycle_update

; Detach/demote overlay sprites before clearing controller ownership. Immutable
; bitmap assets remain resident for attract and later GAME OVER entries.
game_over_lifecycle_exit:
    ld a,(game_over_lifecycle_active)
    or a
    ret z
    call presentation_renderer_exit
    call presentation_overlay_exit
    xor a
    ld (game_over_lifecycle_active),a
    ld (game_over_dismiss_requested),a
    scf
    ret
; end game_over_lifecycle_exit
