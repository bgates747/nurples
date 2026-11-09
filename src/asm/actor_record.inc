; Shared sprite/active-tile record contract.
;
; Bytes 0..40 remain the legacy common header. Byte 41 is a generation token
; owned by the allocator/transform boundary. Bytes 42..63 belong to the actor
; type declared in its own include and must be initialized for every lifetime.
actor_record_size:             equ 64
actor_record_shared_end:       equ 41
actor_record_generation:       equ 41
actor_record_custom:           equ 42
actor_record_custom_size:      equ actor_record_size-actor_record_custom
actor_type_just_spawned_bit:   equ 7
actor_type_mask:               equ %01111111

; Begin a new logical lifetime in an already allocated and validated physical
; record. Generation zero is reserved for an uninitialized table slot.
; inputs: ix = destination record
; outputs: generation advanced; actor-owned tail zeroed; ix preserved
; destroys: a,bc,de,hl
actor_record_begin_lifetime_ix:
    ld a,(ix+actor_record_generation)
    inc a
    jp nz,@generation_ready
    inc a ; skip reserved generation zero after wrap
@generation_ready:
    ld (ix+actor_record_generation),a
    lea hl,ix+actor_record_custom
    ld bc,actor_record_custom_size
    xor a
    call clear_mem
    ret
; end actor_record_begin_lifetime_ix

; ez80asm has no ASSERT directive. Select an undefined symbol on a mismatch.
    .if actor_record_shared_end-actor_record_generation
        dl actor_record_generation_offset_mismatch
    .endif
    .if [actor_record_custom+actor_record_custom_size]-actor_record_size
        dl actor_record_custom_span_mismatch
    .endif
