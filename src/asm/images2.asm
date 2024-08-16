; Bitmap indices:
BUF_0TILE_EMPTY: equ $0100
BUF_1TILE_CROSS: equ $0101
BUF_2TILE_HORIZ: equ $0102
BUF_3TILE_VERT: equ $0103
BUF_4TILE_SQUARE: equ $0104
BUF_5TILE_CIRCLE: equ $0105
BUF_6TILE_PAD: equ $0106
BUF_7TILE_TURRET: equ $0107
BUF_CIRCLE: equ $0108
BUF_CRATER: equ $0109
BUF_EXPLOSION_A: equ $010A
BUF_EXPLOSION_B: equ $010B
BUF_EXPLOSION_C: equ $010C
BUF_EXPLOSION_D: equ $010D
BUF_EXPLOSION_E: equ $010E
BUF_FIREBALL_A: equ $010F
BUF_FIREBALL_B: equ $0110
BUF_LASER_A: equ $0111
BUF_LASER_B: equ $0112
BUF_PAD: equ $0113
BUF_SEEKER_000: equ $0114
BUF_SEEKER_001: equ $0115
BUF_SEEKER_002: equ $0116
BUF_SEEKER_003: equ $0117
BUF_SEEKER_004: equ $0118
BUF_SEEKER_005: equ $0119
BUF_SEEKER_006: equ $011A
BUF_SEEKER_007: equ $011B
BUF_SEEKER_008: equ $011C
BUF_SEEKER_009: equ $011D
BUF_SEEKER_010: equ $011E
BUF_SEEKER_011: equ $011F
BUF_SEEKER_012: equ $0120
BUF_SEEKER_013: equ $0121
BUF_SEEKER_014: equ $0122
BUF_SEEKER_015: equ $0123
BUF_SEEKER_016: equ $0124
BUF_SEEKER_017: equ $0125
BUF_SEEKER_018: equ $0126
BUF_SEEKER_019: equ $0127
BUF_SEEKER_020: equ $0128
BUF_SEEKER_021: equ $0129
BUF_SEEKER_022: equ $012A
BUF_SEEKER_023: equ $012B
BUF_SEEKER_024: equ $012C
BUF_SEEKER_025: equ $012D
BUF_SEEKER_026: equ $012E
BUF_SEEKER_027: equ $012F
BUF_SEEKER_028: equ $0130
BUF_SEEKER_029: equ $0131
BUF_SEEKER_030: equ $0132
BUF_SEEKER_031: equ $0133
BUF_SHIP_0L: equ $0134
BUF_SHIP_1C: equ $0135
BUF_SHIP_2R: equ $0136
BUF_SHIP_SMALL: equ $0137
BUF_STAR: equ $0138
BUF_TURRET: equ $0139
BUF_TURRET_ROT: equ $013A

; import .rgba bitmap files and load them into VDP buffers
bmp2_init:
	ld hl, @cmd
	ld bc, @end-@cmd
	rst.lil $18
	ret
