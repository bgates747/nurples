dithering_type: db 0x00 ; 0=none, 1=bayer ordered matrix, 2=floyd-steinberg
current_image_id: dl 256 ; buffer_id of the current image
current_image_index: dl 0 ; index of the current image

keyboard_masks: ds 16 ; 16 bytes for the keyboard masks 

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

; ============ KEYBOARD INPUT ============
do_input:
; get user keyboard input
    ld ix,keyboard_masks
    ld a,-1 ; anything non-zero to indicates no key pressed

; ------ QUIT ------
; 113 Escape
    bit 0,(ix+14)
	jp nz,main_end

; ------ RENDERING OPTIONS ------
; ; 99 Space cycle through dithering types
;     bit 2,(ix+12)
;     call nz,cycle_dithering
;     xor a

; 26 Left
    bit 1,(ix+3)
    jp z,@Left
    ld de,(current_image_index)
    dec de
    ld (current_image_index),de
    xor a
@Left:

; 122 Right
    bit 1,(ix+15)
    jp z,@Right
    ld de,(current_image_index)
    inc de
    ld (current_image_index),de
    xor a
@Right:
; check keypress flag and branch accordingly
    and a
    jp z,rendbmp

; check slideshow timer and branch accordingly
    call tmr_slideshow_get
    jp p,no_move
    call tmr_slideshow_set
    ld de,(current_image_index)
    inc de
    ld (current_image_index),de
    jp rendbmp

; slideshow timer routines
tmr_slideshow: ds 6 ; buffer to hold slideshow timer
tmr_slideshow_reset: dl 10*120

; set a countdown timer for the slideshow using the timestamp timer
; returns: hl = current time
; destroys: af,hl,de,ix,iy
tmr_slideshow_set:
    ld iy,tmr_slideshow
    ld hl,(tmr_slideshow_reset)
    ld (iy+3),hl ; set time remaining
    call timestamp_tick
    ld hl,(timestamp_now)   ; get current timestamp
    ld (iy+0),hl ; set start time
    ret

; gets time remaining on the slideshow countdown timer following the global timestamp
; returns: hl pos = time remaining in 1/120ths of a second,hl neg = time past expiration
;          sign flags: pos = time not expired,zero or neg = time expired
; destroys: af,hl,de,ix,iy
tmr_slideshow_get:
    call timestamp_tick
    ld iy,tmr_slideshow
    call timestamp_tmr_get
    ret