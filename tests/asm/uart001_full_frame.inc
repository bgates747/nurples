; UART-001 deterministic full-frame continuation
; ================================================
; Included only by uart001_fixture.inc in the isolated build. The staging
; builder redirects every production RST 18h and RST 10h VDU write through
; the collectors below. Collection is dormant during startup and teardown.

UART001_FULL_FRAMES_PER_MODE: equ 1200
UART001_FULL_WARMUP_FRAMES: equ 120
UART001_FULL_MEASURED_FRAMES: equ UART001_FULL_FRAMES_PER_MODE-UART001_FULL_WARMUP_FRAMES
UART001_FULL_EXPECTED_TICKS: equ UART001_FULL_MEASURED_FRAMES*2
UART001_FULL_PACKET_CAPACITY: equ 2048
UART001_FULL_TRACE_SIZE: equ 9
UART001_FULL_REFERENCE_BYTES: equ UART001_FULL_FRAMES_PER_MODE*UART001_FULL_TRACE_SIZE

UART001_FULL_FAULT_OVERFLOW: equ 0
UART001_FULL_FAULT_TRACE: equ 1
UART001_FULL_FAULT_SCENE: equ 2
UART001_FULL_FAULT_CALLBACKS: equ 3
UART001_FULL_FAULT_RECEIVER: equ 4
UART001_FULL_FAULT_SETUP: equ 5

; MOS RUN re-enters resident RAM. Clear every control that could otherwise
; preserve a live collector, borrowed source, reference, or timing result.
uart001_full_process_init:
    xor a
    ld (uart001_full_active),a
    ld (uart001_full_capture_active),a
    ld (uart001_full_mode),a
    ld (uart001_full_frame),a
    ld (uart001_full_frame+1),a
    ld (uart001_full_frame+2),a
    ld (uart001_full_packet_length),a
    ld (uart001_full_packet_length+1),a
    ld (uart001_full_packet_length+2),a
    ld (uart001_full_faults),a
    ld hl,0ffffffh
    ld (uart001_full_first_mismatch),hl
    ld (uart001_full_have_previous_start),a
    ld (uart001_full_script_inputs),a
    ld (uart001_full_script_inputs+1),a
    ld (uart001_full_script_inputs+2),a
    ld (uart001_full_quit_latency),a
    ld (uart001_full_quit_latency+1),a
    ld (uart001_full_quit_latency+2),a
    ld hl,uart001_full_results_begin
    ld bc,uart001_full_results_end-uart001_full_results_begin
    call clear_mem
    ld hl,uart001_full_reference
    ld bc,UART001_FULL_REFERENCE_BYTES
    call clear_mem
    ld hl,uart001_full_packet_a
    ld (uart001_full_packet_current),hl
    ld (uart001_full_packet_write),hl
    ret
; end uart001_full_process_init

; Entered only after the accepted sprite-only controls have fully drained.
uart001_full_begin:
    xor a
    ld (uart001_active),a
    ld (uart001_full_mode),a
    ld (uart001_callbacks),a
    ld (uart001_callbacks+1),a
    ld (uart001_callbacks+2),a
    call uart001_full_setup_mode
    jp nc,@failed
    ld a,1
    ld (uart001_full_active),a
    ret
@failed:
    ld hl,uart001_full_faults
    set UART001_FULL_FAULT_SETUP,(hl)
    ld a,1
    ld (uart001_halted),a
    ret
; end uart001_full_begin

; Rebuild the same logical scene before every transport control. All setup
; traffic is direct and outside the measured stream; the previous mode has
; already crossed source-release, TEMT and receiver-execution boundaries.
uart001_full_setup_mode:
    xor a
    ld (uart001_full_capture_active),a
    ld (uart001_source_in_flight),a
    ld (uart001_done),a
    ld (uart001_buffer_index),a
    ld (uart001_full_frame),a
    ld (uart001_full_frame+1),a
    ld (uart001_full_frame+2),a
    ld (uart001_callbacks),a
    ld (uart001_callbacks+1),a
    ld (uart001_callbacks+2),a
    ld (uart001_full_have_previous_start),a
    ld hl,0
    ld (timestamp_now),hl
    ld (timestamp_old),hl
    ld (timestamp_chg),hl
    ld a,050h
    ld (r_seed),a

    call reset_runtime_tables
    ld hl,0
    ld (player_score),hl
    ld (player_starting_score),hl
    ld a,(player_ships_max)
    ld (player_ships),a
    ld hl,BUF_BG_STARS_0
    ld (bg_bufferId),hl
    ld a,tiles_dg
    call init_tileset
    call draw_player_cockpit
    call set_gfx_playing_field
    call init_bg
    call player_init
    ret nc
    ld hl,game_playing
    ld (game_state),hl
    ld hl,player_alive
    ld (player_state),hl
    ld hl,enemies_alive
    ld (enemies_state),hl
    xor a
    ld (player_event),a

