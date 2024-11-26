; Bitmap indices:
BUF_LEVEL_00: equ $8000

; import .rgba bitmap files and load them into VDP buffers
bmp2_init:
	ld hl, @cmd
	ld bc, @end-@cmd
	rst.lil $18
	ret
@cmd:
	LOADBMPBUFFER2 BUF_LEVEL_00,256,4096,"../rgba2/level_00.rgba2"
@end:
