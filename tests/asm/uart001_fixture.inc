; UART-001 controlled game-output fixture
; =======================================
; This file is copied into an isolated assembly tree by
; build/scripts/build_uart001_fixture.py. It is never included by the normal
; campaign build.
;
; The fixture lets normal game_init load the real AGNB assets and define the
; real sprite set. It then replaces do_game with three deterministic 120-frame
; explosion-group cohorts:
;   0: existing fragmented synchronous sprite commands
;   1: the same bytes in one synchronous packet
;   2: the same bytes through RC3 syscall 25h
;
; Explosions are the smallest real move_sprite_group cohort whose behavior can
; remain VDU-output-silent for the complete interval. Their motion is fixed and
; their animation timer is held above the cohort length.

UART001_MODE_FRAGMENTED: equ 0
UART001_MODE_PACKET_SYNC: equ 1
UART001_MODE_PACKET_ASYNC: equ 2
UART001_MODE_COUNT: equ 3
UART001_FRAMES_PER_MODE: equ 120
UART001_SPRITE_COUNT: equ explosions_max
UART001_COMMAND_BYTES_PER_SPRITE: equ 15
UART001_UPDATE_BYTES: equ 3
UART001_PACKET_BYTES: equ UART001_SPRITE_COUNT*UART001_COMMAND_BYTES_PER_SPRITE+UART001_UPDATE_BYTES
UART001_PACKET_CAPACITY: equ 128
UART001_UART0_LSR: equ 0c5h
UART001_UART_LSR_TEMT: equ 040h

UART001_FAULT_CONTEXT: equ 0
UART001_FAULT_SETUP: equ 1
UART001_FAULT_ACTOR: equ 2
UART001_FAULT_PACKET_LENGTH: equ 3
UART001_FAULT_PACKET_MISMATCH: equ 4
UART001_FAULT_SOURCE_TIMEOUT: equ 5
UART001_FAULT_TEMT_TIMEOUT: equ 6
UART001_FAULT_RECEIVER_FENCE: equ 7

; MOS RUN re-enters resident RAM. Rebuild every fixture-owned control field
; before the first dispatcher call rather than trusting assembled DB values.
uart001_process_init:
    xor a
    ld (uart001_active),a
    ld (uart001_complete),a
    ld (uart001_halted),a
    ld (uart001_mode),a
    ld (uart001_frame),a
    ld (uart001_frame+1),a
    ld (uart001_frame+2),a
    ld (uart001_mode_started),a
    ld (uart001_mode_started+1),a
    ld (uart001_mode_started+2),a
    ld (uart001_faults),a
    ld (uart001_callbacks),a
    ld (uart001_callbacks+1),a
    ld (uart001_callbacks+2),a
    ld (uart001_source_in_flight),a
    ld (uart001_done),a
    ld (uart001_buffer_index),a
    ld (uart001_reference_valid),a
    ld (uart001_setup_remaining),a
    ld (uart001_packet_length),a
    ld (uart001_packet_length+1),a
    ld (uart001_packet_length+2),a
    ld hl,uart001_packet_a
    ld (uart001_packet_current),hl
    ld (uart001_packet_write),hl
    ld hl,uart001_mode_elapsed
    ld bc,UART001_MODE_COUNT*3
    call clear_mem
    ret
; end uart001_process_init

; Called in place of do_game by the isolated source overlay.
uart001_do_game:
    ld a,(uart001_halted)
    or a
    jp nz,uart001_recover_or_hold
    ld a,(uart001_active)
    or a
    jp nz,uart001_tick

; Retain the real startup/resource-loading path until game_init publishes
; game_playing. The fixture begins only after every production sprite frame is
; installed in the VDP.
    call do_game
    ld hl,(game_state)
    ld de,game_playing
    or a
    sbc hl,de
    ret nz
    call uart001_context_valid
    jp c,@context_ok
    ld hl,uart001_faults
    set UART001_FAULT_CONTEXT,(hl)
    ld a,1
    ld (game_exit_requested),a
    ret
@context_ok:
    xor a
    ld (uart001_mode),a
    ld (uart001_buffer_index),a
    ld (uart001_reference_valid),a
    ld (uart001_source_in_flight),a
    ld (uart001_done),a
    ld (uart001_faults),a
    ld (uart001_callbacks),a
    ld (uart001_callbacks+1),a
    ld (uart001_callbacks+2),a
    call uart001_begin_mode
    ld a,1
    ld (uart001_active),a
    ret
