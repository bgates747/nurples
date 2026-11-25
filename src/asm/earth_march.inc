; IY: ordinary air record. Step 0 waits, 1 marches left, 2 marches right.
; A row is admitted before the actor update; a new actor skips that update.
earth_march_tick:
    ld a,(terrain_step_pixels) ; ordinary terrain step is zero or one
    or a
    jr z,@horizontal
    ld hl,(iy+sprite_y)
    ld de,(iy+sprite_yvel)
    add hl,de
    ld (iy+sprite_y),hl
@horizontal:
    ld a,(iy+sprite_move_step)
    or a
    jr nz,@stride
    ld a,(iy+sprite_move_timer)
    or a
    jr z,@begin
    ld a,(terrain_step_pixels)
    or a
    jp z,side_visibility
    dec (iy+sprite_move_timer)
    jp nz,side_visibility
    ; This final wait tick already moved one full terrain pixel above.
    ld hl,128
    ld (iy+sprite_yvel),hl ; half speed beginning on the first sideways step
    jp side_visibility
@begin:
    ld a,1
    ld (iy+sprite_move_step),a
    ld (iy+sprite_move_timer),64 ; cols 4..15: four tiles of left travel
@stride:
    ld de,-256
    cp 1
    jr z,@advance
    ld de,256
@advance:
    ld (iy+sprite_xvel),de
    ld hl,(iy+sprite_x)
    add hl,de
    call side_bound_x ; footprint guard, never use an individual edge to time the turn
    dec (iy+sprite_move_timer)
    jp nz,side_visibility
    ld a,(iy+sprite_move_step)
    xor 3 ; 1 <-> 2
    ld (iy+sprite_move_step),a
    ld (iy+sprite_move_timer),64 ; twelve tiles wide in a sixteen-tile field
    jp side_visibility
