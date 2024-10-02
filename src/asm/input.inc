keyboard_masks: ds 16 ; 16 bytes for the keyboard masks 

; reset the local keyboard masks to zero
; inputs: none
; outputs: none
; clobbers: a, b, iy
reset_keys:
    ld iy,keyboard_masks
    xor a
    ld b,16 ; loop counter
@loop:
    ld (iy),a
    inc iy
    djnz @loop
    ret

; check system virtual keyboard map for keys down and or them with local keyboard masks
; inputs: none
; outputs: none
; clobbers: a, b, ix, iy
set_keys:
    MOSCALL mos_getkbmap ; ix points to the virtual keyboard map
    ld iy,keyboard_masks 
    ld b,16 ; loop counter
@loop:
    ld a,(ix)
    or (iy)
    ld (iy),a
    inc ix
    inc iy
    djnz @loop
    ret

; ============ KEYBOARD INPUT ============
do_input:
; get user keyboard input
    ld ix,keyboard_masks
    ld a,-1 ; anything non-zero to indicates no ship movement key pressed

; ------ QUIT ------
; 113 Escape
    bit 0,(ix+14)
	jp nz,main_end

; 99 Space fire zee missiles
    bit 2,(ix+12)
    jp z,@Space
@Space:

; 26 Left
    bit 1,(ix+3)
    jp z,@Left

    xor a
@Left:

; 122 Right
    bit 1,(ix+15)
    jp z,@Right

@Right:

; check keypress flag and branch accordingly
    and a 

    ret