; One representative of each sprite behavior starts alive. The production
; level and fixed fire trace add active tiles, lasers and later actors.
    ld hl,seeker_defaults
    ld ix,seekers_base
    ld a,seekers_max
    ld bc,48*256
    ld de,64*256
    call spawn_sprite_from_coords
    ret nc
    ld hl,speed_seeker
    ld (ix+sprite_vel),hl
    ld hl,move_enemy_seeker_orbit
    ld (ix+sprite_move_program),hl

    ld hl,fireball_defaults
    ld ix,fireballs_base
    ld a,fireballs_max
    ld bc,208*256
    ld de,80*256
    call spawn_sprite_from_coords
    ret nc
    ld hl,64*256
    ld (ix+sprite_heading),hl
    ld hl,0
    ld (ix+sprite_xvel),hl
    ld (ix+sprite_yvel),hl

    ld hl,explosion_defaults
    ld ix,explosions_base
    ld a,explosions_max
    ld bc,128*256
    ld de,144*256
    call spawn_sprite_from_coords
    ret nc
    ld hl,0
    ld (ix+sprite_xvel),hl
    ld (ix+sprite_yvel),hl

    call uart001_full_read_clock
    ld (uart001_full_mode_started),hl
    ld (uart001_full_previous_start),hl
    scf
    ret
; end uart001_full_setup_mode

; One complete logical frame per outer scheduler iteration. The outer loop
; retains its single VBlank and Escape sample. For async mode, packet N remains
; eligible to transmit while the next invocation computes/builds N+1.
uart001_full_tick:
    call uart001_full_read_clock
    ld (uart001_full_frame_started),hl
    call uart001_full_record_interval

; Production timers see the same 60 Hz logical clock in every control.
    ld hl,(uart001_full_frame)
    add hl,hl
    ld (timestamp_now),hl
    ld hl,2
    ld (timestamp_chg),hl

; Keep collision work live without allowing this transport fixture to leave
; PLAYING because cumulative incidental damage depleted the player.
    ld a,(player_shields_max)
    ld (player_shields),a
    ld ix,player_begin
    set sprite_alive,(ix+sprite_collisions)
    res sprite_just_died,(ix+sprite_collisions)
@shield_ready:

    call uart001_full_packet_begin
    ld a,1
    ld (uart001_full_capture_active),a
    call game_playing
; Establish an explicit presentation boundary for every logical frame.
    call vdu_sprite_update
    xor a
    ld (uart001_full_capture_active),a

    call uart001_full_read_clock
    ld de,(uart001_full_frame_started)
    or a
    sbc hl,de
    call uart001_full_record_update_ticks

    ld hl,(game_state)
    ld de,game_playing
    or a
    sbc hl,de
    jp z,@scene_valid
    ld hl,uart001_full_faults
    set UART001_FULL_FAULT_SCENE,(hl)
    jp @halt
@scene_valid:
    ld a,(uart001_full_faults)
    bit UART001_FULL_FAULT_OVERFLOW,a
    jp nz,@halt
    call uart001_full_hash_and_check
    jp nc,@halt

    call uart001_full_read_clock
    ld (uart001_full_send_started),hl
    call uart001_full_send_packet
    jp nc,@halt
    call uart001_full_read_clock
    ld de,(uart001_full_send_started)
    or a
    sbc hl,de
    call uart001_full_record_send_ticks
    call uart001_full_record_packet

    ld hl,(uart001_full_frame)
    inc hl
    ld (uart001_full_frame),hl
    ld de,UART001_FULL_FRAMES_PER_MODE
    or a
    sbc hl,de
    ret nz
    call uart001_full_finish_mode
    ret
@halt:
    ld a,1
    ld (uart001_halted),a
    ret
; end uart001_full_tick

