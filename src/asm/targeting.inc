    align 256

; targeting_computer variables
Bearing_t: dw #0000 ; 8.8 fixed
Heading_t: dw #0000 ; 8.8 fixed
Vp: dw #0000 ; 8.8 fixed
Vp_x: dw #0000 ; 8.8 fixed
Vp_y: dw #0000 ; 8.8 fixed
Vt: dw #0000 ; 8.8 fixed
Vt_x: dw #0000 ; 8.8 fixed
Vt_y: dw #0000 ; 8.8 fixed

; Inputs:   targeting_computer variables
; Note:     a call to orientation_to_player provides these inputs
; Outputs:  h.l is the 16.8 fixed firing angle256
;           b.c and d.e are the 16.8 fixed x,y component projectile velocities
; https://old.robowiki.net/cgi-bin/robowiki?LinearTargeting
targeting_computer:
; compute target velocity from x,y component velocities
    ld bc,(player_xvel) 
    ld de,(player_yvel)
    dec d ; account for vertical ground movement: b.c=player_xvel,d.e=player_yvel-1

    call cartesian_to_polar ; b.c=Heading_t, d.e=Vt
    ld (Heading_t),bc
    ld (Vt),de

; compute Heading_t-Bearing_t
    ld h,b
    ld l,c
    ld bc,(Bearing_t)
    and a ; clear carry
    sbc hl,bc ; h.l=Heading_t-Bearing_t

; compute sin(Heading_t-Bearing_t)
    ld b,h
    ld c,l
    call sin_bc ; h.l=sin(Heading_t-Bearing_t)

; compute (Vt*sin(Heading_t-Bearing_t))
    ex de,hl
    ld bc,(Vt)
    call BC_Mul_DE_88 ; h.l=(Vt*sin(Heading_t-Bearing_t))

; compute (Vt * sin(Heading_t-Bearing_t)) / Vp
    ld b,h
    ld c,l
    ld de,(Vp)
    call div_88 ; h.l=(Vt*sin(Heading_t-Bearing_t)) / Vp
; answer is in radians, convert to degrees256
    ex de,hl
    ld bc,#28BE ; 40.74=57.29578*256/360
    call BC_Mul_DE_88 

; add lead angle to target bearing
    ld de,(Bearing_t)
    add hl,de ; h.l=lead angle+target bearing
    push hl

; compute component projectile velocities
    ld b,h
    ld c,l
    ld de,(Vp)
    call polar_to_cartesian ; b.c=Vp_x, d.e=Vp_y

    ld (Vp_x),bc
    ld (Vp_y),de
    pop hl ; h.l=lead angle+target bearing
    ret
