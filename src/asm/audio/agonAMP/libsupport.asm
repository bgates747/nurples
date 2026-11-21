; libsupport.asm - small routines shared by the agonAMP sound library.
;
; Copyright (c) 2026 Nick J. Date
;
; This software is provided 'as-is', without any express or implied
; warranty. In no event will the authors be held liable for any damages
; arising from the use of this software.
;
; Permission is granted to anyone to use this software for any purpose,
; including commercial applications, and to alter it and redistribute it
; freely, subject to the following restrictions:
;
; 1. The origin of this software must not be misrepresented; you must not
;    claim that you wrote the original software. If you use this software
;    in a product, an acknowledgment in the product documentation would be
;    appreciated but is not required.
; 2. Altered source versions must be plainly marked as such, and must not
;    be misrepresented as being the original software.
; 3. This notice may not be removed or altered from any source
;    distribution.
;
; Part of the reusable library in lib/. It provides:
;
;   vdu_put    queue one byte for the VDP
;   vdu_flush  send the queued bytes to the VDP
;   lib_getw   fetch a 16 bit value from memory
;   lib_div24  24 bit unsigned division
;
; Everything sent to the VDP goes through a small buffer and is passed
; to MOS in blocks (RST 18h), which is much faster than one MOS call per
; byte and keeps each VDU command sequence together. A program using the
; library may send its own VDU output through vdu_put as well, or use
; MOS directly, as long as it never sends bytes from an interrupt while
; a sequence is part-way through.

lib_divnum:	ds	3
vdu_ptr:	dl	vdu_buf
vdu_buf:	ds	256

; vdu_put: queue one byte for the VDP. Preserves all registers except F.
vdu_put:
	push	hl
	ld	hl,(vdu_ptr)
	ld	(hl),a
	inc	hl
	ld	(vdu_ptr),hl
	push	de
	ld	de,vdu_buf+240
	or	a
	sbc	hl,de
	pop	de
	pop	hl
	ret	c
	; buffer nearly full: fall through and send it

; vdu_flush: send everything queued so far. Preserves all registers.
vdu_flush:
	push	af
	push	bc
	push	de
	push	hl
	push	ix
	push	iy
	ld	hl,(vdu_ptr)
	ld	de,vdu_buf
	or	a
	sbc	hl,de
	jr	z,@empty
	push	hl
	pop	bc
	ld	hl,vdu_buf
	ld	(vdu_ptr),hl
	rst.lil	18h
@empty:
	pop	iy
	pop	ix
	pop	hl
	pop	de
	pop	bc
	pop	af
	ret

; lib_getw: fetch a 16 bit little-endian value.
; In:  HL = address. Out: HL = value, zero extended. Preserves BC, DE.
lib_getw:
	push	de
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	ld	hl,0
	ld	l,e
	ld	h,d
	pop	de
	ret

; lib_div24: unsigned division. In: HL = dividend, DE = divisor (below
; 800000h). Out: HL = quotient. Preserves BC, DE.
lib_div24:
	push	bc
	ld	(lib_divnum),hl
	ld	hl,0
	ld	b,24
@bit:
	push	hl
	ld	hl,(lib_divnum)
	add	hl,hl			; top bit of the dividend into carry
	ld	(lib_divnum),hl
	pop	hl
	adc	hl,hl			; and on into the remainder
	sbc	hl,de
	jr	nc,@fits
	add	hl,de
	jr	@next
@fits:
	push	hl
	ld	hl,lib_divnum
	inc	(hl)			; set this bit of the quotient
	pop	hl
@next:
	djnz	@bit
	ld	hl,(lib_divnum)
	pop	bc
	ret