@cmd:
	LOADBMPBUFFER2 BUF_0TILE_EMPTY,16,16,"nurples/src/rgba2/0tile_empty.rgba2"
	LOADBMPBUFFER2 BUF_1TILE_CROSS,16,16,"nurples/src/rgba2/1tile_cross.rgba2"
	LOADBMPBUFFER2 BUF_2TILE_HORIZ,16,16,"nurples/src/rgba2/2tile_horiz.rgba2"
	LOADBMPBUFFER2 BUF_3TILE_VERT,16,16,"nurples/src/rgba2/3tile_vert.rgba2"
	LOADBMPBUFFER2 BUF_4TILE_SQUARE,16,16,"nurples/src/rgba2/4tile_square.rgba2"
	LOADBMPBUFFER2 BUF_5TILE_CIRCLE,16,16,"nurples/src/rgba2/5tile_circle.rgba2"
	LOADBMPBUFFER2 BUF_6TILE_PAD,16,16,"nurples/src/rgba2/6tile_pad.rgba2"
	LOADBMPBUFFER2 BUF_7TILE_TURRET,16,16,"nurples/src/rgba2/7tile_turret.rgba2"
	LOADBMPBUFFER2 BUF_CIRCLE,16,16,"nurples/src/rgba2/circle.rgba2"
	LOADBMPBUFFER2 BUF_CRATER,16,16,"nurples/src/rgba2/crater.rgba2"
	LOADBMPBUFFER2 BUF_EXPLOSION_A,16,16,"nurples/src/rgba2/explosion_a.rgba2"
	LOADBMPBUFFER2 BUF_EXPLOSION_B,16,16,"nurples/src/rgba2/explosion_b.rgba2"
	LOADBMPBUFFER2 BUF_EXPLOSION_C,16,16,"nurples/src/rgba2/explosion_c.rgba2"
	LOADBMPBUFFER2 BUF_EXPLOSION_D,16,16,"nurples/src/rgba2/explosion_d.rgba2"
	LOADBMPBUFFER2 BUF_EXPLOSION_E,16,16,"nurples/src/rgba2/explosion_e.rgba2"
	LOADBMPBUFFER2 BUF_FIREBALL_A,7,7,"nurples/src/rgba2/fireball_a.rgba2"
	LOADBMPBUFFER2 BUF_FIREBALL_B,7,7,"nurples/src/rgba2/fireball_b.rgba2"
	LOADBMPBUFFER2 BUF_LASER_A,5,13,"nurples/src/rgba2/laser_a.rgba2"
	LOADBMPBUFFER2 BUF_LASER_B,5,13,"nurples/src/rgba2/laser_b.rgba2"
	LOADBMPBUFFER2 BUF_PAD,16,16,"nurples/src/rgba2/pad.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_000,16,16,"nurples/src/rgba2/seeker_000.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_001,16,16,"nurples/src/rgba2/seeker_001.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_002,16,16,"nurples/src/rgba2/seeker_002.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_003,16,16,"nurples/src/rgba2/seeker_003.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_004,16,16,"nurples/src/rgba2/seeker_004.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_005,16,16,"nurples/src/rgba2/seeker_005.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_006,16,16,"nurples/src/rgba2/seeker_006.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_007,16,16,"nurples/src/rgba2/seeker_007.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_008,16,16,"nurples/src/rgba2/seeker_008.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_009,16,16,"nurples/src/rgba2/seeker_009.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_010,16,16,"nurples/src/rgba2/seeker_010.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_011,16,16,"nurples/src/rgba2/seeker_011.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_012,16,16,"nurples/src/rgba2/seeker_012.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_013,16,16,"nurples/src/rgba2/seeker_013.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_014,16,16,"nurples/src/rgba2/seeker_014.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_015,16,16,"nurples/src/rgba2/seeker_015.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_016,16,16,"nurples/src/rgba2/seeker_016.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_017,16,16,"nurples/src/rgba2/seeker_017.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_018,16,16,"nurples/src/rgba2/seeker_018.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_019,16,16,"nurples/src/rgba2/seeker_019.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_020,16,16,"nurples/src/rgba2/seeker_020.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_021,16,16,"nurples/src/rgba2/seeker_021.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_022,16,16,"nurples/src/rgba2/seeker_022.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_023,16,16,"nurples/src/rgba2/seeker_023.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_024,16,16,"nurples/src/rgba2/seeker_024.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_025,16,16,"nurples/src/rgba2/seeker_025.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_026,16,16,"nurples/src/rgba2/seeker_026.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_027,16,16,"nurples/src/rgba2/seeker_027.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_028,16,16,"nurples/src/rgba2/seeker_028.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_029,16,16,"nurples/src/rgba2/seeker_029.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_030,16,16,"nurples/src/rgba2/seeker_030.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_031,16,16,"nurples/src/rgba2/seeker_031.rgba2"
	LOADBMPBUFFER2 BUF_SHIP_0L,16,16,"nurples/src/rgba2/ship_0l.rgba2"
	LOADBMPBUFFER2 BUF_SHIP_1C,16,16,"nurples/src/rgba2/ship_1c.rgba2"
	LOADBMPBUFFER2 BUF_SHIP_2R,16,16,"nurples/src/rgba2/ship_2r.rgba2"
	LOADBMPBUFFER2 BUF_SHIP_SMALL,8,8,"nurples/src/rgba2/ship_small.rgba2"
	LOADBMPBUFFER2 BUF_STAR,5,5,"nurples/src/rgba2/star.rgba2"
	LOADBMPBUFFER2 BUF_TURRET,16,16,"nurples/src/rgba2/turret.rgba2"
	LOADBMPBUFFER2 BUF_TURRET_ROT,16,16,"nurples/src/rgba2/turret_rot.rgba2"
@end:
