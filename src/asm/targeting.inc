    align 256

; compute orientation to player 
; based on relative positions
; returns: h.l 16.8 fixed angle256 to player
;    ub.c and ud.e as 16.8 signed fixed point numbers
;    representing delta-x/y *to* target respectively
orientation_to_player:
    ld iy,(table_pointer) 
    ld bc,(iy+sprite_x)
    ld de,(iy+sprite_y)
    ld ix,(player_x)
    ld iy,(player_y)
    call dxy168
    push bc
    push de
    call atan2_168fast
    pop de
    pop bc
    ld iy,(table_pointer) 
    ret
; end orientation_to_player

; targeting_computer variables fixed 16.8 format
Bearing_t: dl 0x000000 ; angle to target
Heading_t: dl 0x000000 ; angle of target motion
Vp: dl 0x000000 ; velocity of projectile
Vp_x: dl 0x000000 ; x component of projectile velocity
Vp_y: dl 0x000000 ; y component of projectile velocity
Vt: dl 0x000000 ; velocity of target
Vt_x: dl 0x000000 ; x component of target velocity
Vt_y: dl 0x000000 ; y component of target velocity

; Inputs:   Bearing_t, Vp
; Note:     a call to orientation_to_player provides Bearing_t
; Outputs:  uh.l is the 16.8 fixed firing angle256
;           ub.c and ud.e are the 16.8 fixed x,y component projectile velocities
; https://old.robowiki.net/cgi-bin/robowiki?LinearTargeting
targeting_computer:
; compute target velocity from x,y component velocities
    ld bc,(player_xvel) 
    ld hl,(player_yvel)
    ld de,-1*256 ; account for vertical ground movement
    add hl,de
    ex de,hl ; ud.e = player_yvel-1
    call cartesian_to_polar ; uh.l = Heading_t, ud.e = Vt
    ld (Heading_t),hl
    ld (Vt),de

; Bearing_t is already set by orientation_to_player
; compute Heading_t-Bearing_t
    ld de,(Bearing_t)
    and a ; clear carry
    sbc hl,de ; Uh.l = Heading_t-Bearing_t

; compute sin(Heading_t-Bearing_t)
    call sin168 ; uh.l = sin(Heading_t-Bearing_t)

; compute (Vt*sin(Heading_t-Bearing_t))
    ld de,(Vt)
    call smul168 ; uh.l = (Vt*sin(Heading_t-Bearing_t))

; compute (Vt * sin(Heading_t-Bearing_t)) / Vp
    ld de,(Vp)
    call sdiv168 ; uh.l = (Vt*sin(Heading_t-Bearing_t)) / Vp
; answer is in radians, convert to degrees256
    ld de,0x0028BE ; 40.74 = 57.29578*256/360
    call smul168 ; uh.l = angle in degrees256 

; add lead angle to target bearing
    ld de,(Bearing_t)
    add hl,de ; h.l = lead angle+target bearing
    push hl

; compute component projectile velocities
    ld de,(Vp)
    call polar_to_cartesian ; ub.c = Vp_x, ud.e = Vp_y
    ld (Vp_x),bc
    ld (Vp_y),de
    pop hl ; uh.l = lead angle+target bearing
    ret