; end uart001_do_game

; RC3's admitted application context is foreground ADL, actual MBASE zero,
; with interrupts enabled. This fixture is assembled ADL; verify the two
; runtime properties that the raw syscall cannot safely reject itself.
; outputs: carry set when valid
uart001_context_valid:
    ld a,mb
    or a
    jp nz,@invalid
    ld a,i
    jp po,@invalid
    scf
    ret
@invalid:
    xor a
    ret
; end uart001_context_valid

uart001_begin_mode:
    call uart001_setup_records
    jp nc,@failed
    xor a
    ld (uart001_frame),a
    ld (uart001_frame+1),a
    ld (uart001_frame+2),a
    ld hl,(timestamp_now)
    ld (uart001_mode_started),hl
    ret
@failed:
    ld hl,uart001_faults
    set UART001_FAULT_SETUP,(hl)
    ld a,1
    ld (game_exit_requested),a
    ret
; end uart001_begin_mode

; Hide every production slot, rebuild only the eight explosion records, and
; present them once outside the measured cohort. Production sprite definitions
; remain resident in the VDP.
; outputs: carry set on success
uart001_setup_records:
    xor a
    ld b,table_num_records
@hide_loop:
    push af
    push bc
    call vdu_sprite_select
    call vdu_sprite_hide
    pop bc
    pop af
    inc a
    djnz @hide_loop

    call table_init
    ld iy,uart001_positions
    ld a,UART001_SPRITE_COUNT
    ld (uart001_setup_remaining),a
@spawn_loop:
    ld bc,(iy+0)
    ld de,(iy+3)
    push iy
    ld hl,explosion_defaults
    ld ix,explosions_base
    ld a,explosions_max
    call spawn_sprite_from_coords
    pop iy
    jp nc,@failed
    res 7,(ix+sprite_type)
    ld a,0ffh
    ld (ix+sprite_animation),a
    ld (ix+sprite_animation_timer),a
    xor a
    ld (ix+sprite_frame),a
    ld hl,0
    ld (ix+sprite_xvel),hl
    ld (ix+sprite_yvel),hl

; Initial placement/show is setup traffic, not part of any measured control.
    ld a,(ix+sprite_id)
    call vdu_sprite_select
    xor a
    call vdu_sprite_select_frame
    ld bc,(ix+sprite_x)
    ld de,(ix+sprite_y)
    call vdu_sprite_move_abs168
    call vdu_sprite_show

    lea iy,iy+6
    ld hl,uart001_setup_remaining
    dec (hl)
    jp nz,@spawn_loop
; The production loop presents sprites when its later HUD drawing reaches the
; VDP. This isolated fixture has no later drawing operation, so make its setup
; presentation boundary explicit.
    call vdu_sprite_update
    scf
    ret
@failed:
    xor a
    ret
; end uart001_setup_records

uart001_tick:
    call uart001_packet_begin
    ld iy,explosions_base
    ld b,UART001_SPRITE_COUNT
@actor_loop:
    push bc
    ld (table_pointer),iy
    push iy
    call move_explosion
    pop iy
    bit sprite_alive,(iy+sprite_collisions)
    jp z,@actor_failed
    call uart001_packet_append_sprite

    ld a,(uart001_mode)
    cp UART001_MODE_FRAGMENTED
    jp nz,@next_actor
; The diagnostic reference is the unmodified helper sequence and ordering.
    ld a,(iy+sprite_id)
    call vdu_sprite_select
    ld a,(iy+sprite_frame)
    call vdu_sprite_select_frame
    ld bc,(iy+sprite_x)
    ld de,(iy+sprite_y)
    call vdu_sprite_move_abs168
@next_actor:
    lea iy,iy+table_record_size
    xor a
    ld (sprite_screen_edge),a
    pop bc
    djnz @actor_loop

; Include the explicit presentation boundary in the canonical packet. The
; fragmented control emits the same bytes through the established helper;
; packet modes send them as the packet tail.
    call uart001_packet_append_update
    ld a,(uart001_mode)
    or a
    jp nz,@packet_ready
    call vdu_sprite_update
@packet_ready:

    call uart001_check_packet
    jp nc,@halt
    call uart001_send_packet
    jp nc,@halt

    ld hl,(uart001_frame)
    inc hl
    ld (uart001_frame),hl
    ld de,UART001_FRAMES_PER_MODE
    or a
    sbc hl,de
    ret nz
    call uart001_finish_mode
    ret

