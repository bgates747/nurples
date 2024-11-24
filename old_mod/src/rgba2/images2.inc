; Bitmap indices:
BMP_0TILE_EMPTY: equ $00
BMP_1TILE_CROSS: equ $01
BMP_2TILE_HORIZ: equ $02
BMP_3TILE_VERT: equ $03
BMP_4TILE_SQUARE: equ $04
BMP_5TILE_CIRCLE: equ $05
BMP_6TILE_PAD: equ $06
BMP_7TILE_TURRET: equ $07
BMP_CIRCLE: equ $08
BMP_CRATER: equ $09
BMP_EXPLOSION_A: equ $0A
BMP_EXPLOSION_B: equ $0B
BMP_EXPLOSION_C: equ $0C
BMP_EXPLOSION_D: equ $0D
BMP_EXPLOSION_E: equ $0E
BMP_FIREBALL_A: equ $0F
BMP_FIREBALL_B: equ $10
BMP_LASER_A: equ $11
BMP_LASER_B: equ $12
BMP_PAD: equ $13
BMP_SEEKER_000: equ $14
BMP_SEEKER_001: equ $15
BMP_SEEKER_002: equ $16
BMP_SEEKER_003: equ $17
BMP_SEEKER_004: equ $18
BMP_SEEKER_005: equ $19
BMP_SEEKER_006: equ $1A
BMP_SEEKER_007: equ $1B
BMP_SEEKER_008: equ $1C
BMP_SEEKER_009: equ $1D
BMP_SEEKER_010: equ $1E
BMP_SEEKER_011: equ $1F
BMP_SEEKER_012: equ $20
BMP_SEEKER_013: equ $21
BMP_SEEKER_014: equ $22
BMP_SEEKER_015: equ $23
BMP_SHIP_0L: equ $24
BMP_SHIP_1C: equ $25
BMP_SHIP_2R: equ $26
BMP_SHIP_SMALL: equ $27
BMP_STAR: equ $28
BMP_TURRET: equ $29
BMP_TURRET_ROT: equ $2A

; import .rgba bitmap files and load them into VDP buffers
bmp2_init:
	ld hl, @cmd
	ld bc, @end-@cmd
	rst.lil $18
	ret
@cmd:
	LOADBMP BMP_0TILE_EMPTY,16,16,"../rgba2/0tile_empty.rgba2"
	LOADBMP BMP_1TILE_CROSS,16,16,"../rgba2/1tile_cross.rgba2"
	LOADBMP BMP_2TILE_HORIZ,16,16,"../rgba2/2tile_horiz.rgba2"
	LOADBMP BMP_3TILE_VERT,16,16,"../rgba2/3tile_vert.rgba2"
	LOADBMP BMP_4TILE_SQUARE,16,16,"../rgba2/4tile_square.rgba2"
	LOADBMP BMP_5TILE_CIRCLE,16,16,"../rgba2/5tile_circle.rgba2"
	LOADBMP BMP_6TILE_PAD,16,16,"../rgba2/6tile_pad.rgba2"
	LOADBMP BMP_7TILE_TURRET,16,16,"../rgba2/7tile_turret.rgba2"
	LOADBMP BMP_CIRCLE,16,16,"../rgba2/circle.rgba2"
	LOADBMP BMP_CRATER,16,16,"../rgba2/crater.rgba2"
	LOADBMP BMP_EXPLOSION_A,16,16,"../rgba2/explosion_a.rgba2"
	LOADBMP BMP_EXPLOSION_B,16,16,"../rgba2/explosion_b.rgba2"
	LOADBMP BMP_EXPLOSION_C,16,16,"../rgba2/explosion_c.rgba2"
	LOADBMP BMP_EXPLOSION_D,16,16,"../rgba2/explosion_d.rgba2"
	LOADBMP BMP_EXPLOSION_E,16,16,"../rgba2/explosion_e.rgba2"
	LOADBMP BMP_FIREBALL_A,7,7,"../rgba2/fireball_a.rgba2"
	LOADBMP BMP_FIREBALL_B,7,7,"../rgba2/fireball_b.rgba2"
	LOADBMP BMP_LASER_A,5,13,"../rgba2/laser_a.rgba2"
	LOADBMP BMP_LASER_B,5,13,"../rgba2/laser_b.rgba2"
	LOADBMP BMP_PAD,16,16,"../rgba2/pad.rgba2"
	LOADBMP BMP_SEEKER_000,16,16,"../rgba2/seeker_000.rgba2"
	LOADBMP BMP_SEEKER_001,16,16,"../rgba2/seeker_001.rgba2"
	LOADBMP BMP_SEEKER_002,16,16,"../rgba2/seeker_002.rgba2"
	LOADBMP BMP_SEEKER_003,16,16,"../rgba2/seeker_003.rgba2"
	LOADBMP BMP_SEEKER_004,16,16,"../rgba2/seeker_004.rgba2"
	LOADBMP BMP_SEEKER_005,16,16,"../rgba2/seeker_005.rgba2"
	LOADBMP BMP_SEEKER_006,16,16,"../rgba2/seeker_006.rgba2"
	LOADBMP BMP_SEEKER_007,16,16,"../rgba2/seeker_007.rgba2"
	LOADBMP BMP_SEEKER_008,16,16,"../rgba2/seeker_008.rgba2"
	LOADBMP BMP_SEEKER_009,16,16,"../rgba2/seeker_009.rgba2"
	LOADBMP BMP_SEEKER_010,16,16,"../rgba2/seeker_010.rgba2"
	LOADBMP BMP_SEEKER_011,16,16,"../rgba2/seeker_011.rgba2"
	LOADBMP BMP_SEEKER_012,16,16,"../rgba2/seeker_012.rgba2"
	LOADBMP BMP_SEEKER_013,16,16,"../rgba2/seeker_013.rgba2"
	LOADBMP BMP_SEEKER_014,16,16,"../rgba2/seeker_014.rgba2"
	LOADBMP BMP_SEEKER_015,16,16,"../rgba2/seeker_015.rgba2"
	LOADBMP BMP_SHIP_0L,16,16,"../rgba2/ship_0l.rgba2"
	LOADBMP BMP_SHIP_1C,16,16,"../rgba2/ship_1c.rgba2"
	LOADBMP BMP_SHIP_2R,16,16,"../rgba2/ship_2r.rgba2"
	LOADBMP BMP_SHIP_SMALL,8,8,"../rgba2/ship_small.rgba2"
	LOADBMP BMP_STAR,5,5,"../rgba2/star.rgba2"
	LOADBMP BMP_TURRET,16,16,"../rgba2/turret.rgba2"
	LOADBMP BMP_TURRET_ROT,16,16,"../rgba2/turret_rot.rgba2"
@end:
