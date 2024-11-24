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
BUF_SEEKER_008: equ $0115
BUF_SEEKER_016: equ $0116
BUF_SEEKER_024: equ $0117
BUF_SEEKER_032: equ $0118
BUF_SEEKER_040: equ $0119
BUF_SEEKER_048: equ $011A
BUF_SEEKER_056: equ $011B
BUF_SEEKER_064: equ $011C
BUF_SEEKER_072: equ $011D
BUF_SEEKER_080: equ $011E
BUF_SEEKER_088: equ $011F
BUF_SEEKER_096: equ $0120
BUF_SEEKER_104: equ $0121
BUF_SEEKER_112: equ $0122
BUF_SEEKER_120: equ $0123
BUF_SEEKER_128: equ $0124
BUF_SEEKER_136: equ $0125
BUF_SEEKER_144: equ $0126
BUF_SEEKER_152: equ $0127
BUF_SEEKER_160: equ $0128
BUF_SEEKER_168: equ $0129
BUF_SEEKER_176: equ $012A
BUF_SEEKER_184: equ $012B
BUF_SEEKER_192: equ $012C
BUF_SEEKER_200: equ $012D
BUF_SEEKER_208: equ $012E
BUF_SEEKER_216: equ $012F
BUF_SEEKER_224: equ $0130
BUF_SEEKER_232: equ $0131
BUF_SEEKER_240: equ $0132
BUF_SEEKER_248: equ $0133
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
	LOADBMPBUFFER2 BUF_0TILE_EMPTY,16,16,"../../tgt/sprites/0tile_empty.rgba2"
	LOADBMPBUFFER2 BUF_1TILE_CROSS,16,16,"../../tgt/sprites/1tile_cross.rgba2"
	LOADBMPBUFFER2 BUF_2TILE_HORIZ,16,16,"../../tgt/sprites/2tile_horiz.rgba2"
	LOADBMPBUFFER2 BUF_3TILE_VERT,16,16,"../../tgt/sprites/3tile_vert.rgba2"
	LOADBMPBUFFER2 BUF_4TILE_SQUARE,16,16,"../../tgt/sprites/4tile_square.rgba2"
	LOADBMPBUFFER2 BUF_5TILE_CIRCLE,16,16,"../../tgt/sprites/5tile_circle.rgba2"
	LOADBMPBUFFER2 BUF_6TILE_PAD,16,16,"../../tgt/sprites/6tile_pad.rgba2"
	LOADBMPBUFFER2 BUF_7TILE_TURRET,16,16,"../../tgt/sprites/7tile_turret.rgba2"
	LOADBMPBUFFER2 BUF_CIRCLE,16,16,"../../tgt/sprites/circle.rgba2"
	LOADBMPBUFFER2 BUF_CRATER,16,16,"../../tgt/sprites/crater.rgba2"
	LOADBMPBUFFER2 BUF_EXPLOSION_A,16,16,"../../tgt/sprites/explosion_a.rgba2"
	LOADBMPBUFFER2 BUF_EXPLOSION_B,16,16,"../../tgt/sprites/explosion_b.rgba2"
	LOADBMPBUFFER2 BUF_EXPLOSION_C,16,16,"../../tgt/sprites/explosion_c.rgba2"
	LOADBMPBUFFER2 BUF_EXPLOSION_D,16,16,"../../tgt/sprites/explosion_d.rgba2"
	LOADBMPBUFFER2 BUF_EXPLOSION_E,16,16,"../../tgt/sprites/explosion_e.rgba2"
	LOADBMPBUFFER2 BUF_FIREBALL_A,7,7,"../../tgt/sprites/fireball_a.rgba2"
	LOADBMPBUFFER2 BUF_FIREBALL_B,7,7,"../../tgt/sprites/fireball_b.rgba2"
	LOADBMPBUFFER2 BUF_LASER_A,5,13,"../../tgt/sprites/laser_a.rgba2"
	LOADBMPBUFFER2 BUF_LASER_B,5,13,"../../tgt/sprites/laser_b.rgba2"
	LOADBMPBUFFER2 BUF_PAD,16,16,"../../tgt/sprites/pad.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_000,16,16,"../../tgt/sprites/seeker_000.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_008,16,16,"../../tgt/sprites/seeker_008.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_016,16,16,"../../tgt/sprites/seeker_016.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_024,16,16,"../../tgt/sprites/seeker_024.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_032,16,16,"../../tgt/sprites/seeker_032.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_040,16,16,"../../tgt/sprites/seeker_040.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_048,16,16,"../../tgt/sprites/seeker_048.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_056,16,16,"../../tgt/sprites/seeker_056.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_064,16,16,"../../tgt/sprites/seeker_064.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_072,16,16,"../../tgt/sprites/seeker_072.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_080,16,16,"../../tgt/sprites/seeker_080.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_088,16,16,"../../tgt/sprites/seeker_088.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_096,16,16,"../../tgt/sprites/seeker_096.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_104,16,16,"../../tgt/sprites/seeker_104.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_112,16,16,"../../tgt/sprites/seeker_112.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_120,16,16,"../../tgt/sprites/seeker_120.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_128,16,16,"../../tgt/sprites/seeker_128.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_136,16,16,"../../tgt/sprites/seeker_136.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_144,16,16,"../../tgt/sprites/seeker_144.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_152,16,16,"../../tgt/sprites/seeker_152.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_160,16,16,"../../tgt/sprites/seeker_160.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_168,16,16,"../../tgt/sprites/seeker_168.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_176,16,16,"../../tgt/sprites/seeker_176.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_184,16,16,"../../tgt/sprites/seeker_184.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_192,16,16,"../../tgt/sprites/seeker_192.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_200,16,16,"../../tgt/sprites/seeker_200.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_208,16,16,"../../tgt/sprites/seeker_208.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_216,16,16,"../../tgt/sprites/seeker_216.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_224,16,16,"../../tgt/sprites/seeker_224.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_232,16,16,"../../tgt/sprites/seeker_232.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_240,16,16,"../../tgt/sprites/seeker_240.rgba2"
	LOADBMPBUFFER2 BUF_SEEKER_248,16,16,"../../tgt/sprites/seeker_248.rgba2"
	LOADBMPBUFFER2 BUF_SHIP_0L,16,16,"../../tgt/sprites/ship_0l.rgba2"
	LOADBMPBUFFER2 BUF_SHIP_1C,16,16,"../../tgt/sprites/ship_1c.rgba2"
	LOADBMPBUFFER2 BUF_SHIP_2R,16,16,"../../tgt/sprites/ship_2r.rgba2"
	LOADBMPBUFFER2 BUF_SHIP_SMALL,8,8,"../../tgt/sprites/ship_small.rgba2"
	LOADBMPBUFFER2 BUF_STAR,5,5,"../../tgt/sprites/star.rgba2"
	; LOADBMPBUFFER2 BUF_TURRET,16,16,"../../tgt/sprites/turret.rgba2"
	; LOADBMPBUFFER2 BUF_TURRET_ROT,16,16,"../../tgt/sprites/turret_rot.rgba2"
@end:
