; Bitmap indices:
BUF_0032: equ 0x0020 ; 32  
BUF_0033: equ 0x0021 ; 33 !
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
BUF_0048: equ 0x0030 ; 48 0
BUF_0049: equ 0x0031 ; 49 1
BUF_0050: equ 0x0032 ; 50 2
BUF_0051: equ 0x0033 ; 51 3
BUF_0052: equ 0x0034 ; 52 4
BUF_0053: equ 0x0035 ; 53 5
BUF_0054: equ 0x0036 ; 54 6
BUF_0055: equ 0x0037 ; 55 7
BUF_0056: equ 0x0038 ; 56 8
BUF_0057: equ 0x0039 ; 57 9
; Missing character 58 :
; Missing character 59 ;
; Missing character 60 <
; Missing character 61 =
; Missing character 62 >
BUF_0063: equ 0x003F ; 63 ?
; Missing character 64 @
BUF_0065: equ 0x0041 ; 65 A
BUF_0066: equ 0x0042 ; 66 B
BUF_0067: equ 0x0043 ; 67 C
BUF_0068: equ 0x0044 ; 68 D
BUF_0069: equ 0x0045 ; 69 E
BUF_0070: equ 0x0046 ; 70 F
BUF_0071: equ 0x0047 ; 71 G
BUF_0072: equ 0x0048 ; 72 H
BUF_0073: equ 0x0049 ; 73 I
BUF_0074: equ 0x004A ; 74 J
BUF_0075: equ 0x004B ; 75 K
BUF_0076: equ 0x004C ; 76 L
BUF_0077: equ 0x004D ; 77 M
BUF_0078: equ 0x004E ; 78 N
BUF_0079: equ 0x004F ; 79 O
BUF_0080: equ 0x0050 ; 80 P
BUF_0081: equ 0x0051 ; 81 Q
BUF_0082: equ 0x0052 ; 82 R
BUF_0083: equ 0x0053 ; 83 S
BUF_0084: equ 0x0054 ; 84 T
BUF_0085: equ 0x0055 ; 85 U
BUF_0086: equ 0x0056 ; 86 V
BUF_0087: equ 0x0057 ; 87 W
BUF_0088: equ 0x0058 ; 88 X
BUF_0089: equ 0x0059 ; 89 Y
BUF_0090: equ 0x005A ; 90 Z
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
	dl 0x000106,BUF_0032
	dl 0x000E03,BUF_0033
	dl 0x000106,BUF_0032 ; Missing character 34
	dl 0x000106,BUF_0032 ; Missing character 35
	dl 0x000106,BUF_0032 ; Missing character 36
	dl 0x000106,BUF_0032 ; Missing character 37
	dl 0x000106,BUF_0032 ; Missing character 38
	dl 0x000106,BUF_0032 ; Missing character 39
	dl 0x000106,BUF_0032 ; Missing character 40
	dl 0x000106,BUF_0032 ; Missing character 41
	dl 0x000106,BUF_0032 ; Missing character 42
	dl 0x000106,BUF_0032 ; Missing character 43
	dl 0x000106,BUF_0032 ; Missing character 44
	dl 0x000106,BUF_0032 ; Missing character 45
	dl 0x000106,BUF_0032 ; Missing character 46
	dl 0x000106,BUF_0032 ; Missing character 47
	dl 0x000E08,BUF_0048
	dl 0x000E08,BUF_0049
	dl 0x000E08,BUF_0050
	dl 0x000E08,BUF_0051
	dl 0x000E08,BUF_0052
	dl 0x000E08,BUF_0053
	dl 0x000E08,BUF_0054
	dl 0x000E08,BUF_0055
	dl 0x000E08,BUF_0056
	dl 0x000E08,BUF_0057
	dl 0x000106,BUF_0032 ; Missing character 58
	dl 0x000106,BUF_0032 ; Missing character 59
	dl 0x000106,BUF_0032 ; Missing character 60
	dl 0x000106,BUF_0032 ; Missing character 61
	dl 0x000106,BUF_0032 ; Missing character 62
	dl 0x000E08,BUF_0063
	dl 0x000106,BUF_0032 ; Missing character 64
	dl 0x000E07,BUF_0065
	dl 0x000E08,BUF_0066
	dl 0x000E07,BUF_0067
	dl 0x000E08,BUF_0068
	dl 0x000E08,BUF_0069
	dl 0x000E07,BUF_0070
	dl 0x000E08,BUF_0071
	dl 0x000E07,BUF_0072
	dl 0x000E07,BUF_0073
	dl 0x000E08,BUF_0074
	dl 0x000E07,BUF_0075
	dl 0x000E08,BUF_0076
	dl 0x000E09,BUF_0077
	dl 0x000E07,BUF_0078
	dl 0x000E08,BUF_0079
	dl 0x000E07,BUF_0080
	dl 0x000F08,BUF_0081
	dl 0x000E08,BUF_0082
	dl 0x000E08,BUF_0083
	dl 0x000E07,BUF_0084
	dl 0x000E08,BUF_0085
	dl 0x000E07,BUF_0086
	dl 0x000E0B,BUF_0087
	dl 0x000E07,BUF_0088
	dl 0x000E08,BUF_0089
	dl 0x000E07,BUF_0090
	dl 0x000106,BUF_0032 ; Missing character 91
	dl 0x000106,BUF_0032 ; Missing character 92
	dl 0x000106,BUF_0032 ; Missing character 93
	dl 0x000106,BUF_0032 ; Missing character 94
	dl 0x000106,BUF_0032 ; Missing character 95
	dl 0x000106,BUF_0032 ; Missing character 96
	dl 0x000106,BUF_0032 ; Missing character 97
	dl 0x000106,BUF_0032 ; Missing character 98
	dl 0x000106,BUF_0032 ; Missing character 99
	dl 0x000106,BUF_0032 ; Missing character 100
	dl 0x000106,BUF_0032 ; Missing character 101
	dl 0x000106,BUF_0032 ; Missing character 102
	dl 0x000106,BUF_0032 ; Missing character 103
	dl 0x000106,BUF_0032 ; Missing character 104
	dl 0x000106,BUF_0032 ; Missing character 105
	dl 0x000106,BUF_0032 ; Missing character 106
	dl 0x000106,BUF_0032 ; Missing character 107
	dl 0x000106,BUF_0032 ; Missing character 108
	dl 0x000106,BUF_0032 ; Missing character 109
	dl 0x000106,BUF_0032 ; Missing character 110
	dl 0x000106,BUF_0032 ; Missing character 111
	dl 0x000106,BUF_0032 ; Missing character 112
	dl 0x000106,BUF_0032 ; Missing character 113
	dl 0x000106,BUF_0032 ; Missing character 114
	dl 0x000106,BUF_0032 ; Missing character 115
	dl 0x000106,BUF_0032 ; Missing character 116
	dl 0x000106,BUF_0032 ; Missing character 117
	dl 0x000106,BUF_0032 ; Missing character 118
	dl 0x000106,BUF_0032 ; Missing character 119
	dl 0x000106,BUF_0032 ; Missing character 120
	dl 0x000106,BUF_0032 ; Missing character 121
	dl 0x000106,BUF_0032 ; Missing character 122