uart001_full_packet_begin:
    ld a,(uart001_buffer_index)
    or a
    ld hl,uart001_full_packet_a
    jp z,@selected
    ld hl,uart001_full_packet_b
@selected:
    ld (uart001_full_packet_current),hl
    ld (uart001_full_packet_write),hl
    ld hl,0
    ld (uart001_full_packet_length),hl
    ret
; end uart001_full_packet_begin

; Staged replacement for every block VDU write. BC=0 retains MOS's terminated
; string convention. Registers and flags are preserved while collecting so all
; three controls execute identical game logic.
uart001_vdu_write:
    push af
    ld a,(uart001_full_capture_active)
    or a
    jp nz,@capture
    pop af
    rst.lil 18h
    ret
@capture:
    push bc
    push de
    push hl
    ld (uart001_full_capture_source),hl
    ld a,b
    or c
    jp nz,@have_length
    ld bc,0
@scan:
    ld a,(hl)
    or a
    jp z,@have_length
    inc hl
    inc bc
    jp @scan
@have_length:
    ld hl,(uart001_full_capture_source)
    call uart001_full_append
    pop hl
    pop de
    pop bc
    ld a,(uart001_full_mode)
    or a
    jp nz,@captured
    push bc
    push de
    push hl
    rst.lil 18h
    pop hl
    pop de
    pop bc
@captured:
    pop af
    ret
; end uart001_vdu_write

; Staged replacement for every single-byte VDU write.
uart001_vdu_write_a:
    push af
    ld (uart001_full_capture_char),a
    ld a,(uart001_full_capture_active)
    or a
    jp nz,@capture
    pop af
    rst.lil 10h
    ret
@capture:
    push bc
    push de
    push hl
    ld hl,uart001_full_capture_char
    ld bc,1
    call uart001_full_append
    pop hl
    pop de
    pop bc
    ld a,(uart001_full_mode)
    or a
    jp nz,@captured
    ld a,(uart001_full_capture_char)
    rst.lil 10h
@captured:
    pop af
    ret
; end uart001_vdu_write_a

; Append BC bytes from HL to the currently BUILDING static buffer.
uart001_full_append:
    ld a,b
    or c
    ret z
    ld (uart001_full_capture_source),hl
    ld (uart001_full_capture_length),bc
    ld hl,(uart001_full_packet_length)
    add hl,bc
    ld de,UART001_FULL_PACKET_CAPACITY+1
    or a
    sbc hl,de
    jp nc,@overflow
    add hl,de
    ld (uart001_full_packet_length),hl
    ld hl,(uart001_full_capture_source)
    ld de,(uart001_full_packet_write)
    ld bc,(uart001_full_capture_length)
    ldir
    ld (uart001_full_packet_write),de
    ret
@overflow:
    ld hl,uart001_full_faults
    set UART001_FULL_FAULT_OVERFLOW,(hl)
    ret
; end uart001_full_append

; Fragmented writes were already forwarded by the collectors. Packetized sync
; emits the identical aggregate once. Async waits only for source release before
; the next submit; TEMT is a mode/quit boundary, not a per-packet barrier.
uart001_full_send_packet:
    ld a,(uart001_full_mode)
    or a
    jp z,@sent
    cp UART001_MODE_PACKET_SYNC
    jp z,@sync
    call uart001_wait_source_release
    ret nc
    xor a
    ld (uart001_done),a
    ld a,1
    ld (uart001_source_in_flight),a
    ld hl,(uart001_full_packet_current)
    ld bc,(uart001_full_packet_length)
    ld de,uart001_callback
    ld a,025h
    rst.lil 08h
    ld a,(uart001_buffer_index)
    xor 1
    ld (uart001_buffer_index),a
@sent:
    scf
    ret
@sync:
    ld hl,(uart001_full_packet_current)
    ld bc,(uart001_full_packet_length)
    rst.lil 18h
    scf
    ret
; end uart001_full_send_packet

; Fixed input trace: 32 frames right, 32 neutral, 32 left, 32 neutral;
; request a laser every 32 frames. This replaces only the staged player-input
; dispatch and retains the real movement, laser timer, collision and renderer.
uart001_player_input:
    ld iy,player_begin
    ld (table_pointer),iy
    ld hl,0
    ld (iy+sprite_xvel),hl
    ld (iy+sprite_yvel),hl
    ld a,(uart001_full_frame)
    and 127
    cp 32
    jp c,@right
    cp 64
    jp c,@center
    cp 96
    jp c,@left
