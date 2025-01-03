; This file is created by build_98_asm_sfx.py, do not edit it!

SFX_num_buffers: equ 1
; SFX buffer ids:
BUF_SPACE_ADVENTURE: equ 0x3000

; SFX buffer id reverse lookup:
SFX_buffer_id_lut:
	dl BUF_SPACE_ADVENTURE

; SFX duration lookup:
SFX_duration_lut:
	dl 9758 ; SPACE_ADVENTURE

; SFX load routines jump table:
SFX_load_routines_table:
	dl load_sfx_SPACE_ADVENTURE

; Import sfx .raw files and load them into VDP buffers

load_sfx_SPACE_ADVENTURE:
	ld hl,FSPACE_ADVENTURE
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_SPACE_ADVENTURE
	ld ix,163884
	ld iy,FSPACE_ADVENTURE
	call vdu_load_sfx
	ret

; File name lookups:
FSPACE_ADVENTURE: db "music/SPACE_ADVENTURE.raw",0

; Play sfx routines

sfx_play_space_adventure:
	ld hl,BUF_SPACE_ADVENTURE
	ld bc,9758
	jp vdu_play_sfx