; Import .rgba2 bitmap files and load them into VDP buffers
load_font_rc:

	ld hl,Frc032
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0032
	ld bc,6
	ld de,1
	ld ix,6
	call vdu_load_img

	ld hl,Frc033
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0033
	ld bc,3
	ld de,14
	ld ix,42
	call vdu_load_img
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

	ld hl,Frc048
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0048
	ld bc,8
	ld de,14
	ld ix,112
	call vdu_load_img

	ld hl,Frc049
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0049
	ld bc,8
	ld de,14
	ld ix,112
	call vdu_load_img

	ld hl,Frc050
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0050
	ld bc,8
	ld de,14
	ld ix,112
	call vdu_load_img

	ld hl,Frc051
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0051
	ld bc,8
	ld de,14
	ld ix,112
	call vdu_load_img

	ld hl,Frc052
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0052
	ld bc,8
	ld de,14
	ld ix,112
	call vdu_load_img

	ld hl,Frc053
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0053
	ld bc,8
	ld de,14
	ld ix,112
	call vdu_load_img

	ld hl,Frc054
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0054
	ld bc,8
	ld de,14
	ld ix,112
	call vdu_load_img

	ld hl,Frc055
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0055
	ld bc,8
	ld de,14
	ld ix,112
	call vdu_load_img

	ld hl,Frc056
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0056
	ld bc,8
	ld de,14
	ld ix,112
	call vdu_load_img

	ld hl,Frc057
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0057
	ld bc,8
	ld de,14
	ld ix,112
	call vdu_load_img