@center:
    ld a,1
    jp @present
@right:
    ld hl,player_speed
    ld (iy+sprite_xvel),hl
    ld a,2
    jp @present
@left:
    ld hl,-player_speed
    ld (iy+sprite_xvel),hl
    xor a
@present:
    push af
    ld a,(iy+sprite_id)
    call vdu_sprite_select
    pop af
    call vdu_sprite_select_frame
    ld (table_pointer),iy
    call move_sprite
    ld bc,(iy+sprite_x)
    ld de,(iy+sprite_y)
    call vdu_sprite_move_abs168
    ld a,(uart001_full_frame)
    and 31
    ret nz
    ld hl,(uart001_full_script_inputs)
    inc hl
    ld (uart001_full_script_inputs),hl
    jp player_spawn_laser
; end uart001_player_input

; A compact per-frame oracle: output length/hash plus actor/collision-state
; hash. Host capture remains the exact byte-stream authority; this oracle makes
; the guest fail at the first divergent logical frame.
uart001_full_hash_and_check:
    call uart001_full_hash_reset
    ld hl,(uart001_full_packet_current)
    ld bc,(uart001_full_packet_length)
    call uart001_full_hash_block
    ld hl,(uart001_full_packet_length)
    ld (uart001_full_current_trace+0),hl
    ld hl,(uart001_full_hash_value)
    ld (uart001_full_current_trace+3),hl

    call uart001_full_hash_reset
    ld iy,table_base
    ld b,table_num_records
@sprite_loop:
    push bc
    push iy
    pop hl
    ld bc,sprite_frame+1
    call uart001_full_hash_block
    lea iy,iy+table_record_size
    pop bc
    djnz @sprite_loop

    ld a,(num_active_tiles)
    or a
    jp z,@tiles_done
    ld b,a
    ld iy,tile_stack
@tile_loop:
    push bc
    ld hl,(iy)
    ld bc,tile_frame+1
    call uart001_full_hash_block
    lea iy,iy+3
    pop bc
    djnz @tile_loop
@tiles_done:
    ld hl,r_seed
    ld bc,1
    call uart001_full_hash_block
    ld hl,tiles_row_defs
    ld bc,8
    call uart001_full_hash_block
    ld hl,cur_baseBufferId
    ld bc,15
    call uart001_full_hash_block
    ld hl,bg_y_plot
    ld bc,6
    call uart001_full_hash_block
    ld hl,game_state
    ld bc,8
    call uart001_full_hash_block
    ld hl,player_state
    ld bc,7
    call uart001_full_hash_block
    ld hl,enemies_state
    ld bc,7
    call uart001_full_hash_block
    ld hl,player_score
    ld bc,6
    call uart001_full_hash_block
    ld hl,(uart001_full_hash_value)
    ld (uart001_full_current_trace+6),hl

    call uart001_full_reference_pointer
    ld a,(uart001_full_mode)
    or a
    jp nz,@compare
    push ix
    pop de
    ld hl,uart001_full_current_trace
    ld bc,UART001_FULL_TRACE_SIZE
    ldir
    scf
    ret
@compare:
    ld hl,uart001_full_current_trace
    ld b,UART001_FULL_TRACE_SIZE
@compare_loop:
    ld a,(ix+0)
    cp (hl)
    jp nz,@mismatch
    inc ix
    inc hl
    djnz @compare_loop
    scf
    ret
@mismatch:
    ld hl,uart001_full_faults
    set UART001_FULL_FAULT_TRACE,(hl)
    ld hl,(uart001_full_first_mismatch)
    ld de,0ffffffh
    or a
    sbc hl,de
    jp nz,@failed
    ld hl,(uart001_full_frame)
    ld (uart001_full_first_mismatch),hl
@failed:
    xor a
    ret
; end uart001_full_hash_and_check

uart001_full_hash_reset:
    ld hl,04c9dc5h
    ld (uart001_full_hash_value),hl
    ret

; 24-bit djb2-style fingerprint, H = H*33 XOR byte.
; Inputs: HL=source, BC=nonzero length below 65536.
uart001_full_hash_block:
    ld a,b
    or c
    ret z
    push ix
    push hl
    pop ix