@actor_failed:
    ld hl,uart001_faults
    set UART001_FAULT_ACTOR,(hl)
    pop bc
@halt:
    ld a,1
    ld (uart001_halted),a
    ret
; end uart001_tick

uart001_packet_begin:
    ld a,(uart001_buffer_index)
    or a
    ld hl,uart001_packet_a
    jp z,@selected
    ld hl,uart001_packet_b
@selected:
    ld (uart001_packet_current),hl
    ld (uart001_packet_write),hl
    ld hl,0
    ld (uart001_packet_length),hl
    ret
; end uart001_packet_begin

; Build exactly the bytes produced by select, select-frame and move-absolute168.
; The scratch buffer is copied immediately and is never lent to MOS.
; inputs: IY = stable explosion record
uart001_packet_append_sprite:
    ld a,(iy+sprite_id)
    ld (uart001_sprite_command+3),a
    ld a,(iy+sprite_frame)
    ld (uart001_sprite_command+7),a

; Match vdu_sprite_move_abs168's intentional one-byte-left 24-bit stores: the
; fractional byte is discarded and the two integer bytes enter the command.
    ld de,(iy+sprite_y)
    ld hl,origin_top*256
    add hl,de
    ld (uart001_sprite_command+12),hl
    ld de,(iy+sprite_x)
    ld hl,origin_left*256
    add hl,de
    ld (uart001_sprite_command+10),hl
    ld a,13
    ld (uart001_sprite_command+10),a

    ld hl,uart001_sprite_command
    ld de,(uart001_packet_write)
    ld bc,UART001_COMMAND_BYTES_PER_SPRITE
    ldir
    ld (uart001_packet_write),de
    ld hl,(uart001_packet_length)
    ld de,UART001_COMMAND_BYTES_PER_SPRITE
    add hl,de
    ld (uart001_packet_length),hl
    ret
; end uart001_packet_append_sprite

uart001_packet_append_update:
    ld hl,uart001_update_command
    ld de,(uart001_packet_write)
    ld bc,UART001_UPDATE_BYTES
    ldir
    ld (uart001_packet_write),de
    ld hl,(uart001_packet_length)
    ld de,UART001_UPDATE_BYTES
    add hl,de
    ld (uart001_packet_length),hl
    ret
; end uart001_packet_append_update

; Carry is set only when the packet is the expected length and is byte-identical
; to the first fragmented-control frame's canonical packet.
uart001_check_packet:
    ld hl,(uart001_packet_length)
    ld de,UART001_PACKET_BYTES
    or a
    sbc hl,de
    jp z,@length_ok
    ld hl,uart001_faults
    set UART001_FAULT_PACKET_LENGTH,(hl)
    xor a
    ret
@length_ok:
    ld a,(uart001_reference_valid)
    or a
    jp nz,@compare
    ld hl,(uart001_packet_current)
    ld de,uart001_reference_packet
    ld bc,UART001_PACKET_BYTES
    ldir
    ld a,1
    ld (uart001_reference_valid),a
    scf
    ret
@compare:
    ld hl,(uart001_packet_current)
    ld de,uart001_reference_packet
    ld c,UART001_PACKET_BYTES
@compare_loop:
    ld a,(de)
    cp (hl)
    jp nz,@mismatch
    inc de
    inc hl
    dec c
    jp nz,@compare_loop
    scf
    ret
@mismatch:
    ld hl,uart001_faults
    set UART001_FAULT_PACKET_MISMATCH,(hl)
    xor a
    ret
; end uart001_check_packet

; Carry set means the frame was emitted or accepted for asynchronous sending.
uart001_send_packet:
    ld a,(uart001_mode)
    cp UART001_MODE_FRAGMENTED
    jp z,@sent
    cp UART001_MODE_PACKET_SYNC
    jp z,@sync

; Build buffer N+1 while N may still be in flight, then serialize before the
; next submit. This preserves one MOS borrower without overwriting either slot.
    call uart001_wait_source_release
    ret nc
    call uart001_wait_temt
    ret nc
    xor a
    ld (uart001_done),a
    ld a,1
    ld (uart001_source_in_flight),a
    ld hl,(uart001_packet_current)
    ld bc,(uart001_packet_length)
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
    ld hl,(uart001_packet_current)
    ld bc,(uart001_packet_length)
    rst.lil 18h
    scf
    ret
; end uart001_send_packet

