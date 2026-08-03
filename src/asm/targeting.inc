; compute orientation to player 
; based on relative positions
; returns: uh.l 16.8 fixed angle256 to player
;    ub.c and ud.e as 16.8 signed fixed point numbers
;    representing delta-x/y *to* target respectively
; inputs: iy pointed at sprite record
; destroys: everything except iy (sprite record pointer)
orientation_to_player:
    ld ix,player_begin
; compute dx
    ld hl,(ix+sprite_x)
    ld de,(iy+sprite_x)
    or a ; clear carry
    sbc hl,de ; dx
    push hl ; save dx
; compute dy
    ld hl,(ix+sprite_y)
    ld de,(iy+sprite_y)
    or a ; clear carry
    sbc hl,de ; dy
    ex de,hl ; de = dy
; compute angle to player
    pop bc ; dx (was hl)
    push bc ; save dx
    push de ; save dy
    call atan2_168fast ; uh.l = angle to player
    ld a,%11100000 
    and l ; round angle to nearest 32th of a circle
    ld l,a
    ld (Bearing_t),hl ; return angle to player
    pop de ; restore dy
    pop bc ; restore dx
    ret
; end orientation_to_player

; compute orientation to player 
; based on relative positions
; returns: uh.l 16.8 fixed angle256 to player
;    ub.c and ud.e as 16.8 signed fixed point numbers
;    representing delta-x/y *to* target respectively
; inputs: iy pointed at tile record
; destroys: everything except iy (tile record pointer)
tile_orientation_to_player:
    ld ix,player_begin
; compute dx
    ld hl,(ix+sprite_x)
    ld de,(iy+tile_x)
    or a ; clear carry
    sbc hl,de ; dx
    push hl ; save dx
; compute dy
    ld hl,(ix+sprite_y)
    ld de,(iy+tile_y)
    or a ; clear carry
    sbc hl,de ; dy
    ex de,hl ; de = dy
; compute angle to player
    pop bc ; dx (was hl)
    call atan2_168fast ; uh.l = angle to player
    ld a,%11100000 
    and l ; round angle to nearest 32th of a circle
    ld l,a
    ld (Bearing_t),hl
    ret
; end tile_orientation_to_player

; compute orientation to coordinate 
; based on relative positions
; returns: uh.l 16.8 fixed angle256 to coordinate
;    ub.c and ud.e as 16.8 signed fixed point numbers
;    representing delta-x/y *to* target respectively
; inputs: iy pointed at sprite record, ub.c,ud.e are target x,y coordinates
; destroys: everything except iy (sprite record pointer)
bearing_to_coord:
; compute dx
    push bc
    pop hl
    ld bc,(iy+sprite_x)
    or a ; clear carry
    sbc hl,bc ; dx
    push hl ; save dx
; compute dy
    ex de,hl
    ld de,(iy+sprite_y)
    or a ; clear carry
    sbc hl,de ; dy
    ex de,hl ; de = dy
; compute angle to coordinate
    pop bc ; dx (was hl)
    push bc ; save dx
    push de ; save dy
    call atan2_168fast ; uh.l = angle to coordinate
    ld a,%11100000 
    and l ; round angle to nearest 32th of a circle
    ld l,a
    ld (Bearing_t),hl ; return angle to coordinate
    pop de ; restore dy
    pop bc ; restore dx
    ret
; end bearing_to_coord

; targeting_computer variables fixed 16.8 format
Bearing_t: dl 0x000000 ; angle to target
Heading_t: dl 0x000000 ; angle of target motion
Vp: dl 0x000000 ; velocity of projectile
Vp_x: dl 0x000000 ; x component of projectile velocity
Vp_y: dl 0x000000 ; y component of projectile velocity
Vt: dl 0x000000 ; velocity of target
Vt_x: dl 0x000000 ; x component of target velocity
Vt_y: dl 0x000000 ; y component of target velocity
Heading_p: dl 0x000000 ; angle of projectile motion
lead_angle: dl 0x000000 ; lead angle

; Inputs:   Vp velocity of projectile set,
;           iy pointed to sprite record, 
;           player position and velocities correctly set
; Outputs:  uh.l is the 16.8 fixed firing angle256
;           ub.c and ud.e are the 16.8 fixed x,y component projectile velocities
; Law-of-sines derivation and NanoLinearTargeting small-angle approximation:
; https://old.robowiki.net/cgi-bin/robowiki?LinearTargeting
; Archived mirror:
; https://robowiki.net/wiki/Linear_Targeting/Archived_Talk_20071111
targeting_computer:
    ld ix,player_begin
; compute target speed and heading from its component velocities
    ld bc,(ix+sprite_xvel)
    ld de,(ix+sprite_yvel)
    ld hl,-1*256 ; account for vertical ground movement
    add hl,de
    ex de,hl ; ud.e = player y velocity - ground scroll velocity
    call cartesian_to_polar_sm ; uh.l = Heading_t, ud.e = Vt
    ld (Heading_t),hl
    ld (Vt),de
; B = target heading - bearing to target
    ld de,(Bearing_t)
    or a ; clear carry
    sbc hl,de
; numerator = Vt * sin(B), the target's lateral velocity
    call sin168
    ld de,(Vt)
    call smul168
; sin(A) = Vt * sin(B) / Vp
    ld de,(Vp)
    call sdiv168 ; quotient is returned in ud.e, remainder in uh.l
    ex de,hl
; NanoLinearTargeting drops asin and treats its argument as radians. Convert
; that small-angle result to angle256: 256/(2*pi) = 40.743665...
    ld de,0x0028BE ; 40.7421875 in 16.8 fixed
    call smul168
    ld (lead_angle),hl
; projectile heading = bearing + lead angle
    ld de,(Bearing_t)
    add hl,de

    push hl

; compute component projectile velocities
    ld de,(Vp) ; Vp should have been set by caller
    call polar_to_cartesian ; ub.c = Vp_x, ud.e = Vp_y
    ld (Vp_x),bc
    ; inc d ; account for vertical ground movement
    ld (Vp_y),de
    pop hl ; uh.l = lead angle+target bearing
    ld (Heading_p),hl

    ret

; debug print the results
    PUSH_ALL
    call vdu_home_cursor
    call printNewLine
    call printInline
    asciz "Bt "
    ld hl,(Bearing_t)
    call print_s168_hl

    ; call printInline
    ; asciz "\r\nHt "
    ; ld hl,(Heading_t)
    ; call print_s168_hl

    ; call printInline
    ; asciz "\r\nVt "
    ; ld hl,(Vt)
    ; call print_s168_hl

    ; call printInline
    ; asciz "\r\nVtx"
    ; ld hl,(ix+sprite_xvel)
    ; call print_s168_hl

    ; call printInline
    ; asciz "\r\nVty"
    ; ld hl,(ix+sprite_yvel)
    ; ld de,-1*256 ; account for vertical ground movement
    ; add hl,de
    ; call print_s168_hl

    call printInline
    asciz "\r\nLda"
    ld hl,(lead_angle)
    call print_s168_hl

    call printInline
    asciz "\r\nHp "
    ld hl,(Heading_p)
    call print_s168_hl

    ; call printInline
    ; asciz "\r\nVp "
    ; ld hl,(Vp)
    ; call print_s168_hl

    ; call printInline
    ; asciz "\r\nVpx"
    ; ld hl,(Vp_x)
    ; call print_s168_hl

    ; call printInline
    ; asciz "\r\nVpy"
    ; ld hl,(Vp_y)
    ; call print_s168_hl

    POP_ALL
    ret
