; This file is created by build_98_asm_sfx.py, do not edit it!

SFX_num_buffers: equ 4
; SFX buffer ids:
BUF_AFRICA: equ 0x3000
BUF_COME_UNDONE: equ 0x3001
BUF_EVERY_BREATH_YOU_TAKE: equ 0x3002
BUF_RHIANNON: equ 0x3003

; SFX buffer id reverse lookup:
SFX_buffer_id_lut:
	dl BUF_AFRICA
	dl BUF_COME_UNDONE
	dl BUF_EVERY_BREATH_YOU_TAKE
	dl BUF_RHIANNON

; SFX duration lookup:
SFX_duration_lut:
	dl 29268 ; AFRICA
	dl 29268 ; COME_UNDONE
	dl 29268 ; EVERY_BREATH_YOU_TAKE
	dl 29268 ; RHIANNON

; SFX load routines jump table:
SFX_load_routines_table:
	dl load_sfx_AFRICA
	dl load_sfx_COME_UNDONE
	dl load_sfx_EVERY_BREATH_YOU_TAKE
	dl load_sfx_RHIANNON

; Import sfx .raw files and load them into VDP buffers

load_sfx_AFRICA:
	ld hl,FAFRICA
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_AFRICA
	ld ix,491536
	ld iy,FAFRICA
	call vdu_load_sfx
	ret

load_sfx_COME_UNDONE:
	ld hl,FCOME_UNDONE
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_COME_UNDONE
	ld ix,491536
	ld iy,FCOME_UNDONE
	call vdu_load_sfx
	ret

load_sfx_EVERY_BREATH_YOU_TAKE:
	ld hl,FEVERY_BREATH_YOU_TAKE
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_EVERY_BREATH_YOU_TAKE
	ld ix,491536
	ld iy,FEVERY_BREATH_YOU_TAKE
	call vdu_load_sfx
	ret

load_sfx_RHIANNON:
	ld hl,FRHIANNON
	ld de,filedata
	ld bc,65536
	ld a,mos_load
	RST.LIL 08h
	ld hl,BUF_RHIANNON
	ld ix,491536
	ld iy,FRHIANNON
	call vdu_load_sfx
	ret

; File name lookups:
FAFRICA: db "music/AFRICA.raw",0
FCOME_UNDONE: db "music/COME_UNDONE.raw",0
FEVERY_BREATH_YOU_TAKE: db "music/EVERY_BREATH_YOU_TAKE.raw",0
FRHIANNON: db "music/RHIANNON.raw",0

; Play sfx routines

sfx_play_africa:
	ld hl,BUF_AFRICA
	ld bc,29268
	jp vdu_play_sfx

sfx_play_come_undone:
	ld hl,BUF_COME_UNDONE
	ld bc,29268
	jp vdu_play_sfx

sfx_play_every_breath_you_take:
	ld hl,BUF_EVERY_BREATH_YOU_TAKE
	ld bc,29268
	jp vdu_play_sfx

sfx_play_rhiannon:
	ld hl,BUF_RHIANNON
	ld bc,29268
	jp vdu_play_sfx