; Callback contract: primary A and one resident flag/counter only. No MOS/VDP,
; EI/DI, index/alternate registers, stack manipulation or resubmission.
uart001_callback:
    ld a,(uart001_callbacks)
    inc a
    ld (uart001_callbacks),a
    jp nz,@done
    ld a,(uart001_callbacks+1)
    inc a
    ld (uart001_callbacks+1),a
    jp nz,@done
    ld a,(uart001_callbacks+2)
    inc a
    ld (uart001_callbacks+2),a
@done:
    ld a,1
    ld (uart001_done),a
    ret
; end uart001_callback

; 65,536 foreground polls are a local progress bound, not an API timeout or
; cancellation. On failure the static source and callback remain resident.
uart001_wait_source_release:
    ld a,(uart001_source_in_flight)
    or a
    jp z,@released
    ld bc,0
@wait:
    ld a,(uart001_done)
    or a
    jp nz,@observed
    dec c
    jp nz,@wait
    djnz @wait
    ld hl,uart001_faults
    set UART001_FAULT_SOURCE_TIMEOUT,(hl)
    xor a
    ret
@observed:
    xor a
    ld (uart001_source_in_flight),a
@released:
    scf
    ret
; end uart001_wait_source_release

uart001_wait_temt:
    ld bc,0
@wait:
    in0 a,(UART001_UART0_LSR)
    and UART001_UART_LSR_TEMT
    jp nz,@empty
    dec c
    jp nz,@wait
    djnz @wait
    ld hl,uart001_faults
    set UART001_FAULT_TEMT_TIMEOUT,(hl)
    xor a
    ret
@empty:
    scf
    ret
; end uart001_wait_temt

uart001_finish_mode:
    call uart001_wait_source_release
    jp nc,@halt
    call uart001_wait_temt
    jp nc,@halt
; A fresh ordered VDP query follows physical drain. Its matching response is a
; receiver-execution token for these stock sprite commands, not optical scanout.
    ld a,128
    call vdu_get_text_colour
    jp c,@fence_ok
    ld hl,uart001_faults
    set UART001_FAULT_RECEIVER_FENCE,(hl)
@fence_ok:
    ld hl,(timestamp_now)
    ld de,(uart001_mode_started)
    or a
    sbc hl,de
    push hl
    ld a,(uart001_mode)
    ld l,a
    ld h,3
    mlt hl
    ld de,uart001_mode_elapsed
    add hl,de
    push hl
    pop ix
    pop hl
    ld (ix+0),hl

    ld a,(uart001_mode)
    inc a
    ld (uart001_mode),a
    cp UART001_MODE_COUNT
    jp nc,@complete
    call uart001_begin_mode
    ret
@complete:
    ld a,1
    ld (uart001_complete),a
    ld (game_exit_requested),a
    ret
@halt:
    ld a,1
    ld (uart001_halted),a
    ret
; end uart001_finish_mode

; A timed-out source wait is not cancellation. Continue observing without any
; ordinary VDU output; exit only after source release and physical drain.
uart001_recover_or_hold:
    call uart001_wait_source_release
    ret nc
    call uart001_wait_temt
    ret nc
    ld a,1
    ld (game_exit_requested),a
    ret
; end uart001_recover_or_hold

uart001_active: db 0
uart001_complete: db 0
uart001_halted: db 0
uart001_mode: db 0
uart001_frame: dl 0
uart001_mode_started: dl 0
uart001_mode_elapsed: blkb UART001_MODE_COUNT*3,0
uart001_faults: db 0
uart001_callbacks: dl 0
uart001_source_in_flight: db 0
uart001_done: db 0
uart001_buffer_index: db 0
uart001_reference_valid: db 0
uart001_setup_remaining: db 0
uart001_packet_current: dl uart001_packet_a
uart001_packet_write: dl uart001_packet_a
uart001_packet_length: dl 0

uart001_sprite_command:
    db 23,27,4,0
    db 23,27,10,0
    db 23,27,13
    dw 0,0

uart001_update_command:
    db 23,27,15

uart001_packet_a: blkb UART001_PACKET_CAPACITY,0
uart001_packet_b: blkb UART001_PACKET_CAPACITY,0
uart001_reference_packet: blkb UART001_PACKET_CAPACITY,0

uart001_positions:
    dl 64*256,64*256
    dl 112*256,64*256
    dl 160*256,64*256
    dl 208*256,64*256
    dl 64*256,128*256
    dl 112*256,128*256
    dl 160*256,128*256
    dl 208*256,128*256
