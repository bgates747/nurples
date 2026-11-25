enemies_state: dl enemies_alive
enemies_fault_bad_state: equ 0
enemies_fault_flags: db 0 ; latched fail-closed diagnostics

do_enemies: 
    ld hl,(enemies_state)
    call enemies_state_pointer_valid
    jp c,@dispatch
    ld hl,enemies_fault_flags
    set enemies_fault_bad_state,(hl)
    ld hl,enemies_dead ; actor-local fail-closed state for the next tick
    ld (enemies_state),hl
    ret
@dispatch:
    CALL_HL
    ret

; Exact stabilization whitelist for writable legacy enemy-state pointers.
enemies_state_pointer_valid:
    ld de,enemies_alive
    call @matches
    jp z,@valid
    ld de,enemies_despawning
    call @matches
    jp z,@valid
    ld de,enemies_dead
    call @matches
    jp z,@valid
    ld de,enemies_respawning
    call @matches
    jp z,@valid
    xor a
    ret
@valid:
    scf
    ret
@matches:
    push hl
    or a
    sbc hl,de
    pop hl
    ret
; end enemies_state_pointer_valid

enemies_alive:
    ld iy,enemy_sprites_begin
    ld b,table_num_enemy_records
    call move_sprite_group
    ret

enemies_despawning:
    ret

enemies_dead:
    ret

enemies_respawning:
    ret
