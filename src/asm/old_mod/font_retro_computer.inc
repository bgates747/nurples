; Bitmap indices:
BUF_0288: equ 0x0120 ; 32  
BUF_0289: equ 0x0121 ; 33 !
; Missing character 34 "
; Missing character 35 #
; Missing character 36 $
; Missing character 37 %
; Missing character 38 &
; Missing character 39 '
; Missing character 40 (
; Missing character 41 )
; Missing character 42 *
; Missing character 43 +
; Missing character 44 ,
; Missing character 45 -
; Missing character 46 .
; Missing character 47 /
BUF_0304: equ 0x0130 ; 48 0
BUF_0305: equ 0x0131 ; 49 1
BUF_0306: equ 0x0132 ; 50 2
BUF_0307: equ 0x0133 ; 51 3
BUF_0308: equ 0x0134 ; 52 4
BUF_0309: equ 0x0135 ; 53 5
BUF_0310: equ 0x0136 ; 54 6
BUF_0311: equ 0x0137 ; 55 7
BUF_0312: equ 0x0138 ; 56 8
BUF_0313: equ 0x0139 ; 57 9
; Missing character 58 :
; Missing character 59 ;
; Missing character 60 <
; Missing character 61 =
; Missing character 62 >
BUF_0319: equ 0x013F ; 63 ?
; Missing character 64 @
BUF_0321: equ 0x0141 ; 65 A
BUF_0322: equ 0x0142 ; 66 B
BUF_0323: equ 0x0143 ; 67 C
BUF_0324: equ 0x0144 ; 68 D
BUF_0325: equ 0x0145 ; 69 E
BUF_0326: equ 0x0146 ; 70 F
BUF_0327: equ 0x0147 ; 71 G
BUF_0328: equ 0x0148 ; 72 H
BUF_0329: equ 0x0149 ; 73 I
BUF_0330: equ 0x014A ; 74 J
BUF_0331: equ 0x014B ; 75 K
BUF_0332: equ 0x014C ; 76 L
BUF_0333: equ 0x014D ; 77 M
BUF_0334: equ 0x014E ; 78 N
BUF_0335: equ 0x014F ; 79 O
BUF_0336: equ 0x0150 ; 80 P
BUF_0337: equ 0x0151 ; 81 Q
BUF_0338: equ 0x0152 ; 82 R
BUF_0339: equ 0x0153 ; 83 S
BUF_0340: equ 0x0154 ; 84 T
BUF_0341: equ 0x0155 ; 85 U
BUF_0342: equ 0x0156 ; 86 V
BUF_0343: equ 0x0157 ; 87 W
BUF_0344: equ 0x0158 ; 88 X
BUF_0345: equ 0x0159 ; 89 Y
BUF_0346: equ 0x015A ; 90 Z
; Missing character 91 [
; Missing character 92 \
; Missing character 93 ]
; Missing character 94 ^
; Missing character 95 _
; Missing character 96 `
; Missing character 97 a
; Missing character 98 b
; Missing character 99 c
; Missing character 100 d
; Missing character 101 e
; Missing character 102 f
; Missing character 103 g
; Missing character 104 h
; Missing character 105 i
; Missing character 106 j
; Missing character 107 k
; Missing character 108 l
; Missing character 109 m
; Missing character 110 n
; Missing character 111 o
; Missing character 112 p
; Missing character 113 q
; Missing character 114 r
; Missing character 115 s
; Missing character 116 t
; Missing character 117 u
; Missing character 118 v
; Missing character 119 w
; Missing character 120 x
; Missing character 121 y
; Missing character 122 z
; [y_offset, dim_y, dim_x], buffer_id label: ; mind the little-endian order when fetching these!!!
font_rc:
	dl 0x000106,BUF_0288
	dl 0x000E03,BUF_0289
	dl 0x000106,BUF_0288 ; Missing character 34
	dl 0x000106,BUF_0288 ; Missing character 35
	dl 0x000106,BUF_0288 ; Missing character 36
	dl 0x000106,BUF_0288 ; Missing character 37
	dl 0x000106,BUF_0288 ; Missing character 38
	dl 0x000106,BUF_0288 ; Missing character 39
	dl 0x000106,BUF_0288 ; Missing character 40
	dl 0x000106,BUF_0288 ; Missing character 41
	dl 0x000106,BUF_0288 ; Missing character 42
	dl 0x000106,BUF_0288 ; Missing character 43
	dl 0x000106,BUF_0288 ; Missing character 44
	dl 0x000106,BUF_0288 ; Missing character 45
	dl 0x000106,BUF_0288 ; Missing character 46
	dl 0x000106,BUF_0288 ; Missing character 47
	dl 0x000E08,BUF_0304
	dl 0x000E08,BUF_0305
	dl 0x000E08,BUF_0306
	dl 0x000E08,BUF_0307
	dl 0x000E08,BUF_0308
	dl 0x000E08,BUF_0309
	dl 0x000E08,BUF_0310
	dl 0x000E08,BUF_0311
	dl 0x000E08,BUF_0312
	dl 0x000E08,BUF_0313
	dl 0x000106,BUF_0288 ; Missing character 58
	dl 0x000106,BUF_0288 ; Missing character 59
	dl 0x000106,BUF_0288 ; Missing character 60
	dl 0x000106,BUF_0288 ; Missing character 61
	dl 0x000106,BUF_0288 ; Missing character 62
	dl 0x000E08,BUF_0319
	dl 0x000106,BUF_0288 ; Missing character 64
	dl 0x000E07,BUF_0321
	dl 0x000E08,BUF_0322
	dl 0x000E07,BUF_0323
	dl 0x000E08,BUF_0324
	dl 0x000E08,BUF_0325
	dl 0x000E07,BUF_0326
	dl 0x000E08,BUF_0327
	dl 0x000E07,BUF_0328
	dl 0x000E07,BUF_0329
	dl 0x000E08,BUF_0330
	dl 0x000E07,BUF_0331
	dl 0x000E08,BUF_0332
	dl 0x000E09,BUF_0333
	dl 0x000E07,BUF_0334
	dl 0x000E08,BUF_0335
	dl 0x000E07,BUF_0336
	dl 0x000F08,BUF_0337
	dl 0x000E08,BUF_0338
	dl 0x000E08,BUF_0339
	dl 0x000E07,BUF_0340
	dl 0x000E08,BUF_0341
	dl 0x000E07,BUF_0342
	dl 0x000E0B,BUF_0343
	dl 0x000E07,BUF_0344
	dl 0x000E08,BUF_0345
	dl 0x000E07,BUF_0346
	dl 0x000106,BUF_0288 ; Missing character 91
	dl 0x000106,BUF_0288 ; Missing character 92
	dl 0x000106,BUF_0288 ; Missing character 93
	dl 0x000106,BUF_0288 ; Missing character 94
	dl 0x000106,BUF_0288 ; Missing character 95
	dl 0x000106,BUF_0288 ; Missing character 96
	dl 0x000106,BUF_0288 ; Missing character 97
	dl 0x000106,BUF_0288 ; Missing character 98
	dl 0x000106,BUF_0288 ; Missing character 99
	dl 0x000106,BUF_0288 ; Missing character 100
	dl 0x000106,BUF_0288 ; Missing character 101
	dl 0x000106,BUF_0288 ; Missing character 102
	dl 0x000106,BUF_0288 ; Missing character 103
	dl 0x000106,BUF_0288 ; Missing character 104
	dl 0x000106,BUF_0288 ; Missing character 105
	dl 0x000106,BUF_0288 ; Missing character 106
	dl 0x000106,BUF_0288 ; Missing character 107
	dl 0x000106,BUF_0288 ; Missing character 108
	dl 0x000106,BUF_0288 ; Missing character 109
	dl 0x000106,BUF_0288 ; Missing character 110
	dl 0x000106,BUF_0288 ; Missing character 111
	dl 0x000106,BUF_0288 ; Missing character 112
	dl 0x000106,BUF_0288 ; Missing character 113
	dl 0x000106,BUF_0288 ; Missing character 114
	dl 0x000106,BUF_0288 ; Missing character 115
	dl 0x000106,BUF_0288 ; Missing character 116
	dl 0x000106,BUF_0288 ; Missing character 117
	dl 0x000106,BUF_0288 ; Missing character 118
	dl 0x000106,BUF_0288 ; Missing character 119
	dl 0x000106,BUF_0288 ; Missing character 120
	dl 0x000106,BUF_0288 ; Missing character 121
	dl 0x000106,BUF_0288 ; Missing character 122

; Import .rgba2 bitmap files and load them into VDP buffers
load_font_rc:

	ld hl,BUF_0288
	ld bc,6
	ld de,1
	ld ix,6
	ld iy,Frc032
	call vdu_load_img_2

	ld hl,BUF_0289
	ld bc,3
	ld de,14
	ld ix,42
	ld iy,Frc033
	call vdu_load_img_2
; Missing character 34
; Missing character 35
; Missing character 36
; Missing character 37
; Missing character 38
; Missing character 39
; Missing character 40
; Missing character 41
; Missing character 42
; Missing character 43
; Missing character 44
; Missing character 45
; Missing character 46
; Missing character 47

	ld hl,BUF_0304
	ld bc,8
	ld de,14
	ld ix,112
	ld iy,Frc048
	call vdu_load_img_2

	ld hl,BUF_0305
	ld bc,8
	ld de,14
	ld ix,112
	ld iy,Frc049
	call vdu_load_img_2

	ld hl,BUF_0306
	ld bc,8
	ld de,14
	ld ix,112
	ld iy,Frc050
	call vdu_load_img_2

	ld hl,BUF_0307
	ld bc,8
	ld de,14
	ld ix,112
	ld iy,Frc051
	call vdu_load_img_2

	ld hl,BUF_0308
	ld bc,8
	ld de,14
	ld ix,112
	ld iy,Frc052
	call vdu_load_img_2

	ld hl,BUF_0309
	ld bc,8
	ld de,14
	ld ix,112
	ld iy,Frc053
	call vdu_load_img_2

	ld hl,BUF_0310
	ld bc,8
	ld de,14
	ld ix,112
	ld iy,Frc054
	call vdu_load_img_2

	ld hl,BUF_0311
	ld bc,8
	ld de,14
	ld ix,112
	ld iy,Frc055
	call vdu_load_img_2

	ld hl,BUF_0312
	ld bc,8
	ld de,14
	ld ix,112
	ld iy,Frc056
	call vdu_load_img_2

	ld hl,BUF_0313
	ld bc,8
	ld de,14
	ld ix,112
	ld iy,Frc057
	call vdu_load_img_2
; Missing character 58
; Missing character 59
; Missing character 60
; Missing character 61
; Missing character 62

	ld hl,BUF_0319
	ld bc,8
	ld de,14
	ld ix,112
	ld iy,Frc063
	call vdu_load_img_2
; Missing character 64

	ld hl,BUF_0321
	ld bc,7
	ld de,14
	ld ix,98
	ld iy,Frc065
	call vdu_load_img_2

	ld hl,BUF_0322
	ld bc,8
	ld de,14
	ld ix,112
	ld iy,Frc066
	call vdu_load_img_2

	ld hl,BUF_0323
	ld bc,7
	ld de,14
	ld ix,98
	ld iy,Frc067
	call vdu_load_img_2

	ld hl,BUF_0324
	ld bc,8
	ld de,14
	ld ix,112
	ld iy,Frc068
	call vdu_load_img_2

	ld hl,BUF_0325
	ld bc,8
	ld de,14
	ld ix,112
	ld iy,Frc069
	call vdu_load_img_2

	ld hl,BUF_0326
	ld bc,7
	ld de,14
	ld ix,98
	ld iy,Frc070
	call vdu_load_img_2

	ld hl,BUF_0327
	ld bc,8
	ld de,14
	ld ix,112
	ld iy,Frc071
	call vdu_load_img_2

	ld hl,BUF_0328
	ld bc,7
	ld de,14
	ld ix,98
	ld iy,Frc072
	call vdu_load_img_2

	ld hl,BUF_0329
	ld bc,7
	ld de,14
	ld ix,98
	ld iy,Frc073
	call vdu_load_img_2

	ld hl,BUF_0330
	ld bc,8
	ld de,14
	ld ix,112
	ld iy,Frc074
	call vdu_load_img_2

	ld hl,BUF_0331
	ld bc,7
	ld de,14
	ld ix,98
	ld iy,Frc075
	call vdu_load_img_2

	ld hl,BUF_0332
	ld bc,8
	ld de,14
	ld ix,112
	ld iy,Frc076
	call vdu_load_img_2

	ld hl,BUF_0333
	ld bc,9
	ld de,14
	ld ix,126
	ld iy,Frc077
	call vdu_load_img_2

	ld hl,BUF_0334
	ld bc,7
	ld de,14
	ld ix,98
	ld iy,Frc078
	call vdu_load_img_2

	ld hl,BUF_0335
	ld bc,8
	ld de,14
	ld ix,112
	ld iy,Frc079
	call vdu_load_img_2

	ld hl,BUF_0336
	ld bc,7
	ld de,14
	ld ix,98
	ld iy,Frc080
	call vdu_load_img_2

	ld hl,BUF_0337
	ld bc,8
	ld de,15
	ld ix,120
	ld iy,Frc081
	call vdu_load_img_2

	ld hl,BUF_0338
	ld bc,8
	ld de,14
	ld ix,112
	ld iy,Frc082
	call vdu_load_img_2

	ld hl,BUF_0339
	ld bc,8
	ld de,14
	ld ix,112
	ld iy,Frc083
	call vdu_load_img_2

	ld hl,BUF_0340
	ld bc,7
	ld de,14
	ld ix,98
	ld iy,Frc084
	call vdu_load_img_2

	ld hl,BUF_0341
	ld bc,8
	ld de,14
	ld ix,112
	ld iy,Frc085
	call vdu_load_img_2

	ld hl,BUF_0342
	ld bc,7
	ld de,14
	ld ix,98
	ld iy,Frc086
	call vdu_load_img_2

	ld hl,BUF_0343
	ld bc,11
	ld de,14
	ld ix,154
	ld iy,Frc087
	call vdu_load_img_2

	ld hl,BUF_0344
	ld bc,7
	ld de,14
	ld ix,98
	ld iy,Frc088
	call vdu_load_img_2

	ld hl,BUF_0345
	ld bc,8
	ld de,14
	ld ix,112
	ld iy,Frc089
	call vdu_load_img_2

	ld hl,BUF_0346
	ld bc,7
	ld de,14
	ld ix,98
	ld iy,Frc090
	call vdu_load_img_2
; Missing character 91
; Missing character 92
; Missing character 93
; Missing character 94
; Missing character 95
; Missing character 96
; Missing character 97
; Missing character 98
; Missing character 99
; Missing character 100
; Missing character 101
; Missing character 102
; Missing character 103
; Missing character 104
; Missing character 105
; Missing character 106
; Missing character 107
; Missing character 108
; Missing character 109
; Missing character 110
; Missing character 111
; Missing character 112
; Missing character 113
; Missing character 114
; Missing character 115
; Missing character 116
; Missing character 117
; Missing character 118
; Missing character 119
; Missing character 120
; Missing character 121
; Missing character 122

	ret

Frc032: db "fonts/rc/032.rgba2",0
Frc033: db "fonts/rc/033.rgba2",0
Frc048: db "fonts/rc/048.rgba2",0
Frc049: db "fonts/rc/049.rgba2",0
Frc050: db "fonts/rc/050.rgba2",0
Frc051: db "fonts/rc/051.rgba2",0
Frc052: db "fonts/rc/052.rgba2",0
Frc053: db "fonts/rc/053.rgba2",0
Frc054: db "fonts/rc/054.rgba2",0
Frc055: db "fonts/rc/055.rgba2",0
Frc056: db "fonts/rc/056.rgba2",0
Frc057: db "fonts/rc/057.rgba2",0
Frc063: db "fonts/rc/063.rgba2",0
Frc065: db "fonts/rc/065.rgba2",0
Frc066: db "fonts/rc/066.rgba2",0
Frc067: db "fonts/rc/067.rgba2",0
Frc068: db "fonts/rc/068.rgba2",0
Frc069: db "fonts/rc/069.rgba2",0
Frc070: db "fonts/rc/070.rgba2",0
Frc071: db "fonts/rc/071.rgba2",0
Frc072: db "fonts/rc/072.rgba2",0
Frc073: db "fonts/rc/073.rgba2",0
Frc074: db "fonts/rc/074.rgba2",0
Frc075: db "fonts/rc/075.rgba2",0
Frc076: db "fonts/rc/076.rgba2",0
Frc077: db "fonts/rc/077.rgba2",0
Frc078: db "fonts/rc/078.rgba2",0
Frc079: db "fonts/rc/079.rgba2",0
Frc080: db "fonts/rc/080.rgba2",0
Frc081: db "fonts/rc/081.rgba2",0
Frc082: db "fonts/rc/082.rgba2",0
Frc083: db "fonts/rc/083.rgba2",0
Frc084: db "fonts/rc/084.rgba2",0
Frc085: db "fonts/rc/085.rgba2",0
Frc086: db "fonts/rc/086.rgba2",0
Frc087: db "fonts/rc/087.rgba2",0
Frc088: db "fonts/rc/088.rgba2",0
Frc089: db "fonts/rc/089.rgba2",0
Frc090: db "fonts/rc/090.rgba2",0