; Missing character 58
; Missing character 59
; Missing character 60
; Missing character 61
; Missing character 62

	ld hl,Frc063
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0063
	ld bc,8
	ld de,14
	ld ix,112
	call vdu_load_img
; Missing character 64

	ld hl,Frc065
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0065
	ld bc,7
	ld de,14
	ld ix,98
	call vdu_load_img

	ld hl,Frc066
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0066
	ld bc,8
	ld de,14
	ld ix,112
	call vdu_load_img

	ld hl,Frc067
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0067
	ld bc,7
	ld de,14
	ld ix,98
	call vdu_load_img

	ld hl,Frc068
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0068
	ld bc,8
	ld de,14
	ld ix,112
	call vdu_load_img

	ld hl,Frc069
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0069
	ld bc,8
	ld de,14
	ld ix,112
	call vdu_load_img

	ld hl,Frc070
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0070
	ld bc,7
	ld de,14
	ld ix,98
	call vdu_load_img

	ld hl,Frc071
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0071
	ld bc,8
	ld de,14
	ld ix,112
	call vdu_load_img

	ld hl,Frc072
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0072
	ld bc,7
	ld de,14
	ld ix,98
	call vdu_load_img

	ld hl,Frc073
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0073
	ld bc,7
	ld de,14
	ld ix,98
	call vdu_load_img

	ld hl,Frc074
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0074
	ld bc,8
	ld de,14
	ld ix,112
	call vdu_load_img

	ld hl,Frc075
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0075
	ld bc,7
	ld de,14
	ld ix,98
	call vdu_load_img

	ld hl,Frc076
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0076
	ld bc,8
	ld de,14
	ld ix,112
	call vdu_load_img

	ld hl,Frc077
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0077
	ld bc,9
	ld de,14
	ld ix,126
	call vdu_load_img

	ld hl,Frc078
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0078
	ld bc,7
	ld de,14
	ld ix,98
	call vdu_load_img

	ld hl,Frc079
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0079
	ld bc,8
	ld de,14
	ld ix,112
	call vdu_load_img

	ld hl,Frc080
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0080
	ld bc,7
	ld de,14
	ld ix,98
	call vdu_load_img

	ld hl,Frc081
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0081
	ld bc,8
	ld de,15
	ld ix,120
	call vdu_load_img

	ld hl,Frc082
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0082
	ld bc,8
	ld de,14
	ld ix,112
	call vdu_load_img

	ld hl,Frc083
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0083
	ld bc,8
	ld de,14
	ld ix,112
	call vdu_load_img

	ld hl,Frc084
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0084
	ld bc,7
	ld de,14
	ld ix,98
	call vdu_load_img

	ld hl,Frc085
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0085
	ld bc,8
	ld de,14
	ld ix,112
	call vdu_load_img

	ld hl,Frc086
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0086
	ld bc,7
	ld de,14
	ld ix,98
	call vdu_load_img

	ld hl,Frc087
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0087
	ld bc,11
	ld de,14
	ld ix,154
	call vdu_load_img

	ld hl,Frc088
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0088
	ld bc,7
	ld de,14
	ld ix,98
	call vdu_load_img

	ld hl,Frc089
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0089
	ld bc,8
	ld de,14
	ld ix,112
	call vdu_load_img

	ld hl,Frc090
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_0090
	ld bc,7
	ld de,14
	ld ix,98
	call vdu_load_img
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