@loop:
    ld hl,(uart001_full_hash_value)
    push hl
    add hl,hl
    add hl,hl
    add hl,hl
    add hl,hl
    add hl,hl
    pop de
    add hl,de
    ld a,l
    xor (ix+0)
    ld l,a
    ld (uart001_full_hash_value),hl
    lea ix,ix+1
    dec bc
    ld a,b
    or c
    jp nz,@loop
    pop ix
    ret
; end uart001_full_hash_block

uart001_full_reference_pointer:
    ld hl,(uart001_full_frame)
    push hl
    pop de
    add hl,hl
    add hl,hl
    add hl,hl
    add hl,de
    ld de,uart001_full_reference
    add hl,de
    push hl
    pop ix
    ret

uart001_full_finish_mode:
    call uart001_wait_source_release
    jp nc,@halt
    call uart001_wait_temt
    jp nc,@halt
    ld a,128
    call vdu_get_text_colour
    jp c,@fence_ok
    ld hl,uart001_full_faults
    set UART001_FULL_FAULT_RECEIVER,(hl)
@fence_ok:
    call uart001_full_read_clock
    ld de,(uart001_full_measure_started)
    or a
    sbc hl,de
    push hl
    ld de,uart001_full_mode_elapsed
    call uart001_full_mode_slot
    pop hl
    ld (ix+0),hl
    ld de,UART001_FULL_EXPECTED_TICKS
    or a
    sbc hl,de
    push hl
    ld de,uart001_full_mode_timer_drift
    call uart001_full_mode_slot
    pop hl
    ld (ix+0),hl

    ld hl,(uart001_callbacks)
    push hl
    ld de,uart001_full_mode_callbacks
    call uart001_full_mode_slot
    pop hl
    ld (ix+0),hl
    ld a,(uart001_full_mode)
    cp UART001_MODE_PACKET_ASYNC
    jp nz,@callbacks_zero
    ld de,UART001_FULL_FRAMES_PER_MODE
    or a
    sbc hl,de
    jp z,@callbacks_ok
    jp @callbacks_bad
@callbacks_zero:
    ld de,0
    or a
    sbc hl,de
    jp z,@callbacks_ok
@callbacks_bad:
    ld hl,uart001_full_faults
    set UART001_FULL_FAULT_CALLBACKS,(hl)
@callbacks_ok:
    ld a,(uart001_full_mode)
    inc a
    ld (uart001_full_mode),a
    cp UART001_MODE_COUNT
    jp nc,@complete
    call uart001_full_setup_mode
    jp nc,@setup_failed
    ret
@complete:
    xor a
    ld (uart001_full_active),a
    ld a,1
    ld (uart001_complete),a
    ld (game_exit_requested),a
    ret
@setup_failed:
    ld hl,uart001_full_faults
    set UART001_FULL_FAULT_SETUP,(hl)
@halt:
    ld a,1
    ld (uart001_halted),a
    ret
; end uart001_full_finish_mode

; Escape is patched into the staged outer loop. Stop collection, reconcile the
; one possible borrow, then cross physical and receiver boundaries before the
; production display teardown emits ordinary VDU traffic.
uart001_escape:
    call uart001_full_read_clock
    ld (uart001_full_quit_started),hl
    xor a
    ld (uart001_full_capture_active),a
    call uart001_wait_source_release
    ret nc
    call uart001_wait_temt
    ret nc
    ld a,128
    call vdu_get_text_colour
    jp c,@fence_ok
    ld hl,uart001_full_faults
    set UART001_FULL_FAULT_RECEIVER,(hl)
@fence_ok:
    call uart001_full_read_clock
    ld de,(uart001_full_quit_started)
    or a
    sbc hl,de
    ld (uart001_full_quit_latency),hl
    ret
; end uart001_escape

uart001_full_read_clock:
    MOSCALL mos_sysvars
    ld hl,(ix+sysvar_time)
    ret

; Timing and accounting helpers. Warm-up frames are traced but excluded.
uart001_full_record_interval:
    ld a,(uart001_full_have_previous_start)
    or a
    jp z,@remember
    ld hl,(uart001_full_frame)
    ld de,UART001_FULL_WARMUP_FRAMES
    or a
    sbc hl,de
    jp c,@remember
    jp nz,@measure
    ld hl,(uart001_full_frame_started)
    ld (uart001_full_measure_started),hl
    jp @remember
