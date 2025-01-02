; This file is created by build_98_asm_sfx.py, do not edit it!

SFX_num_buffers: equ 2
; SFX buffer ids:
BUF_SPACE_ADVENTURE: equ 0x3000
BUF_SPACE_ADVENTURE_MOD: equ 0x3001

; SFX buffer id reverse lookup:
SFX_buffer_id_lut:
	dl BUF_SPACE_ADVENTURE
	dl BUF_SPACE_ADVENTURE_MOD

; SFX duration lookup:
SFX_duration_lut:
	dl 127710 ; SPACE_ADVENTURE
	dl 9755 ; SPACE_ADVENTURE_MOD

; SFX load routines jump table:
SFX_load_routines_table:
	dl load_sfx_SPACE_ADVENTURE
	dl load_sfx_SPACE_ADVENTURE_MOD

; Import sfx .raw files and load them into VDP buffers

load_sfx_SPACE_ADVENTURE:
	ld hl,FSPACE_ADVENTURE
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_SPACE_ADVENTURE
	ld ix,2144731
	ld iy,FSPACE_ADVENTURE
	call vdu_load_sfx
	ret

load_sfx_SPACE_ADVENTURE_MOD:
	ld hl,FSPACE_ADVENTURE_MOD
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_SPACE_ADVENTURE_MOD
	ld ix,163840
	ld iy,FSPACE_ADVENTURE_MOD
	call vdu_load_sfx
	ret

; File name lookups:
FSPACE_ADVENTURE: db "music/SPACE_ADVENTURE.raw",0
FSPACE_ADVENTURE_MOD: db "music/SPACE_ADVENTURE_MOD.raw",0

; Play sfx routines

sfx_play_space_adventure:
	ld hl,BUF_SPACE_ADVENTURE
	ld bc,127710
	jp vdu_play_sfx

sfx_play_space_adventure_mod:
	ld hl,BUF_SPACE_ADVENTURE_MOD
	ld bc,9755
	jp vdu_play_sfx
