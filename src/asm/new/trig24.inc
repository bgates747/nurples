
; convert signed angles from a 360 to 256 degree circle
; inputs: uh.l is the angle360 in 16.8 fixed format
; outputs: uh.l is the angle256 in 16.8 fixed format
; destroys: TODO
deg_360_to_256:
	push de ; preserve de
; make angle positive and store sign flag
	call hlu_abs
	push af
; multiply by 256 first to keep precision
	hlu_mul256 ; uh.l = uh.l * 256
; divide uh.l by 360
	ld de,360
	call udiv24 ; ud.e = degrees / 360
	ex de,hl    ; uh.l = degrees / 360
; restore sign flag and adjust output accordingly
	pop af
	jp p,@pos ; positive number
	call neg_hlu
@pos:
; restore de and return uh.l as the result
	pop de
	ret

; fixed 16.8 routine
; cos(uh.l) --> uh.l
; destroys: de
cos168:
; for cos we simply increment the angle by 90 degrees
; or 0x004000 in 16.8 degrees256
; which makes it a sin problem
    ld de,0x004000
    add hl,de ; modulo 256 happens below
; fall through to sin168

; ---------------------
; fixed 16.8 routine
; sin(uh.l) --> uh.l
; destroys: af
sin168:
	push de
	push ix
; handle negative angles appropriately
	call hlu_abs
	jp p,@f
	ld de,-256*256
	add hl,de
@@:
; get the lookup value for the integer portion of the angle
	ld a,l ; save fractional part of the angle
	ld l,6 ; multiply by 6 to get our lookup index
	ex de,hl ; can't add ix,hl
	mlt de ; gosh that is handy
    ld ix,sin_lut_816 ; grab the lut address
    add ix,de ; bump hl by the index
    ld hl,(ix) ; hl = sin(int(angle))
	or a  ; check fractional part of the angle for zero
	jp z,@f ; no fractional part so skip interpolation
