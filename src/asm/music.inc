; This file is created by build_98_asm_sfx.py, do not edit it!

SFX_num_buffers: equ 2
; SFX buffer ids:
BUF_AMBIENT_BEAT70: equ 0x3000
BUF_SPACE_ADVENTURE: equ 0x3001

; SFX buffer id reverse lookup:
SFX_buffer_id_lut:
	dl BUF_AMBIENT_BEAT70
	dl BUF_SPACE_ADVENTURE

; SFX duration lookup:
SFX_duration_lut:
	dl 9762 ; AMBIENT_BEAT70
	dl 9758 ; SPACE_ADVENTURE

; SFX load routines jump table:
SFX_load_routines_table:
	dl load_sfx_AMBIENT_BEAT70
	dl load_sfx_SPACE_ADVENTURE

; Import sfx .raw files and load them into VDP buffers

load_sfx_AMBIENT_BEAT70:
	ld hl,FAMBIENT_BEAT70
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_AMBIENT_BEAT70
	ld ix,163968
	ld iy,FAMBIENT_BEAT70
	call vdu_load_sfx
	ret

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
FAMBIENT_BEAT70: db "music/AMBIENT_BEAT70.raw",0
FSPACE_ADVENTURE: db "music/SPACE_ADVENTURE.raw",0

; Play sfx routines

sfx_play_ambient_beat70:
	ld hl,BUF_AMBIENT_BEAT70
	ld bc,9762
	jp vdu_play_sfx

sfx_play_space_adventure:
	ld hl,BUF_SPACE_ADVENTURE
	ld bc,9758
	jp vdu_play_sfx