@measure:
    ld hl,(uart001_full_frame_started)
    ld de,(uart001_full_previous_start)
    or a
    sbc hl,de
    push hl
    ld de,uart001_full_mode_interval_max
    call uart001_full_mode_slot
    pop hl
    call uart001_full_update_max
    ld de,2
    or a
    sbc hl,de
    jp c,@remember
    jp z,@remember
    ld de,uart001_full_mode_missed
    call uart001_full_mode_slot
    ld hl,(ix+0)
    inc hl
    ld (ix+0),hl
@remember:
    ld hl,(uart001_full_frame_started)
    ld (uart001_full_previous_start),hl
    ld a,1
    ld (uart001_full_have_previous_start),a
    ret

uart001_full_record_update_ticks:
    push hl
    ld hl,(uart001_full_frame)
    ld de,UART001_FULL_WARMUP_FRAMES
    or a
    sbc hl,de
    pop hl
    ret c
    push hl
    ld de,uart001_full_mode_update_max
    call uart001_full_mode_slot
    pop hl
    jp uart001_full_update_max

uart001_full_record_send_ticks:
    push hl
    ld hl,(uart001_full_frame)
    ld de,UART001_FULL_WARMUP_FRAMES
    or a
    sbc hl,de
    pop hl
    ret c
    push hl
    ld de,uart001_full_mode_send_max
    call uart001_full_mode_slot
    pop hl
    jp uart001_full_update_max

uart001_full_update_max:
    ld de,(ix+0)
    push hl
    or a
    sbc hl,de
    pop hl
    ret c
    ret z
    ld (ix+0),hl
    ret

uart001_full_record_packet:
    ld hl,(uart001_full_frame)
    ld de,UART001_FULL_WARMUP_FRAMES
    or a
    sbc hl,de
    ret c
    ld de,uart001_full_mode_bytes
    call uart001_full_mode_slot
    ld hl,(ix+0)
    ld de,(uart001_full_packet_length)
    add hl,de
    ld (ix+0),hl
    ld de,uart001_full_mode_packet_max
    call uart001_full_mode_slot
    ld hl,(uart001_full_packet_length)
    jp uart001_full_update_max

; Input DE=array base, output IX=&array[mode*3].
uart001_full_mode_slot:
    ld a,(uart001_full_mode)
    ld hl,0
    ld l,a
    ld h,3
    mlt hl
    add hl,de
    push hl
    pop ix
    ret

uart001_full_active: db 0
uart001_full_capture_active: db 0
uart001_full_mode: db 0
uart001_full_frame: dl 0
uart001_full_faults: db 0
uart001_full_first_mismatch: dl 0ffffffh
uart001_full_packet_current: dl uart001_full_packet_a
uart001_full_packet_write: dl uart001_full_packet_a
uart001_full_packet_length: dl 0
uart001_full_capture_source: dl 0
uart001_full_capture_length: dl 0
uart001_full_capture_char: db 0
uart001_full_hash_value: dl 0
uart001_full_current_trace: blkb UART001_FULL_TRACE_SIZE,0
uart001_full_mode_started: dl 0
uart001_full_measure_started: dl 0
uart001_full_frame_started: dl 0
uart001_full_previous_start: dl 0
uart001_full_send_started: dl 0
uart001_full_have_previous_start: db 0
uart001_full_quit_started: dl 0
uart001_full_quit_latency: dl 0
uart001_full_script_inputs: dl 0

uart001_full_results_begin:
uart001_full_mode_elapsed: blkb UART001_MODE_COUNT*3,0
uart001_full_mode_timer_drift: blkb UART001_MODE_COUNT*3,0
uart001_full_mode_callbacks: blkb UART001_MODE_COUNT*3,0
uart001_full_mode_bytes: blkb UART001_MODE_COUNT*3,0
uart001_full_mode_packet_max: blkb UART001_MODE_COUNT*3,0
uart001_full_mode_interval_max: blkb UART001_MODE_COUNT*3,0
uart001_full_mode_update_max: blkb UART001_MODE_COUNT*3,0
uart001_full_mode_send_max: blkb UART001_MODE_COUNT*3,0
uart001_full_mode_missed: blkb UART001_MODE_COUNT*3,0
uart001_full_results_end:

uart001_full_packet_a: blkb UART001_FULL_PACKET_CAPACITY,0
uart001_full_packet_b: blkb UART001_FULL_PACKET_CAPACITY,0
uart001_full_reference: blkb UART001_FULL_REFERENCE_BYTES,0