; interpolate the fractional part of the angle
	ld de,(ix+3)
	ex de,hl
	call smul24x8
	call hlu_sdiv256
	add hl,de ; hl = sin(int(angle)) + interpolation factor * (sin(int(angle+1)) - sin(int(angle))
@@:
	call hlu_sdiv256
	pop ix
	pop de
    ret ; and out

; 16.8 fixed inputs / outputs
; takes: uh.l as angle in degrees 256
;        ud.e as radius
; returns ub.c as dx, ud.e as dy, uh.l as radius
;        displacements from origin (0,0)
; destroys: everything except indexes
polar_to_cartesian:
	; call printNewLine ; DEBUG
; back up input parameters
    ld (@angle), hl
    ld (@radius), de
; compute dx = cos(uh.l) * ud.e
    call cos168 ; uh.l = cos(uh.l)
	; call print_hex_hl ; DEBUG
	; call print_s168_hl ; DEBUG
	ld de,(@radius)
	call smul168 ; uh.l = dx
    push hl      
; compute dy = sin(uh.l) * ud.e
    ld hl,(@angle)
    call sin168 ; uh.l = sin(uh.l)
	; call print_hex_hl ; DEBUG
	; call print_s168_hl ; DEBUG
    ld de,(@radius)
    call smul168    ; uh.l = dy
    ex de,hl       ; de = dy for output
    pop bc          ; bc = dx for output
; and out
    ret
@angle: ds 3
@radius: ds 3

; inputs: ub.c and ud.e are x0 and y0 in 16.8 fixed format
;         ui.x and ui.y are x1 and y1 in 16.8 fixed format
; output: ub.c and ud.e are dx and dy in 16.8 fixed format
;         also populates scratch locations dx168 and dy168
; destroys: a,hl,bc,de
dxy168:
; compute dx = x1-x0
    xor a ; clear carry
    push ix ; move ix to hl via the stack
    pop hl ; hl = x1
    sbc hl,bc ; hl = dx
    ld (dx168),hl ; dx to scratch
; compute dy = y1-y0
    xor a ; clear carry
    push iy ; move iy to hl via the stack
    pop hl ; hl = y1
    sbc hl,de ; hl = dy
    ld (dy168),hl ; dy to scratch
; populate output registers and return
    ex de,hl        ; ud.e = dy
    ld bc,(dx168)   ; ub.c = dx
    ret

; compute the euclidian distance between two cartesian coordinates
; using the formula d = sqrt(dx^2+dy^2
; inputs: ub.c and ud.e are x0 and y0 in 16.8 fixed format
;         ui.x and ui.y are x1 and y1 in 16.8 fixed format
; output; uh.l is the 16.8 fixed format distance
;       dx168/y are the 16.8 fixed format dx and dy
; destroys: a,hl,bc,de
distance168:
; compute dx = x1-x0
    xor a ; clear carry
    push ix ; move ix to hl via the stack
    pop hl ; hl = x1
    sbc hl,bc ; hl = dx
    ld (dx168),hl ; dx to scratch
; ; test dx for overflow
; 	ld de,0x007F00 ; max positive 16.8 value we can square without overflow
; 	ex de,hl
; 	sbc hl,de ; test for overflow
; 	push af ; carry indicates overflow
; compute dy = y1-y0
    xor a ; clear carry
    push iy ; move iy to hl via the stack
    pop hl ; hl = y1
    sbc hl,de ; hl = dy
    ld (dy168),hl ; dy to scratch
; ; test dy for overflow
; 	ld de,0x007F00 ; max positive 16.8 value we can square without overflow
; 	ex de,hl
; 	sbc hl,de ; test for overflow
; 	push af ; carry indicates overflow
; compute dy^2
	ld hl,(dy168)
    call hlu_abs  ; make dy positive so we can use unsigned multiply
    ; call hlu_div2 ; divide hlu by 2 to give us some headroom
    push hl ; load hl/2 to bc via the stack
    pop bc ; bc = dy/2
    ex de,hl ; de = dy/2
    call umul168 ; uh.l = dy^2/2
    push hl ; dy^2/2 to the stack
; compute dx^2
    ld hl,(dx168) ; get back dx
    call hlu_abs  ; make dx positive so we can use unsigned multiply
    ; call hlu_div2 ; divide hlu by 2 to give us some headroom
    push hl ; load hl/2 to bc via the stack
    pop bc ; bc = dx/2
    ex de,hl ; de = dx/2
    call umul168 ; uh.l = dx^2/2
; commpute dy^2+dx^2
    pop de ; get back dx^2/2
    add hl,de ; hl = dx^2/2+dy^2/2
; compute sqrt(dx^2/2+dy^2/2)
    call sqrt168 ; uh.l = distance/2
    ; add hl,hl ; hl = distance
; ; check for overflow
; 	pop af ; get back the overflow flags
; 	sbc a,a ; will be -1 if overflow, 0 if not
; 	ld b,a ; save the overflow flag
; 	pop af ; get back the overflow flags
; 	sbc a,a ; will be -1 if overflow, 0 if not
; 	add a,b ; if a != 0 then we had overflow
;     ret z ; no overflow we're done
; @overflow:
; 	ld hl,0x7FFFFF ; max positive 16.8 fixed value indicates overflow
	ret
@scratch: ds 6
dx168: ds 6
dy168: ds 6

; atan2(ub.c,ud.e) --> uh.l
; inputs: ub.c and ud.e are dx and dy in 16.8 fixed format
;   whether inputs are integers or fractional doesn't matter
;   so long as the sign bit of the upper byte is correct
; output: uh.l is the 16.8 fixed angle in degrees 256
; angles are COMPASS HEADINGS based on
; screen coordinate conventions,where the y axis is flipped
; #E0 315      0       45 #20
;        -x,-y | +x,-y
; #C0 270------+------ 90 #40
;        -x,+y | +x,+y
; #A0 225   180 #80   135 #60
atan2_168game:
; get signs and make everything positive
; get abs(x) and store its original sign
    push bc
    pop hl
    call hlu_abs ; if x was negative this also sets the sign flag
    push hl ; store abs(x)
    pop bc ; bc = abs(x)
    push af ; store sign of x
; get abs(y) and store its original sign
    ex de,hl ; hl = y
    call hlu_abs ; if y was negative this also sets the sign flag
    ex de,hl ; de = abs(y)
    push af ; store sign of y
; if abs(bc) < abs(de),then we do bc/de,otherwise de/bc
; this ensures that our lookup value is between 0 and 1 inclusive
    xor a ; clear the carry flag
    push de
    pop hl
    sbc hl,bc
    push af ; save sign of de - bc
    jp p,@1 ; bc <= de, so we skip ahead
; otherwise we swap bc and de
    push bc
    pop hl
    ex de,hl
    push hl
    pop bc
@1:
; now we're ready to snag our preliminary result
    call atan_168game ; uh.l comes back with prelim result
; now we adjust uh.l based on sign of de - bc
    pop af
    jp p,@2 ; bc <= de,so we skip ahead
    ex de,hl
    ld hl,0x004000 ; 90 degrees
    xor a ; clear the carry flag
    sbc hl,de ; subtract result from 90 degrees
    ; ld de,0 ; prep to clear hlu
    ; ld d,h
    ; ld e,l
    ; ex de,hl ; now we have 0 <= uh.l < 256 in 16.8 fixed format
    ; fall through
@2:
; now the fun part of adjusting the result
; based on which quadrant (x,y) is in
; #E0 315      0       45 #20
;        -x,-y | +x,-y
; #C0 270------+------ 90 #40
;        -x,+y | +x,+y
; #A0 225   180 #80   135 #60
    pop af ; sign of y
    jp z,@y_zero
    jp p,@y_pos
; y neg,check x
    pop af ; sign of x
    jp z,@y_neg_x_zero
    jp p,@y_neg_x_pos
; y neg,x neg
; angle is 270-360
; negating the intermediate does the trick
    call neg_hlu
    jr @zero_hlu

@y_neg_x_zero:
; y neg,x zero
; angle is 0
    ld hl,0
    ret
@y_neg_x_pos:
; y neg,x pos
; angle is 0 to 90
; so we're good
    ret

@y_pos:
    pop af ; sign of x
    jp z,@y_pos_x_zero
    jp p,@y_pos_x_pos
; y pos,x neg
; angle is 180-270
; so we add 180 to intermediate
    ld de,0x008000
    add hl,de
    jr @zero_hlu
@y_pos_x_zero:
; y pos,x zero
; angle is 180
    ld hl,0x008000
    ret
@y_pos_x_pos:
; y pos,x pos
; angle is 90-180
; neg the intermediate and add 180 degrees
    call neg_hlu
    ld de,0x008000
    add hl,de
    jr @zero_hlu

@y_zero:
    pop af ; sign of x
    jp m,@y_zero_x_neg
; y zero,x pos
; angle is 90,nothing to do
    ret
@y_zero_x_neg:
; y zero ,x neg
; angle is 270
    ld hl,0x00C000
    ret
@zero_hlu:
    xor a
    ld (@scratch),hl
    ld (@scratch+2),a
    ld hl,(@scratch)
    ret
@scratch: ds 6

; inputs: ub.c and ud.e are dx and dy in 16.8 fixed format
; output: uh.l is the 16.8 fixed format angle
; destroys: a,hl,bc,de
; the following note was written by github copilot:
; note: this routine is a bit of a hack
;      but it works
;      and it's fast
;      and it's small
;      and it's accurate
;      and it's easy to understand
;      and it's easy to modify
;      and it's easy to use
;      and it's easy to remember
;      and it's easy to love
;      and it's easy to hate
;      and it's easy to ignore
;      and it's easy to forget
;      and it's easy to remember
;      and it's easy to forget
;      and it's easy to remember
;      (ok the bot is stuck in a loop)
; REAL NOTE: only works for angles from 0 to 45 degrees
;   use atan2_168 (which calls this proc) to handle the full 360 degrees
atan_168game:
; because we use compass headings instead of geometric angles
; we compute dx/dy which is 1/tan(theta) in the maths world
; we can do faster unsigned division here because we know dx and dy are positive
	call udiv168 ; uh.l = dx/dy	
; ; TODO: IMPLEMENT THIS, RIGHT NOW IS IS BUGGED
; ; test uh.l for 0
;     add hl,de
;     or a
;     sbc hl,de 
;     jr z,@is_zero
; ; test uh.l for 1
;     xor a ; clear carry
;     ex de,hl
;     ld hl,0x000100 ; 1 in 16.8 fixed format
;     sbc hl,de
;     jr z,@is_45
; ; END TODO

; no special cases so we move on
; l contains the fractional portion of tan(uh.l)
; we multiply it by three to get our lookup table index
    ld h,3
    mlt hl ; gosh that is handy
    ld de,0 ; clear deu
    ld d,h ; copy hl to de
    ld e,l ; de contains our index
    ld hl,atan_lut_168 ; grab the lut address
    add hl,de ; bump hl by the index
    ld hl,(hl) ; don't try this on a z80!
    ret ; and out
@is_45:
    ld hl,0x002000 ; 45 degrees decimal
    ret
; for the case tan(0)
@is_zero:
    ld hl,0x000000
    ret

sin_lut_168:
	dl 0x000000 ; 0.000 00, 0.000
	dl 0x000006 ; 1.406 01, 0.025
	dl 0x00000C ; 2.813 02, 0.049
	dl 0x000012 ; 4.219 03, 0.074
	dl 0x000019 ; 5.625 04, 0.098
	dl 0x00001F ; 7.031 05, 0.122
	dl 0x000025 ; 8.438 06, 0.147
	dl 0x00002B ; 9.844 07, 0.171
	dl 0x000031 ; 11.250 08, 0.195
	dl 0x000038 ; 12.656 09, 0.219
	dl 0x00003E ; 14.063 0A, 0.243
	dl 0x000044 ; 15.469 0B, 0.267
	dl 0x00004A ; 16.875 0C, 0.290
	dl 0x000050 ; 18.281 0D, 0.314
	dl 0x000056 ; 19.688 0E, 0.337
	dl 0x00005C ; 21.094 0F, 0.360
	dl 0x000061 ; 22.500 10, 0.383
	dl 0x000067 ; 23.906 11, 0.405
	dl 0x00006D ; 25.313 12, 0.428
	dl 0x000073 ; 26.719 13, 0.450
	dl 0x000078 ; 28.125 14, 0.471
	dl 0x00007E ; 29.531 15, 0.493
	dl 0x000083 ; 30.938 16, 0.514
	dl 0x000088 ; 32.344 17, 0.535
	dl 0x00008E ; 33.750 18, 0.556
	dl 0x000093 ; 35.156 19, 0.576
	dl 0x000098 ; 36.563 1A, 0.596
	dl 0x00009D ; 37.969 1B, 0.615
	dl 0x0000A2 ; 39.375 1C, 0.634
	dl 0x0000A7 ; 40.781 1D, 0.653
	dl 0x0000AB ; 42.188 1E, 0.672
	dl 0x0000B0 ; 43.594 1F, 0.690
	dl 0x0000B5 ; 45.000 20, 0.707
	dl 0x0000B9 ; 46.406 21, 0.724
	dl 0x0000BD ; 47.813 22, 0.741
	dl 0x0000C1 ; 49.219 23, 0.757
	dl 0x0000C5 ; 50.625 24, 0.773
	dl 0x0000C9 ; 52.031 25, 0.788
	dl 0x0000CD ; 53.438 26, 0.803
	dl 0x0000D1 ; 54.844 27, 0.818
	dl 0x0000D4 ; 56.250 28, 0.831
	dl 0x0000D8 ; 57.656 29, 0.845
	dl 0x0000DB ; 59.063 2A, 0.858
	dl 0x0000DE ; 60.469 2B, 0.870
	dl 0x0000E1 ; 61.875 2C, 0.882
	dl 0x0000E4 ; 63.281 2D, 0.893
	dl 0x0000E7 ; 64.688 2E, 0.904
	dl 0x0000EA ; 66.094 2F, 0.914
	dl 0x0000EC ; 67.500 30, 0.924
	dl 0x0000EE ; 68.906 31, 0.933
	dl 0x0000F1 ; 70.313 32, 0.942
	dl 0x0000F3 ; 71.719 33, 0.950
	dl 0x0000F4 ; 73.125 34, 0.957
	dl 0x0000F6 ; 74.531 35, 0.964
	dl 0x0000F8 ; 75.938 36, 0.970
	dl 0x0000F9 ; 77.344 37, 0.976
	dl 0x0000FB ; 78.750 38, 0.981
	dl 0x0000FC ; 80.156 39, 0.985
	dl 0x0000FD ; 81.563 3A, 0.989
	dl 0x0000FE ; 82.969 3B, 0.992
	dl 0x0000FE ; 84.375 3C, 0.995
	dl 0x0000FF ; 85.781 3D, 0.997
	dl 0x0000FF ; 87.188 3E, 0.999
	dl 0x0000FF ; 88.594 3F, 1.000
	dl 0x000100 ; 90.000 40, 1.000
	dl 0x0000FF ; 91.406 41, 1.000
	dl 0x0000FF ; 92.813 42, 0.999
	dl 0x0000FF ; 94.219 43, 0.997
	dl 0x0000FE ; 95.625 44, 0.995
	dl 0x0000FE ; 97.031 45, 0.992
	dl 0x0000FD ; 98.438 46, 0.989
	dl 0x0000FC ; 99.844 47, 0.985
	dl 0x0000FB ; 101.250 48, 0.981
	dl 0x0000F9 ; 102.656 49, 0.976
	dl 0x0000F8 ; 104.063 4A, 0.970
	dl 0x0000F6 ; 105.469 4B, 0.964
	dl 0x0000F4 ; 106.875 4C, 0.957
	dl 0x0000F3 ; 108.281 4D, 0.950
	dl 0x0000F1 ; 109.688 4E, 0.942
	dl 0x0000EE ; 111.094 4F, 0.933
	dl 0x0000EC ; 112.500 50, 0.924
	dl 0x0000EA ; 113.906 51, 0.914
	dl 0x0000E7 ; 115.313 52, 0.904
	dl 0x0000E4 ; 116.719 53, 0.893
	dl 0x0000E1 ; 118.125 54, 0.882
	dl 0x0000DE ; 119.531 55, 0.870
	dl 0x0000DB ; 120.938 56, 0.858
	dl 0x0000D8 ; 122.344 57, 0.845
	dl 0x0000D4 ; 123.750 58, 0.831
	dl 0x0000D1 ; 125.156 59, 0.818
	dl 0x0000CD ; 126.563 5A, 0.803
	dl 0x0000C9 ; 127.969 5B, 0.788
	dl 0x0000C5 ; 129.375 5C, 0.773
	dl 0x0000C1 ; 130.781 5D, 0.757
	dl 0x0000BD ; 132.188 5E, 0.741
	dl 0x0000B9 ; 133.594 5F, 0.724
	dl 0x0000B5 ; 135.000 60, 0.707
	dl 0x0000B0 ; 136.406 61, 0.690
	dl 0x0000AB ; 137.813 62, 0.672
	dl 0x0000A7 ; 139.219 63, 0.653
	dl 0x0000A2 ; 140.625 64, 0.634
	dl 0x00009D ; 142.031 65, 0.615
	dl 0x000098 ; 143.438 66, 0.596
	dl 0x000093 ; 144.844 67, 0.576
	dl 0x00008E ; 146.250 68, 0.556
	dl 0x000088 ; 147.656 69, 0.535
	dl 0x000083 ; 149.063 6A, 0.514
	dl 0x00007E ; 150.469 6B, 0.493
	dl 0x000078 ; 151.875 6C, 0.471
	dl 0x000073 ; 153.281 6D, 0.450
	dl 0x00006D ; 154.688 6E, 0.428
	dl 0x000067 ; 156.094 6F, 0.405
	dl 0x000061 ; 157.500 70, 0.383
	dl 0x00005C ; 158.906 71, 0.360
	dl 0x000056 ; 160.313 72, 0.337
	dl 0x000050 ; 161.719 73, 0.314
	dl 0x00004A ; 163.125 74, 0.290
	dl 0x000044 ; 164.531 75, 0.267
	dl 0x00003E ; 165.938 76, 0.243
	dl 0x000038 ; 167.344 77, 0.219
	dl 0x000031 ; 168.750 78, 0.195
	dl 0x00002B ; 170.156 79, 0.171
	dl 0x000025 ; 171.563 7A, 0.147
	dl 0x00001F ; 172.969 7B, 0.122
	dl 0x000019 ; 174.375 7C, 0.098
	dl 0x000012 ; 175.781 7D, 0.074
	dl 0x00000C ; 177.188 7E, 0.049
	dl 0x000006 ; 178.594 7F, 0.025
	dl 0x000000 ; 180.000 80, 0.000
	dl 0xFFFFFA ; 181.406 81, -0.025
	dl 0xFFFFF4 ; 182.813 82, -0.049
	dl 0xFFFFEE ; 184.219 83, -0.074
	dl 0xFFFFE7 ; 185.625 84, -0.098
	dl 0xFFFFE1 ; 187.031 85, -0.122
	dl 0xFFFFDB ; 188.438 86, -0.147
	dl 0xFFFFD5 ; 189.844 87, -0.171
	dl 0xFFFFCF ; 191.250 88, -0.195
	dl 0xFFFFC8 ; 192.656 89, -0.219
	dl 0xFFFFC2 ; 194.063 8A, -0.243
	dl 0xFFFFBC ; 195.469 8B, -0.267
	dl 0xFFFFB6 ; 196.875 8C, -0.290
	dl 0xFFFFB0 ; 198.281 8D, -0.314
	dl 0xFFFFAA ; 199.688 8E, -0.337
	dl 0xFFFFA4 ; 201.094 8F, -0.360
	dl 0xFFFF9F ; 202.500 90, -0.383
	dl 0xFFFF99 ; 203.906 91, -0.405
	dl 0xFFFF93 ; 205.313 92, -0.428
	dl 0xFFFF8D ; 206.719 93, -0.450
	dl 0xFFFF88 ; 208.125 94, -0.471
	dl 0xFFFF82 ; 209.531 95, -0.493
	dl 0xFFFF7D ; 210.938 96, -0.514
	dl 0xFFFF78 ; 212.344 97, -0.535
	dl 0xFFFF72 ; 213.750 98, -0.556
	dl 0xFFFF6D ; 215.156 99, -0.576
	dl 0xFFFF68 ; 216.563 9A, -0.596
	dl 0xFFFF63 ; 217.969 9B, -0.615
	dl 0xFFFF5E ; 219.375 9C, -0.634
	dl 0xFFFF59 ; 220.781 9D, -0.653
	dl 0xFFFF55 ; 222.188 9E, -0.672
	dl 0xFFFF50 ; 223.594 9F, -0.690
	dl 0xFFFF4B ; 225.000 A0, -0.707
	dl 0xFFFF47 ; 226.406 A1, -0.724
	dl 0xFFFF43 ; 227.813 A2, -0.741
	dl 0xFFFF3F ; 229.219 A3, -0.757
	dl 0xFFFF3B ; 230.625 A4, -0.773
	dl 0xFFFF37 ; 232.031 A5, -0.788
	dl 0xFFFF33 ; 233.438 A6, -0.803
	dl 0xFFFF2F ; 234.844 A7, -0.818
	dl 0xFFFF2C ; 236.250 A8, -0.831
	dl 0xFFFF28 ; 237.656 A9, -0.845
	dl 0xFFFF25 ; 239.063 AA, -0.858
	dl 0xFFFF22 ; 240.469 AB, -0.870
	dl 0xFFFF1F ; 241.875 AC, -0.882
	dl 0xFFFF1C ; 243.281 AD, -0.893
	dl 0xFFFF19 ; 244.688 AE, -0.904
	dl 0xFFFF16 ; 246.094 AF, -0.914
	dl 0xFFFF14 ; 247.500 B0, -0.924
	dl 0xFFFF12 ; 248.906 B1, -0.933
	dl 0xFFFF0F ; 250.313 B2, -0.942
	dl 0xFFFF0D ; 251.719 B3, -0.950
	dl 0xFFFF0C ; 253.125 B4, -0.957
	dl 0xFFFF0A ; 254.531 B5, -0.964
	dl 0xFFFF08 ; 255.938 B6, -0.970
	dl 0xFFFF07 ; 257.344 B7, -0.976
	dl 0xFFFF05 ; 258.750 B8, -0.981
	dl 0xFFFF04 ; 260.156 B9, -0.985
	dl 0xFFFF03 ; 261.563 BA, -0.989
	dl 0xFFFF02 ; 262.969 BB, -0.992
	dl 0xFFFF02 ; 264.375 BC, -0.995
	dl 0xFFFF01 ; 265.781 BD, -0.997
	dl 0xFFFF01 ; 267.188 BE, -0.999
	dl 0xFFFF01 ; 268.594 BF, -1.000
	dl 0xFFFF00 ; 270.000 C0, -1.000
	dl 0xFFFF01 ; 271.406 C1, -1.000
	dl 0xFFFF01 ; 272.813 C2, -0.999
	dl 0xFFFF01 ; 274.219 C3, -0.997
	dl 0xFFFF02 ; 275.625 C4, -0.995
	dl 0xFFFF02 ; 277.031 C5, -0.992
	dl 0xFFFF03 ; 278.438 C6, -0.989
	dl 0xFFFF04 ; 279.844 C7, -0.985
	dl 0xFFFF05 ; 281.250 C8, -0.981
	dl 0xFFFF07 ; 282.656 C9, -0.976
	dl 0xFFFF08 ; 284.063 CA, -0.970
	dl 0xFFFF0A ; 285.469 CB, -0.964
	dl 0xFFFF0C ; 286.875 CC, -0.957
	dl 0xFFFF0D ; 288.281 CD, -0.950
	dl 0xFFFF0F ; 289.688 CE, -0.942
	dl 0xFFFF12 ; 291.094 CF, -0.933
	dl 0xFFFF14 ; 292.500 D0, -0.924
	dl 0xFFFF16 ; 293.906 D1, -0.914
	dl 0xFFFF19 ; 295.313 D2, -0.904
	dl 0xFFFF1C ; 296.719 D3, -0.893
	dl 0xFFFF1F ; 298.125 D4, -0.882
	dl 0xFFFF22 ; 299.531 D5, -0.870
	dl 0xFFFF25 ; 300.938 D6, -0.858
	dl 0xFFFF28 ; 302.344 D7, -0.845
	dl 0xFFFF2C ; 303.750 D8, -0.831
	dl 0xFFFF2F ; 305.156 D9, -0.818
	dl 0xFFFF33 ; 306.563 DA, -0.803
	dl 0xFFFF37 ; 307.969 DB, -0.788
	dl 0xFFFF3B ; 309.375 DC, -0.773
	dl 0xFFFF3F ; 310.781 DD, -0.757
	dl 0xFFFF43 ; 312.188 DE, -0.741
	dl 0xFFFF47 ; 313.594 DF, -0.724
	dl 0xFFFF4B ; 315.000 E0, -0.707
	dl 0xFFFF50 ; 316.406 E1, -0.690
	dl 0xFFFF55 ; 317.813 E2, -0.672
	dl 0xFFFF59 ; 319.219 E3, -0.653
	dl 0xFFFF5E ; 320.625 E4, -0.634
	dl 0xFFFF63 ; 322.031 E5, -0.615
	dl 0xFFFF68 ; 323.438 E6, -0.596
	dl 0xFFFF6D ; 324.844 E7, -0.576
	dl 0xFFFF72 ; 326.250 E8, -0.556
	dl 0xFFFF78 ; 327.656 E9, -0.535
	dl 0xFFFF7D ; 329.063 EA, -0.514
	dl 0xFFFF82 ; 330.469 EB, -0.493
	dl 0xFFFF88 ; 331.875 EC, -0.471
	dl 0xFFFF8D ; 333.281 ED, -0.450
	dl 0xFFFF93 ; 334.688 EE, -0.428
	dl 0xFFFF99 ; 336.094 EF, -0.405
	dl 0xFFFF9F ; 337.500 F0, -0.383
	dl 0xFFFFA4 ; 338.906 F1, -0.360
	dl 0xFFFFAA ; 340.313 F2, -0.337
	dl 0xFFFFB0 ; 341.719 F3, -0.314
	dl 0xFFFFB6 ; 343.125 F4, -0.290
	dl 0xFFFFBC ; 344.531 F5, -0.267
	dl 0xFFFFC2 ; 345.938 F6, -0.243
	dl 0xFFFFC8 ; 347.344 F7, -0.219
	dl 0xFFFFCF ; 348.750 F8, -0.195
	dl 0xFFFFD5 ; 350.156 F9, -0.171
	dl 0xFFFFDB ; 351.563 FA, -0.147
	dl 0xFFFFE1 ; 352.969 FB, -0.122
	dl 0xFFFFE7 ; 354.375 FC, -0.098
	dl 0xFFFFEE ; 355.781 FD, -0.074
	dl 0xFFFFF4 ; 357.188 FE, -0.049
	dl 0xFFFFFA ; 358.594 FF, -0.025
	dl 0x000000 ; 0.000 00, 0.000 for interpolation

sin_lut_816:
    dl 0x000000,0x000648 ; 00 000.000 0.000
    dl 0x000648,0x000648 ; 01 001.406 0.025
    dl 0x000C90,0x000645 ; 02 002.813 0.049
    dl 0x0012D5,0x000642 ; 03 004.219 0.074
    dl 0x001917,0x00063E ; 04 005.625 0.098
    dl 0x001F56,0x00063A ; 05 007.031 0.122
    dl 0x002590,0x000633 ; 06 008.438 0.147
    dl 0x002BC4,0x00062C ; 07 009.844 0.171
    dl 0x0031F1,0x000625 ; 08 011.250 0.195
    dl 0x003816,0x00061D ; 09 012.656 0.219
    dl 0x003E34,0x000613 ; 0A 014.063 0.243
    dl 0x004447,0x000608 ; 0B 015.469 0.267
    dl 0x004A50,0x0005FD ; 0C 016.875 0.290
    dl 0x00504D,0x0005F1 ; 0D 018.281 0.314
    dl 0x00563E,0x0005E3 ; 0E 019.688 0.337
    dl 0x005C22,0x0005D5 ; 0F 021.094 0.360
    dl 0x0061F7,0x0005C6 ; 10 022.500 0.383
    dl 0x0067BD,0x0005B7 ; 11 023.906 0.405
    dl 0x006D74,0x0005A5 ; 12 025.313 0.428
    dl 0x007319,0x000593 ; 13 026.719 0.450
    dl 0x0078AD,0x000580 ; 14 028.125 0.471
    dl 0x007E2E,0x00056E ; 15 029.531 0.493
    dl 0x00839C,0x000559 ; 16 030.938 0.514
    dl 0x0088F5,0x000544 ; 17 032.344 0.535
    dl 0x008E39,0x00052E ; 18 033.750 0.556
    dl 0x009367,0x000518 ; 19 035.156 0.576
    dl 0x009880,0x0004FF ; 1A 036.563 0.596
    dl 0x009D80,0x0004E7 ; 1B 037.969 0.615
    dl 0x00A267,0x0004CE ; 1C 039.375 0.634
    dl 0x00A736,0x0004B5 ; 1D 040.781 0.653
    dl 0x00ABEB,0x00049A ; 1E 042.188 0.672
    dl 0x00B085,0x00047F ; 1F 043.594 0.690
    dl 0x00B504,0x000463 ; 20 045.000 0.707
    dl 0x00B968,0x000447 ; 21 046.406 0.724
    dl 0x00BDAF,0x000429 ; 22 047.813 0.741
    dl 0x00C1D8,0x00040B ; 23 049.219 0.757
    dl 0x00C5E4,0x0003EC ; 24 050.625 0.773
    dl 0x00C9D0,0x0003CE ; 25 052.031 0.788
    dl 0x00CD9F,0x0003AE ; 26 053.438 0.803
    dl 0x00D14D,0x00038D ; 27 054.844 0.818
    dl 0x00D4DB,0x00036C ; 28 056.250 0.831
    dl 0x00D848,0x00034C ; 29 057.656 0.845
    dl 0x00DB94,0x000329 ; 2A 059.063 0.858
    dl 0x00DEBE,0x000307 ; 2B 060.469 0.870
    dl 0x00E1C5,0x0002E4 ; 2C 061.875 0.882
    dl 0x00E4AA,0x0002C1 ; 2D 063.281 0.893
    dl 0x00E76C,0x00029D ; 2E 064.688 0.904
    dl 0x00EA09,0x000279 ; 2F 066.094 0.914
    dl 0x00EC83,0x000255 ; 30 067.500 0.924
    dl 0x00EED8,0x000230 ; 31 068.906 0.933
    dl 0x00F109,0x00020B ; 32 070.313 0.942
    dl 0x00F314,0x0001E5 ; 33 071.719 0.950
    dl 0x00F4FA,0x0001BF ; 34 073.125 0.957
    dl 0x00F6B9,0x00019A ; 35 074.531 0.964
    dl 0x00F854,0x000173 ; 36 075.938 0.970
    dl 0x00F9C7,0x00014D ; 37 077.344 0.976
    dl 0x00FB14,0x000126 ; 38 078.750 0.981
    dl 0x00FC3B,0x0000FF ; 39 080.156 0.985
    dl 0x00FD3A,0x0000D8 ; 3A 081.563 0.989
    dl 0x00FE13,0x0000B1 ; 3B 082.969 0.992
    dl 0x00FEC4,0x000089 ; 3C 084.375 0.995
    dl 0x00FF4E,0x000062 ; 3D 085.781 0.997
    dl 0x00FFB1,0x00003B ; 3E 087.188 0.999
    dl 0x00FFEC,0x000013 ; 3F 088.594 1.000
    dl 0x010000,0xFFFFEC ; 40 090.000 1.000
    dl 0x00FFEC,0xFFFFC4 ; 41 091.406 1.000
    dl 0x00FFB1,0xFFFF9D ; 42 092.813 0.999
    dl 0x00FF4E,0xFFFF76 ; 43 094.219 0.997
    dl 0x00FEC4,0xFFFF4E ; 44 095.625 0.995
    dl 0x00FE13,0xFFFF27 ; 45 097.031 0.992
    dl 0x00FD3A,0xFFFF00 ; 46 098.438 0.989
    dl 0x00FC3B,0xFFFED9 ; 47 099.844 0.985
    dl 0x00FB14,0xFFFEB2 ; 48 101.250 0.981
    dl 0x00F9C7,0xFFFE8C ; 49 102.656 0.976
    dl 0x00F853,0xFFFE66 ; 4A 104.063 0.970
    dl 0x00F6B9,0xFFFE40 ; 4B 105.469 0.964
    dl 0x00F4FA,0xFFFE1A ; 4C 106.875 0.957
    dl 0x00F314,0xFFFDF4 ; 4D 108.281 0.950
    dl 0x00F108,0xFFFDCF ; 4E 109.688 0.942
    dl 0x00EED8,0xFFFDAA ; 4F 111.094 0.933
    dl 0x00EC83,0xFFFD86 ; 50 112.500 0.924
    dl 0x00EA09,0xFFFD61 ; 51 113.906 0.914
    dl 0x00E76B,0xFFFD3E ; 52 115.313 0.904
    dl 0x00E4AA,0xFFFD1B ; 53 116.719 0.893
    dl 0x00E1C5,0xFFFCF8 ; 54 118.125 0.882
    dl 0x00DEBE,0xFFFCD5 ; 55 119.531 0.870
    dl 0x00DB93,0xFFFCB4 ; 56 120.938 0.858
    dl 0x00D848,0xFFFC93 ; 57 122.344 0.845
    dl 0x00D4DB,0xFFFC72 ; 58 123.750 0.831
    dl 0x00D14D,0xFFFC51 ; 59 125.156 0.818
    dl 0x00CD9E,0xFFFC32 ; 5A 126.563 0.803
    dl 0x00C9D0,0xFFFC13 ; 5B 127.969 0.788
    dl 0x00C5E4,0xFFFBF4 ; 5C 129.375 0.773
    dl 0x00C1D8,0xFFFBD5 ; 5D 130.781 0.757
    dl 0x00BDAE,0xFFFBB9 ; 5E 132.188 0.741
    dl 0x00B968,0xFFFB9C ; 5F 133.594 0.724
    dl 0x00B504,0xFFFB80 ; 60 135.000 0.707
    dl 0x00B085,0xFFFB64 ; 61 136.406 0.690
    dl 0x00ABEA,0xFFFB4B ; 62 137.813 0.672
    dl 0x00A736,0xFFFB31 ; 63 139.219 0.653
    dl 0x00A267,0xFFFB18 ; 64 140.625 0.634
    dl 0x009D80,0xFFFAFF ; 65 142.031 0.615
    dl 0x00987F,0xFFFAE8 ; 66 143.438 0.596
    dl 0x009367,0xFFFAD1 ; 67 144.844 0.576
    dl 0x008E39,0xFFFABB ; 68 146.250 0.556
    dl 0x0088F5,0xFFFAA5 ; 69 147.656 0.535
    dl 0x00839B,0xFFFA92 ; 6A 149.063 0.514
    dl 0x007E2E,0xFFFA7F ; 6B 150.469 0.493
    dl 0x0078AD,0xFFFA6C ; 6C 151.875 0.471
    dl 0x007319,0xFFFA59 ; 6D 153.281 0.450
    dl 0x006D73,0xFFFA49 ; 6E 154.688 0.428
    dl 0x0067BD,0xFFFA39 ; 6F 156.094 0.405
    dl 0x0061F7,0xFFFA2A ; 70 157.500 0.383
    dl 0x005C22,0xFFFA1B ; 71 158.906 0.360
    dl 0x00563D,0xFFFA0F ; 72 160.313 0.337
    dl 0x00504D,0xFFFA02 ; 73 161.719 0.314
    dl 0x004A50,0xFFF9F7 ; 74 163.125 0.290
    dl 0x004447,0xFFF9EB ; 75 164.531 0.267
    dl 0x003E33,0xFFF9E3 ; 76 165.938 0.243
    dl 0x003816,0xFFF9DA ; 77 167.344 0.219
    dl 0x0031F1,0xFFF9D3 ; 78 168.750 0.195
    dl 0x002BC4,0xFFF9CB ; 79 170.156 0.171
    dl 0x00258F,0xFFF9C6 ; 7A 171.563 0.147
    dl 0x001F56,0xFFF9C1 ; 7B 172.969 0.122
    dl 0x001917,0xFFF9BD ; 7C 174.375 0.098
    dl 0x0012D5,0xFFF9B9 ; 7D 175.781 0.074
    dl 0x000C8F,0xFFF9B8 ; 7E 177.188 0.049
    dl 0x000648,0xFFF9B7 ; 7F 178.594 0.025
    dl 0x000000,0xFFF9B7 ; 80 180.000 0.000
    dl 0xFFF9B8,0xFFF9B7 ; 81 181.406 -0.025
    dl 0xFFF370,0xFFF9BA ; 82 182.813 -0.049
    dl 0xFFED2B,0xFFF9BD ; 83 184.219 -0.074
    dl 0xFFE6E9,0xFFF9C1 ; 84 185.625 -0.098
    dl 0xFFE0AA,0xFFF9C5 ; 85 187.031 -0.122
    dl 0xFFDA70,0xFFF9CC ; 86 188.438 -0.147
    dl 0xFFD43C,0xFFF9D3 ; 87 189.844 -0.171
    dl 0xFFCE0F,0xFFF9DA ; 88 191.250 -0.195
    dl 0xFFC7EA,0xFFF9E2 ; 89 192.656 -0.219
    dl 0xFFC1CC,0xFFF9EC ; 8A 194.063 -0.243
    dl 0xFFBBB9,0xFFF9F7 ; 8B 195.469 -0.267
    dl 0xFFB5B0,0xFFFA02 ; 8C 196.875 -0.290
    dl 0xFFAFB3,0xFFFA0E ; 8D 198.281 -0.314
    dl 0xFFA9C2,0xFFFA1C ; 8E 199.688 -0.337
    dl 0xFFA3DE,0xFFFA2A ; 8F 201.094 -0.360
    dl 0xFF9E09,0xFFFA39 ; 90 202.500 -0.383
    dl 0xFF9843,0xFFFA48 ; 91 203.906 -0.405
    dl 0xFF928C,0xFFFA5A ; 92 205.313 -0.428
    dl 0xFF8CE7,0xFFFA6C ; 93 206.719 -0.450
    dl 0xFF8753,0xFFFA7F ; 94 208.125 -0.471
    dl 0xFF81D2,0xFFFA91 ; 95 209.531 -0.493
    dl 0xFF7C64,0xFFFAA6 ; 96 210.938 -0.514
    dl 0xFF770B,0xFFFABB ; 97 212.344 -0.535
    dl 0xFF71C7,0xFFFAD1 ; 98 213.750 -0.556
    dl 0xFF6C99,0xFFFAE7 ; 99 215.156 -0.576
    dl 0xFF6780,0xFFFB00 ; 9A 216.563 -0.596
    dl 0xFF6280,0xFFFB18 ; 9B 217.969 -0.615
    dl 0xFF5D99,0xFFFB31 ; 9C 219.375 -0.634
    dl 0xFF58CA,0xFFFB4A ; 9D 220.781 -0.653
    dl 0xFF5415,0xFFFB65 ; 9E 222.188 -0.672
    dl 0xFF4F7B,0xFFFB80 ; 9F 223.594 -0.690
    dl 0xFF4AFC,0xFFFB9C ; A0 225.000 -0.707
    dl 0xFF4698,0xFFFBB8 ; A1 226.406 -0.724
    dl 0xFF4251,0xFFFBD6 ; A2 227.813 -0.741
    dl 0xFF3E28,0xFFFBF4 ; A3 229.219 -0.757
    dl 0xFF3A1C,0xFFFC13 ; A4 230.625 -0.773
    dl 0xFF3630,0xFFFC31 ; A5 232.031 -0.788
    dl 0xFF3261,0xFFFC51 ; A6 233.438 -0.803
    dl 0xFF2EB3,0xFFFC72 ; A7 234.844 -0.818
    dl 0xFF2B25,0xFFFC93 ; A8 236.250 -0.831
    dl 0xFF27B8,0xFFFCB3 ; A9 237.656 -0.845
    dl 0xFF246C,0xFFFCD6 ; AA 239.063 -0.858
    dl 0xFF2142,0xFFFCF8 ; AB 240.469 -0.870
    dl 0xFF1E3B,0xFFFD1B ; AC 241.875 -0.882
    dl 0xFF1B56,0xFFFD3E ; AD 243.281 -0.893
    dl 0xFF1894,0xFFFD62 ; AE 244.688 -0.904
    dl 0xFF15F7,0xFFFD86 ; AF 246.094 -0.914
    dl 0xFF137D,0xFFFDAA ; B0 247.500 -0.924
    dl 0xFF1128,0xFFFDCF ; B1 248.906 -0.933
    dl 0xFF0EF7,0xFFFDF4 ; B2 250.313 -0.942
    dl 0xFF0CEC,0xFFFE1A ; B3 251.719 -0.950
    dl 0xFF0B06,0xFFFE40 ; B4 253.125 -0.957
    dl 0xFF0947,0xFFFE65 ; B5 254.531 -0.964
    dl 0xFF07AC,0xFFFE8C ; B6 255.938 -0.970
    dl 0xFF0639,0xFFFEB2 ; B7 257.344 -0.976
    dl 0xFF04EC,0xFFFED9 ; B8 258.750 -0.981
    dl 0xFF03C5,0xFFFF00 ; B9 260.156 -0.985
    dl 0xFF02C6,0xFFFF27 ; BA 261.563 -0.989
    dl 0xFF01ED,0xFFFF4E ; BB 262.969 -0.992
    dl 0xFF013C,0xFFFF76 ; BC 264.375 -0.995
    dl 0xFF00B2,0xFFFF9D ; BD 265.781 -0.997
    dl 0xFF004F,0xFFFFC4 ; BE 267.188 -0.999
    dl 0xFF0014,0xFFFFEC ; BF 268.594 -1.000
    dl 0xFF0000,0x000013 ; C0 270.000 -1.000
    dl 0xFF0014,0x00003B ; C1 271.406 -1.000
    dl 0xFF004F,0x000062 ; C2 272.813 -0.999
    dl 0xFF00B2,0x000089 ; C3 274.219 -0.997
    dl 0xFF013C,0x0000B1 ; C4 275.625 -0.995
    dl 0xFF01ED,0x0000D8 ; C5 277.031 -0.992
    dl 0xFF02C6,0x0000FF ; C6 278.438 -0.989
    dl 0xFF03C5,0x000126 ; C7 279.844 -0.985
    dl 0xFF04EC,0x00014D ; C8 281.250 -0.981
    dl 0xFF0639,0x000173 ; C9 282.656 -0.976
    dl 0xFF07AD,0x000199 ; CA 284.063 -0.970
    dl 0xFF0947,0x0001BF ; CB 285.469 -0.964
    dl 0xFF0B06,0x0001E5 ; CC 286.875 -0.957
    dl 0xFF0CEC,0x00020B ; CD 288.281 -0.950
    dl 0xFF0EF8,0x000230 ; CE 289.688 -0.942
    dl 0xFF1128,0x000255 ; CF 291.094 -0.933
    dl 0xFF137D,0x000279 ; D0 292.500 -0.924
    dl 0xFF15F7,0x00029E ; D1 293.906 -0.914
    dl 0xFF1895,0x0002C1 ; D2 295.313 -0.904
    dl 0xFF1B56,0x0002E4 ; D3 296.719 -0.893
    dl 0xFF1E3B,0x000307 ; D4 298.125 -0.882
    dl 0xFF2142,0x00032A ; D5 299.531 -0.870
    dl 0xFF246D,0x00034B ; D6 300.938 -0.858
    dl 0xFF27B8,0x00036C ; D7 302.344 -0.845
    dl 0xFF2B25,0x00038D ; D8 303.750 -0.831
    dl 0xFF2EB3,0x0003AE ; D9 305.156 -0.818
    dl 0xFF3262,0x0003CD ; DA 306.563 -0.803
    dl 0xFF3630,0x0003EC ; DB 307.969 -0.788
    dl 0xFF3A1C,0x00040B ; DC 309.375 -0.773
    dl 0xFF3E28,0x00042A ; DD 310.781 -0.757
    dl 0xFF4252,0x000446 ; DE 312.188 -0.741
    dl 0xFF4698,0x000463 ; DF 313.594 -0.724
    dl 0xFF4AFC,0x00047F ; E0 315.000 -0.707
    dl 0xFF4F7B,0x00049B ; E1 316.406 -0.690
    dl 0xFF5416,0x0004B4 ; E2 317.813 -0.672
    dl 0xFF58CA,0x0004CE ; E3 319.219 -0.653
    dl 0xFF5D99,0x0004E7 ; E4 320.625 -0.634
    dl 0xFF6280,0x000500 ; E5 322.031 -0.615
    dl 0xFF6781,0x000517 ; E6 323.438 -0.596
    dl 0xFF6C99,0x00052E ; E7 324.844 -0.576
    dl 0xFF71C7,0x000544 ; E8 326.250 -0.556
    dl 0xFF770B,0x00055A ; E9 327.656 -0.535
    dl 0xFF7C65,0x00056D ; EA 329.063 -0.514
    dl 0xFF81D2,0x000580 ; EB 330.469 -0.493
    dl 0xFF8753,0x000593 ; EC 331.875 -0.471
    dl 0xFF8CE7,0x0005A6 ; ED 333.281 -0.450
    dl 0xFF928D,0x0005B6 ; EE 334.688 -0.428
    dl 0xFF9843,0x0005C6 ; EF 336.094 -0.405
    dl 0xFF9E09,0x0005D5 ; F0 337.500 -0.383
    dl 0xFFA3DE,0x0005E4 ; F1 338.906 -0.360
    dl 0xFFA9C3,0x0005F0 ; F2 340.313 -0.337
    dl 0xFFAFB3,0x0005FD ; F3 341.719 -0.314
    dl 0xFFB5B0,0x000608 ; F4 343.125 -0.290
    dl 0xFFBBB9,0x000614 ; F5 344.531 -0.267
    dl 0xFFC1CD,0x00061C ; F6 345.938 -0.243
    dl 0xFFC7EA,0x000625 ; F7 347.344 -0.219
    dl 0xFFCE0F,0x00062C ; F8 348.750 -0.195
    dl 0xFFD43C,0x000634 ; F9 350.156 -0.171
    dl 0xFFDA71,0x000639 ; FA 351.563 -0.147
    dl 0xFFE0AA,0x00063E ; FB 352.969 -0.122
    dl 0xFFE6E9,0x000642 ; FC 354.375 -0.098
    dl 0xFFED2B,0x000646 ; FD 355.781 -0.074
    dl 0xFFF371,0x000647 ; FE 357.188 -0.049
    dl 0xFFF9B8,0x000648 ; FF 358.594 -0.025
    dl 0x000000,0x000648 ; 00 360.000 0.000

sin_lut_1212:
    dl 0x000000 ; 00, 00.000, +0.000000
    dl 0x000065 ; 01, 01.406, +0.024541
    dl 0x0000C9 ; 02, 02.812, +0.049068
    dl 0x00012D ; 03, 04.219, +0.073565
    dl 0x000191 ; 04, 05.625, +0.098017
    dl 0x0001F5 ; 05, 07.031, +0.122411
    dl 0x000259 ; 06, 08.438, +0.146730
    dl 0x0002BC ; 07, 09.844, +0.170962
    dl 0x00031F ; 08, 11.250, +0.195090
    dl 0x000381 ; 09, 12.656, +0.219101
    dl 0x0003E3 ; 0A, 14.062, +0.242980
    dl 0x000444 ; 0B, 15.469, +0.266713
    dl 0x0004A5 ; 0C, 16.875, +0.290285
    dl 0x000505 ; 0D, 18.281, +0.313682
    dl 0x000564 ; 0E, 19.688, +0.336890
    dl 0x0005C2 ; 0F, 21.094, +0.359895
    dl 0x00061F ; 10, 22.500, +0.382683
    dl 0x00067C ; 11, 23.906, +0.405241
    dl 0x0006D7 ; 12, 25.312, +0.427555
    dl 0x000732 ; 13, 26.719, +0.449611
    dl 0x00078B ; 14, 28.125, +0.471397
    dl 0x0007E3 ; 15, 29.531, +0.492898
    dl 0x00083A ; 16, 30.938, +0.514103
    dl 0x00088F ; 17, 32.344, +0.534998
    dl 0x0008E4 ; 18, 33.750, +0.555570
    dl 0x000937 ; 19, 35.156, +0.575808
    dl 0x000988 ; 1A, 36.562, +0.595699
    dl 0x0009D8 ; 1B, 37.969, +0.615232
    dl 0x000A26 ; 1C, 39.375, +0.634393
    dl 0x000A73 ; 1D, 40.781, +0.653173
    dl 0x000ABF ; 1E, 42.188, +0.671559
    dl 0x000B08 ; 1F, 43.594, +0.689541
    dl 0x000B50 ; 20, 45.000, +0.707107
    dl 0x000B97 ; 21, 46.406, +0.724247
    dl 0x000BDB ; 22, 47.812, +0.740951
    dl 0x000C1E ; 23, 49.219, +0.757209
    dl 0x000C5E ; 24, 50.625, +0.773010
    dl 0x000C9D ; 25, 52.031, +0.788346
    dl 0x000CDA ; 26, 53.438, +0.803208
    dl 0x000D15 ; 27, 54.844, +0.817585
    dl 0x000D4E ; 28, 56.250, +0.831470
    dl 0x000D85 ; 29, 57.656, +0.844854
    dl 0x000DB9 ; 2A, 59.062, +0.857729
    dl 0x000DEC ; 2B, 60.469, +0.870087
    dl 0x000E1C ; 2C, 61.875, +0.881921
    dl 0x000E4B ; 2D, 63.281, +0.893224
    dl 0x000E77 ; 2E, 64.688, +0.903989
    dl 0x000EA1 ; 2F, 66.094, +0.914210
    dl 0x000EC8 ; 30, 67.500, +0.923880
    dl 0x000EEE ; 31, 68.906, +0.932993
    dl 0x000F11 ; 32, 70.312, +0.941544
    dl 0x000F31 ; 33, 71.719, +0.949528
    dl 0x000F50 ; 34, 73.125, +0.956940
    dl 0x000F6C ; 35, 74.531, +0.963776
    dl 0x000F85 ; 36, 75.938, +0.970031
    dl 0x000F9C ; 37, 77.344, +0.975702
    dl 0x000FB1 ; 38, 78.750, +0.980785
    dl 0x000FC4 ; 39, 80.156, +0.985278
    dl 0x000FD4 ; 3A, 81.562, +0.989177
    dl 0x000FE1 ; 3B, 82.969, +0.992480
    dl 0x000FEC ; 3C, 84.375, +0.995185
    dl 0x000FF5 ; 3D, 85.781, +0.997290
    dl 0x000FFB ; 3E, 87.188, +0.998795
    dl 0x000FFF ; 3F, 88.594, +0.999699
    dl 0x001000 ; 40, 90.000, +1.000000
    dl 0x000FFF ; 41, 91.406, +0.999699
    dl 0x000FFB ; 42, 92.812, +0.998795
    dl 0x000FF5 ; 43, 94.219, +0.997290
    dl 0x000FEC ; 44, 95.625, +0.995185
    dl 0x000FE1 ; 45, 97.031, +0.992480
    dl 0x000FD4 ; 46, 98.438, +0.989177
    dl 0x000FC4 ; 47, 99.844, +0.985278
    dl 0x000FB1 ; 48, 101.250, +0.980785
    dl 0x000F9C ; 49, 102.656, +0.975702
    dl 0x000F85 ; 4A, 104.062, +0.970031
    dl 0x000F6C ; 4B, 105.469, +0.963776
    dl 0x000F50 ; 4C, 106.875, +0.956940
    dl 0x000F31 ; 4D, 108.281, +0.949528
    dl 0x000F11 ; 4E, 109.688, +0.941544
    dl 0x000EEE ; 4F, 111.094, +0.932993
    dl 0x000EC8 ; 50, 112.500, +0.923880
    dl 0x000EA1 ; 51, 113.906, +0.914210
    dl 0x000E77 ; 52, 115.312, +0.903989
    dl 0x000E4B ; 53, 116.719, +0.893224
    dl 0x000E1C ; 54, 118.125, +0.881921
    dl 0x000DEC ; 55, 119.531, +0.870087
    dl 0x000DB9 ; 56, 120.938, +0.857729
    dl 0x000D85 ; 57, 122.344, +0.844854
    dl 0x000D4E ; 58, 123.750, +0.831470
    dl 0x000D15 ; 59, 125.156, +0.817585
    dl 0x000CDA ; 5A, 126.562, +0.803208
    dl 0x000C9D ; 5B, 127.969, +0.788346
    dl 0x000C5E ; 5C, 129.375, +0.773010
    dl 0x000C1E ; 5D, 130.781, +0.757209
    dl 0x000BDB ; 5E, 132.188, +0.740951
    dl 0x000B97 ; 5F, 133.594, +0.724247
    dl 0x000B50 ; 60, 135.000, +0.707107
    dl 0x000B08 ; 61, 136.406, +0.689541
    dl 0x000ABF ; 62, 137.812, +0.671559
    dl 0x000A73 ; 63, 139.219, +0.653173
    dl 0x000A26 ; 64, 140.625, +0.634393
    dl 0x0009D8 ; 65, 142.031, +0.615232
    dl 0x000988 ; 66, 143.438, +0.595699
    dl 0x000937 ; 67, 144.844, +0.575808
    dl 0x0008E4 ; 68, 146.250, +0.555570
    dl 0x00088F ; 69, 147.656, +0.534998
    dl 0x00083A ; 6A, 149.062, +0.514103
    dl 0x0007E3 ; 6B, 150.469, +0.492898
    dl 0x00078B ; 6C, 151.875, +0.471397
    dl 0x000732 ; 6D, 153.281, +0.449611
    dl 0x0006D7 ; 6E, 154.688, +0.427555
    dl 0x00067C ; 6F, 156.094, +0.405241
    dl 0x00061F ; 70, 157.500, +0.382683
    dl 0x0005C2 ; 71, 158.906, +0.359895
    dl 0x000564 ; 72, 160.312, +0.336890
    dl 0x000505 ; 73, 161.719, +0.313682
    dl 0x0004A5 ; 74, 163.125, +0.290285
    dl 0x000444 ; 75, 164.531, +0.266713
    dl 0x0003E3 ; 76, 165.938, +0.242980
    dl 0x000381 ; 77, 167.344, +0.219101
    dl 0x00031F ; 78, 168.750, +0.195090
    dl 0x0002BC ; 79, 170.156, +0.170962
    dl 0x000259 ; 7A, 171.562, +0.146730
    dl 0x0001F5 ; 7B, 172.969, +0.122411
    dl 0x000191 ; 7C, 174.375, +0.098017
    dl 0x00012D ; 7D, 175.781, +0.073565
    dl 0x0000C9 ; 7E, 177.188, +0.049068
    dl 0x000065 ; 7F, 178.594, +0.024541
    dl 0x000000 ; 80, 180.000, +0.000000
    dl 0xFFFF9B ; 81, 181.406, -0.024541
    dl 0xFFFF37 ; 82, 182.812, -0.049068
    dl 0xFFFED3 ; 83, 184.219, -0.073565
    dl 0xFFFE6F ; 84, 185.625, -0.098017
    dl 0xFFFE0B ; 85, 187.031, -0.122411
    dl 0xFFFDA7 ; 86, 188.438, -0.146730
    dl 0xFFFD44 ; 87, 189.844, -0.170962
    dl 0xFFFCE1 ; 88, 191.250, -0.195090
    dl 0xFFFC7F ; 89, 192.656, -0.219101
    dl 0xFFFC1D ; 8A, 194.062, -0.242980
    dl 0xFFFBBC ; 8B, 195.469, -0.266713
    dl 0xFFFB5B ; 8C, 196.875, -0.290285
    dl 0xFFFAFB ; 8D, 198.281, -0.313682
    dl 0xFFFA9C ; 8E, 199.688, -0.336890
    dl 0xFFFA3E ; 8F, 201.094, -0.359895
    dl 0xFFF9E1 ; 90, 202.500, -0.382683
    dl 0xFFF984 ; 91, 203.906, -0.405241
    dl 0xFFF929 ; 92, 205.312, -0.427555
    dl 0xFFF8CE ; 93, 206.719, -0.449611
    dl 0xFFF875 ; 94, 208.125, -0.471397
    dl 0xFFF81D ; 95, 209.531, -0.492898
    dl 0xFFF7C6 ; 96, 210.938, -0.514103
    dl 0xFFF771 ; 97, 212.344, -0.534998
    dl 0xFFF71C ; 98, 213.750, -0.555570
    dl 0xFFF6C9 ; 99, 215.156, -0.575808
    dl 0xFFF678 ; 9A, 216.562, -0.595699
    dl 0xFFF628 ; 9B, 217.969, -0.615232
    dl 0xFFF5DA ; 9C, 219.375, -0.634393
    dl 0xFFF58D ; 9D, 220.781, -0.653173
    dl 0xFFF541 ; 9E, 222.188, -0.671559
    dl 0xFFF4F8 ; 9F, 223.594, -0.689541
    dl 0xFFF4B0 ; A0, 225.000, -0.707107
    dl 0xFFF469 ; A1, 226.406, -0.724247
    dl 0xFFF425 ; A2, 227.812, -0.740951
    dl 0xFFF3E2 ; A3, 229.219, -0.757209
    dl 0xFFF3A2 ; A4, 230.625, -0.773010
    dl 0xFFF363 ; A5, 232.031, -0.788346
    dl 0xFFF326 ; A6, 233.438, -0.803208
    dl 0xFFF2EB ; A7, 234.844, -0.817585
    dl 0xFFF2B2 ; A8, 236.250, -0.831470
    dl 0xFFF27B ; A9, 237.656, -0.844854
    dl 0xFFF247 ; AA, 239.062, -0.857729
    dl 0xFFF214 ; AB, 240.469, -0.870087
    dl 0xFFF1E4 ; AC, 241.875, -0.881921
    dl 0xFFF1B5 ; AD, 243.281, -0.893224
    dl 0xFFF189 ; AE, 244.688, -0.903989
    dl 0xFFF15F ; AF, 246.094, -0.914210
    dl 0xFFF138 ; B0, 247.500, -0.923880
    dl 0xFFF112 ; B1, 248.906, -0.932993
    dl 0xFFF0EF ; B2, 250.312, -0.941544
    dl 0xFFF0CF ; B3, 251.719, -0.949528
    dl 0xFFF0B0 ; B4, 253.125, -0.956940
    dl 0xFFF094 ; B5, 254.531, -0.963776
    dl 0xFFF07B ; B6, 255.938, -0.970031
    dl 0xFFF064 ; B7, 257.344, -0.975702
    dl 0xFFF04F ; B8, 258.750, -0.980785
    dl 0xFFF03C ; B9, 260.156, -0.985278
    dl 0xFFF02C ; BA, 261.562, -0.989177
    dl 0xFFF01F ; BB, 262.969, -0.992480
    dl 0xFFF014 ; BC, 264.375, -0.995185
    dl 0xFFF00B ; BD, 265.781, -0.997290
    dl 0xFFF005 ; BE, 267.188, -0.998795
    dl 0xFFF001 ; BF, 268.594, -0.999699
    dl 0xFFF000 ; C0, 270.000, -1.000000
    dl 0xFFF001 ; C1, 271.406, -0.999699
    dl 0xFFF005 ; C2, 272.812, -0.998795
    dl 0xFFF00B ; C3, 274.219, -0.997290
    dl 0xFFF014 ; C4, 275.625, -0.995185
    dl 0xFFF01F ; C5, 277.031, -0.992480
    dl 0xFFF02C ; C6, 278.438, -0.989177
    dl 0xFFF03C ; C7, 279.844, -0.985278
    dl 0xFFF04F ; C8, 281.250, -0.980785
    dl 0xFFF064 ; C9, 282.656, -0.975702
    dl 0xFFF07B ; CA, 284.062, -0.970031
    dl 0xFFF094 ; CB, 285.469, -0.963776
    dl 0xFFF0B0 ; CC, 286.875, -0.956940
    dl 0xFFF0CF ; CD, 288.281, -0.949528
    dl 0xFFF0EF ; CE, 289.688, -0.941544
    dl 0xFFF112 ; CF, 291.094, -0.932993
    dl 0xFFF138 ; D0, 292.500, -0.923880
    dl 0xFFF15F ; D1, 293.906, -0.914210
    dl 0xFFF189 ; D2, 295.312, -0.903989
    dl 0xFFF1B5 ; D3, 296.719, -0.893224
    dl 0xFFF1E4 ; D4, 298.125, -0.881921
    dl 0xFFF214 ; D5, 299.531, -0.870087
    dl 0xFFF247 ; D6, 300.938, -0.857729
    dl 0xFFF27B ; D7, 302.344, -0.844854
    dl 0xFFF2B2 ; D8, 303.750, -0.831470
    dl 0xFFF2EB ; D9, 305.156, -0.817585
    dl 0xFFF326 ; DA, 306.562, -0.803208
    dl 0xFFF363 ; DB, 307.969, -0.788346
    dl 0xFFF3A2 ; DC, 309.375, -0.773010
    dl 0xFFF3E2 ; DD, 310.781, -0.757209
    dl 0xFFF425 ; DE, 312.188, -0.740951
    dl 0xFFF469 ; DF, 313.594, -0.724247
    dl 0xFFF4B0 ; E0, 315.000, -0.707107
    dl 0xFFF4F8 ; E1, 316.406, -0.689541
    dl 0xFFF541 ; E2, 317.812, -0.671559
    dl 0xFFF58D ; E3, 319.219, -0.653173
    dl 0xFFF5DA ; E4, 320.625, -0.634393
    dl 0xFFF628 ; E5, 322.031, -0.615232
    dl 0xFFF678 ; E6, 323.438, -0.595699
    dl 0xFFF6C9 ; E7, 324.844, -0.575808
    dl 0xFFF71C ; E8, 326.250, -0.555570
    dl 0xFFF771 ; E9, 327.656, -0.534998
    dl 0xFFF7C6 ; EA, 329.062, -0.514103
    dl 0xFFF81D ; EB, 330.469, -0.492898
    dl 0xFFF875 ; EC, 331.875, -0.471397
    dl 0xFFF8CE ; ED, 333.281, -0.449611
    dl 0xFFF929 ; EE, 334.688, -0.427555
    dl 0xFFF984 ; EF, 336.094, -0.405241
    dl 0xFFF9E1 ; F0, 337.500, -0.382683
    dl 0xFFFA3E ; F1, 338.906, -0.359895
    dl 0xFFFA9C ; F2, 340.312, -0.336890
    dl 0xFFFAFB ; F3, 341.719, -0.313682
    dl 0xFFFB5B ; F4, 343.125, -0.290285
    dl 0xFFFBBC ; F5, 344.531, -0.266713
    dl 0xFFFC1D ; F6, 345.938, -0.242980
    dl 0xFFFC7F ; F7, 347.344, -0.219101
    dl 0xFFFCE1 ; F8, 348.750, -0.195090
    dl 0xFFFD44 ; F9, 350.156, -0.170962
    dl 0xFFFDA7 ; FA, 351.562, -0.146730
    dl 0xFFFE0B ; FB, 352.969, -0.122411
    dl 0xFFFE6F ; FC, 354.375, -0.098017
    dl 0xFFFED3 ; FD, 355.781, -0.073565
    dl 0xFFFF37 ; FE, 357.188, -0.049068
    dl 0xFFFF9B ; FF, 358.594, -0.024541

atan_lut_168:
	dl 0x000000 ; 000000, 0.000
	dl 0x000028 ; 000001, 0.224
	dl 0x000051 ; 000002, 0.448
	dl 0x00007A ; 000003, 0.671
	dl 0x0000A2 ; 000004, 0.895
	dl 0x0000CB ; 000005, 1.119
	dl 0x0000F4 ; 000006, 1.343
	dl 0x00011D ; 000007, 1.566
	dl 0x000145 ; 000008, 1.790
	dl 0x00016E ; 000009, 2.013
	dl 0x000197 ; 00000A, 2.237
	dl 0x0001BF ; 00000B, 2.460
	dl 0x0001E8 ; 00000C, 2.684
	dl 0x000211 ; 00000D, 2.907
	dl 0x000239 ; 00000E, 3.130
	dl 0x000262 ; 00000F, 3.353
	dl 0x00028B ; 000010, 3.576
	dl 0x0002B3 ; 000011, 3.799
	dl 0x0002DC ; 000012, 4.022
	dl 0x000304 ; 000013, 4.245
	dl 0x00032D ; 000014, 4.467
	dl 0x000355 ; 000015, 4.690
	dl 0x00037E ; 000016, 4.912
	dl 0x0003A6 ; 000017, 5.134
	dl 0x0003CE ; 000018, 5.356
	dl 0x0003F7 ; 000019, 5.578
	dl 0x00041F ; 00001A, 5.799
	dl 0x000448 ; 00001B, 6.021
	dl 0x000470 ; 00001C, 6.242
	dl 0x000498 ; 00001D, 6.463
	dl 0x0004C0 ; 00001E, 6.684
	dl 0x0004E8 ; 00001F, 6.905
	dl 0x000511 ; 000020, 7.125
	dl 0x000539 ; 000021, 7.345
	dl 0x000561 ; 000022, 7.565
	dl 0x000589 ; 000023, 7.785
	dl 0x0005B1 ; 000024, 8.005
	dl 0x0005D9 ; 000025, 8.224
	dl 0x000601 ; 000026, 8.443
	dl 0x000628 ; 000027, 8.662
	dl 0x000650 ; 000028, 8.881
	dl 0x000678 ; 000029, 9.099
	dl 0x0006A0 ; 00002A, 9.317
	dl 0x0006C7 ; 00002B, 9.535
	dl 0x0006EF ; 00002C, 9.752
	dl 0x000716 ; 00002D, 9.970
	dl 0x00073E ; 00002E, 10.187
	dl 0x000765 ; 00002F, 10.403
	dl 0x00078D ; 000030, 10.620
	dl 0x0007B4 ; 000031, 10.836
	dl 0x0007DB ; 000032, 11.051
	dl 0x000803 ; 000033, 11.267
	dl 0x00082A ; 000034, 11.482
	dl 0x000851 ; 000035, 11.697
	dl 0x000878 ; 000036, 11.911
	dl 0x00089F ; 000037, 12.125
	dl 0x0008C6 ; 000038, 12.339
	dl 0x0008ED ; 000039, 12.553
	dl 0x000913 ; 00003A, 12.766
	dl 0x00093A ; 00003B, 12.978
	dl 0x000961 ; 00003C, 13.191
	dl 0x000987 ; 00003D, 13.403
	dl 0x0009AE ; 00003E, 13.614
	dl 0x0009D4 ; 00003F, 13.825
	dl 0x0009FB ; 000040, 14.036
	dl 0x000A21 ; 000041, 14.247
	dl 0x000A47 ; 000042, 14.457
	dl 0x000A6D ; 000043, 14.666
	dl 0x000A94 ; 000044, 14.876
	dl 0x000ABA ; 000045, 15.085
	dl 0x000AE0 ; 000046, 15.293
	dl 0x000B05 ; 000047, 15.501
	dl 0x000B2B ; 000048, 15.709
	dl 0x000B51 ; 000049, 15.916
	dl 0x000B77 ; 00004A, 16.123
	dl 0x000B9C ; 00004B, 16.329
	dl 0x000BC2 ; 00004C, 16.535
	dl 0x000BE7 ; 00004D, 16.740
	dl 0x000C0C ; 00004E, 16.945
	dl 0x000C32 ; 00004F, 17.150
	dl 0x000C57 ; 000050, 17.354
	dl 0x000C7C ; 000051, 17.558
	dl 0x000CA1 ; 000052, 17.761
	dl 0x000CC6 ; 000053, 17.964
	dl 0x000CEB ; 000054, 18.166
	dl 0x000D0F ; 000055, 18.368
	dl 0x000D34 ; 000056, 18.569
	dl 0x000D58 ; 000057, 18.770
	dl 0x000D7D ; 000058, 18.970
	dl 0x000DA1 ; 000059, 19.170
	dl 0x000DC6 ; 00005A, 19.370
	dl 0x000DEA ; 00005B, 19.569
	dl 0x000E0E ; 00005C, 19.767
	dl 0x000E32 ; 00005D, 19.965
	dl 0x000E56 ; 00005E, 20.163
	dl 0x000E7A ; 00005F, 20.360
	dl 0x000E9E ; 000060, 20.556
	dl 0x000EC1 ; 000061, 20.752
	dl 0x000EE5 ; 000062, 20.947
	dl 0x000F08 ; 000063, 21.142
	dl 0x000F2C ; 000064, 21.337
	dl 0x000F4F ; 000065, 21.531
	dl 0x000F72 ; 000066, 21.724
	dl 0x000F95 ; 000067, 21.917
	dl 0x000FB8 ; 000068, 22.109
	dl 0x000FDB ; 000069, 22.301
	dl 0x000FFE ; 00006A, 22.493
	dl 0x001021 ; 00006B, 22.683
	dl 0x001044 ; 00006C, 22.874
	dl 0x001066 ; 00006D, 23.063
	dl 0x001089 ; 00006E, 23.253
	dl 0x0010AB ; 00006F, 23.441
	dl 0x0010CD ; 000070, 23.629
	dl 0x0010EF ; 000071, 23.817
	dl 0x001111 ; 000072, 24.004
	dl 0x001133 ; 000073, 24.191
	dl 0x001155 ; 000074, 24.376
	dl 0x001177 ; 000075, 24.562
	dl 0x001199 ; 000076, 24.747
	dl 0x0011BA ; 000077, 24.931
	dl 0x0011DC ; 000078, 25.115
	dl 0x0011FD ; 000079, 25.298
	dl 0x00121E ; 00007A, 25.481
	dl 0x00123F ; 00007B, 25.663
	dl 0x001260 ; 00007C, 25.844
	dl 0x001281 ; 00007D, 26.025
	dl 0x0012A2 ; 00007E, 26.206
	dl 0x0012C3 ; 00007F, 26.386
	dl 0x0012E4 ; 000080, 26.565
	dl 0x001304 ; 000081, 26.744
	dl 0x001325 ; 000082, 26.922
	dl 0x001345 ; 000083, 27.100
	dl 0x001365 ; 000084, 27.277
	dl 0x001385 ; 000085, 27.453
	dl 0x0013A5 ; 000086, 27.629
	dl 0x0013C5 ; 000087, 27.805
	dl 0x0013E5 ; 000088, 27.979
	dl 0x001405 ; 000089, 28.154
	dl 0x001424 ; 00008A, 28.327
	dl 0x001444 ; 00008B, 28.501
	dl 0x001463 ; 00008C, 28.673
	dl 0x001483 ; 00008D, 28.845
	dl 0x0014A2 ; 00008E, 29.017
	dl 0x0014C1 ; 00008F, 29.187
	dl 0x0014E0 ; 000090, 29.358
	dl 0x0014FF ; 000091, 29.527
	dl 0x00151E ; 000092, 29.697
	dl 0x00153C ; 000093, 29.865
	dl 0x00155B ; 000094, 30.033
	dl 0x001579 ; 000095, 30.201
	dl 0x001598 ; 000096, 30.368
	dl 0x0015B6 ; 000097, 30.534
	dl 0x0015D4 ; 000098, 30.700
	dl 0x0015F2 ; 000099, 30.865
	dl 0x001610 ; 00009A, 31.030
	dl 0x00162E ; 00009B, 31.194
	dl 0x00164C ; 00009C, 31.357
	dl 0x00166A ; 00009D, 31.520
	dl 0x001687 ; 00009E, 31.682
	dl 0x0016A5 ; 00009F, 31.844
	dl 0x0016C2 ; 0000A0, 32.005
	dl 0x0016DF ; 0000A1, 32.166
	dl 0x0016FC ; 0000A2, 32.326
	dl 0x001719 ; 0000A3, 32.486
	dl 0x001736 ; 0000A4, 32.645
	dl 0x001753 ; 0000A5, 32.803
	dl 0x001770 ; 0000A6, 32.961
	dl 0x00178C ; 0000A7, 33.118
	dl 0x0017A9 ; 0000A8, 33.275
	dl 0x0017C5 ; 0000A9, 33.431
	dl 0x0017E2 ; 0000AA, 33.587
	dl 0x0017FE ; 0000AB, 33.742
	dl 0x00181A ; 0000AC, 33.896
	dl 0x001836 ; 0000AD, 34.050
	dl 0x001852 ; 0000AE, 34.203
	dl 0x00186E ; 0000AF, 34.356
	dl 0x00188A ; 0000B0, 34.509
	dl 0x0018A5 ; 0000B1, 34.660
	dl 0x0018C1 ; 0000B2, 34.811
	dl 0x0018DC ; 0000B3, 34.962
	dl 0x0018F7 ; 0000B4, 35.112
	dl 0x001913 ; 0000B5, 35.262
	dl 0x00192E ; 0000B6, 35.410
	dl 0x001949 ; 0000B7, 35.559
	dl 0x001964 ; 0000B8, 35.707
	dl 0x00197F ; 0000B9, 35.854
	dl 0x001999 ; 0000BA, 36.001
	dl 0x0019B4 ; 0000BB, 36.147
	dl 0x0019CE ; 0000BC, 36.293
	dl 0x0019E9 ; 0000BD, 36.438
	dl 0x001A03 ; 0000BE, 36.582
	dl 0x001A1D ; 0000BF, 36.726
	dl 0x001A37 ; 0000C0, 36.870
	dl 0x001A51 ; 0000C1, 37.013
	dl 0x001A6B ; 0000C2, 37.155
	dl 0x001A85 ; 0000C3, 37.297
	dl 0x001A9F ; 0000C4, 37.439
	dl 0x001AB9 ; 0000C5, 37.579
	dl 0x001AD2 ; 0000C6, 37.720
	dl 0x001AEC ; 0000C7, 37.859
	dl 0x001B05 ; 0000C8, 37.999
	dl 0x001B1E ; 0000C9, 38.137
	dl 0x001B37 ; 0000CA, 38.276
	dl 0x001B50 ; 0000CB, 38.413
	dl 0x001B69 ; 0000CC, 38.550
	dl 0x001B82 ; 0000CD, 38.687
	dl 0x001B9B ; 0000CE, 38.823
	dl 0x001BB4 ; 0000CF, 38.959
	dl 0x001BCC ; 0000D0, 39.094
	dl 0x001BE5 ; 0000D1, 39.228
	dl 0x001BFD ; 0000D2, 39.362
	dl 0x001C16 ; 0000D3, 39.496
	dl 0x001C2E ; 0000D4, 39.629
	dl 0x001C46 ; 0000D5, 39.762
	dl 0x001C5E ; 0000D6, 39.894
	dl 0x001C76 ; 0000D7, 40.025
	dl 0x001C8E ; 0000D8, 40.156
	dl 0x001CA5 ; 0000D9, 40.286
	dl 0x001CBD ; 0000DA, 40.416
	dl 0x001CD5 ; 0000DB, 40.546
	dl 0x001CEC ; 0000DC, 40.675
	dl 0x001D04 ; 0000DD, 40.803
	dl 0x001D1B ; 0000DE, 40.931
	dl 0x001D32 ; 0000DF, 41.059
	dl 0x001D49 ; 0000E0, 41.186
	dl 0x001D60 ; 0000E1, 41.312
	dl 0x001D77 ; 0000E2, 41.438
	dl 0x001D8E ; 0000E3, 41.564
	dl 0x001DA5 ; 0000E4, 41.689
	dl 0x001DBB ; 0000E5, 41.814
	dl 0x001DD2 ; 0000E6, 41.938
	dl 0x001DE9 ; 0000E7, 42.061
	dl 0x001DFF ; 0000E8, 42.184
	dl 0x001E15 ; 0000E9, 42.307
	dl 0x001E2C ; 0000EA, 42.429
	dl 0x001E42 ; 0000EB, 42.551
	dl 0x001E58 ; 0000EC, 42.672
	dl 0x001E6E ; 0000ED, 42.793
	dl 0x001E84 ; 0000EE, 42.913
	dl 0x001E99 ; 0000EF, 43.033
	dl 0x001EAF ; 0000F0, 43.152
	dl 0x001EC5 ; 0000F1, 43.271
	dl 0x001EDA ; 0000F2, 43.390
	dl 0x001EF0 ; 0000F3, 43.508
	dl 0x001F05 ; 0000F4, 43.625
	dl 0x001F1B ; 0000F5, 43.742
	dl 0x001F30 ; 0000F6, 43.859
	dl 0x001F45 ; 0000F7, 43.975
	dl 0x001F5A ; 0000F8, 44.091
	dl 0x001F6F ; 0000F9, 44.206
	dl 0x001F84 ; 0000FA, 44.321
	dl 0x001F99 ; 0000FB, 44.435
	dl 0x001FAD ; 0000FC, 44.549
	dl 0x001FC2 ; 0000FD, 44.662
	dl 0x001FD7 ; 0000FE, 44.775
	dl 0x001FEB ; 0000FF, 44.888
	dl 0x002000 ; 000100, 45.000 only needed for interpolation