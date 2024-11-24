; Bitmap indices:
BUF_0TILE_EMPTY: equ $0100
BUF_0TILE_CIRCLE: equ $0101
BUF_0TILE_CROSS: equ $0102
BUF_0TILE_EMPTY: equ $0103
BUF_0TILE_HORIZ: equ $0104
BUF_0TILE_PAD: equ $0105
BUF_0TILE_TURRET: equ $0106
BUF_0TILE_VERT: equ $0107
BUF_EXPLOSION_A: equ $0108
BUF_EXPLOSION_B: equ $0109
BUF_EXPLOSION_C: equ $010A
BUF_EXPLOSION_D: equ $010B
BUF_EXPLOSION_E: equ $010C
BUF_FIREBALL_A: equ $010D
BUF_FIREBALL_B: equ $010E
BUF_LASER_A: equ $010F
BUF_LASER_B: equ $0110
BUF_SEEKER: equ $0111
BUF_SHIP_0L: equ $0112
BUF_SHIP_1C: equ $0113
BUF_SHIP_2R: equ $0114
BUF_SHIP_SMALL: equ $0115
BUF_STAR: equ $0116
BUF_TURRET: equ $0117
BUF_TURRET_ROT: equ $0118

; import .rgba bitmap files and load them into VDP buffers
bmp8_init:
	ld hl, @cmd
	ld bc, @end-@cmd
	rst.lil $18
	ret
@cmd:
	LOADBMPBUFFER8 BUF_0TILE_EMPTY,16,16,"../rgba8/0tile_blank.rgba8"
	LOADBMPBUFFER8 BUF_0TILE_CIRCLE,16,16,"../rgba8/0tile_circle.rgba8"
	LOADBMPBUFFER8 BUF_0TILE_CROSS,16,16,"../rgba8/0tile_cross.rgba8"
	LOADBMPBUFFER8 BUF_0TILE_EMPTY,1,1,"../rgba8/0tile_empty.rgba8"
	LOADBMPBUFFER8 BUF_0TILE_HORIZ,16,16,"../rgba8/0tile_horiz.rgba8"
	LOADBMPBUFFER8 BUF_0TILE_PAD,16,16,"../rgba8/0tile_pad.rgba8"
	LOADBMPBUFFER8 BUF_0TILE_TURRET,16,16,"../rgba8/0tile_turret.rgba8"
	LOADBMPBUFFER8 BUF_0TILE_VERT,16,16,"../rgba8/0tile_vert.rgba8"
	LOADBMPBUFFER8 BUF_EXPLOSION_A,16,16,"../rgba8/explosion_a.rgba8"
	LOADBMPBUFFER8 BUF_EXPLOSION_B,16,16,"../rgba8/explosion_b.rgba8"
	LOADBMPBUFFER8 BUF_EXPLOSION_C,16,16,"../rgba8/explosion_c.rgba8"
	LOADBMPBUFFER8 BUF_EXPLOSION_D,16,16,"../rgba8/explosion_d.rgba8"
	LOADBMPBUFFER8 BUF_EXPLOSION_E,16,16,"../rgba8/explosion_e.rgba8"
	LOADBMPBUFFER8 BUF_FIREBALL_A,7,7,"../rgba8/fireball_a.rgba8"
	LOADBMPBUFFER8 BUF_FIREBALL_B,7,7,"../rgba8/fireball_b.rgba8"
	LOADBMPBUFFER8 BUF_LASER_A,5,13,"../rgba8/laser_a.rgba8"
	LOADBMPBUFFER8 BUF_LASER_B,5,13,"../rgba8/laser_b.rgba8"
	LOADBMPBUFFER8 BUF_SEEKER,16,16,"../rgba8/seeker.rgba8"
	LOADBMPBUFFER8 BUF_SHIP_0L,16,16,"../rgba8/ship_0l.rgba8"
	LOADBMPBUFFER8 BUF_SHIP_1C,16,16,"../rgba8/ship_1c.rgba8"
	LOADBMPBUFFER8 BUF_SHIP_2R,16,16,"../rgba8/ship_2r.rgba8"
	LOADBMPBUFFER8 BUF_SHIP_SMALL,8,8,"../rgba8/ship_small.rgba8"
	LOADBMPBUFFER8 BUF_STAR,5,5,"../rgba8/star.rgba8"
	LOADBMPBUFFER8 BUF_TURRET,16,16,"../rgba8/turret.rgba8"
	LOADBMPBUFFER8 BUF_TURRET_ROT,16,16,"../rgba8/turret_rot.rgba8"
@end:
