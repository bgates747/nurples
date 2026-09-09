; Optional legacy GPIO joystick. These pins cannot also serve as Extender
; UART signals: PC3 low (CTS ready) looks like joystick Down and cancels Up.
; The splash alone patches two baked-in NOP pairs to JR when joystick is off.
; Patching happens before gameplay, with byte stores only (ADL is 24-bit).
; No per-direction option test may disturb A's sprite-frame accumulator.
; GPIO ports for reading joystick input
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
    ld d,0ffh ; all active-low joystick directions released if polling is skipped
player_joystick_direction_gate:
    nop
    nop
    xor a ; clear flags
    in a,(portC) ; read dpad input
    ld d,a ; save dpad input
player_keyboard_directions:
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
    ld bc,-player_speed
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
    ld bc,player_speed
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
    ld bc,-player_speed
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
    ld bc,player_speed
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
player_joystick_fire_gate:
    nop
    nop
    xor a ; clear flags
    in a,(portD) ; read joypad fire button input
    bit btn1_a,a
    jp z,player_spawn_laser
player_keyboard_fire:
; 99 Space: fire main weapon
    bit 2,(ix+12)
    jp nz,player_spawn_laser
; no fire button pressed so go home
    ret
; end player_input

; Called only while the splash owns execution, before any player input.
; Restore every byte so a later game/MOS RUN can reverse the previous choice.
; Destroys: af. Never changes GPIO configuration or UART ownership.
player_joystick_enable:
    xor a
    ld (player_joystick_direction_gate),a
    ld (player_joystick_direction_gate+1),a
    ld (player_joystick_fire_gate),a
    ld (player_joystick_fire_gate+1),a
    ret

player_joystick_disable:
    ld a,player_keyboard_directions-player_joystick_direction_gate-2
    ld (player_joystick_direction_gate+1),a
    ld a,player_keyboard_fire-player_joystick_fire_gate-2
    ld (player_joystick_fire_gate+1),a
    ld a,018h ; JR rel8; NOP and JR both preserve flags and A
    ld (player_joystick_direction_gate),a
    ld (player_joystick_fire_gate),a
    ret

; Reject a skip outside JR's forward range if either block later grows.
    .if [player_keyboard_directions-player_joystick_direction_gate-2] >> 7
        dl joystick_direction_skip_out_of_range
    .endif
    .if [player_keyboard_fire-player_joystick_fire_gate-2] >> 7
        dl joystick_fire_skip_out_of_range
    .endif
