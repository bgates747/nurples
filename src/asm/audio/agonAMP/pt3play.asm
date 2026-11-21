; pt3play.asm - Pro Tracker 3 / Vortex Tracker II module replayer.
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
; Part of the reusable agonAMP sound library in lib/. It turns a PT3
; module held in memory into AY-3-8912 register values, one set per
; 50 Hz frame. It knows nothing about the Agon's sound hardware:
; ayvdp.asm converts the registers into VDP audio commands.
;
; It was written from scratch for the eZ80 in ADL mode, working from the
; published description of the format ("How to decode a Vortex Tracker II
; PT3 File", V. Weaver, 2019). No code was copied from another player.
;
; Needs: libsupport.asm (lib_getw), pt3tab.inc (tone and volume tables)
; and the 16 byte table ay_regs, which ayvdp.asm defines. A program that
; uses the replayer without ayvdp.asm must define ay_regs itself.
;
; Entry points:
;   pt3_init   HL = address of a module: get ready to play it
;   pt3_init_m the same, with A = the number of patterns if the module
;              is to be played with its patterns mirrored, as the second
;              chip of a one-module TurboSound tune is (see pt3ts.asm)
;   pt3_play   advance one frame and fill ay_regs
;   pt3_length frames the song lasts (see the routine for how to use it)
;
; Results:
;   ay_regs    14 bytes, AY registers R0 to R13. R13 (envelope shape) is
;              0FFh on frames where the shape was not written, because a
;              write to R13 restarts the envelope even with the same value.
;   pt3_loops  counts how often the end of the song has been reached; a
;              program can watch it (and zero it) to stop or move on.
;   pt3_patgen changes whenever a pattern starts (including the first).
;   pt3_pos, pt3_row and the PT3C_ fields of pt3_chA/B/C can be read for
;   display.
;
; All registers are used. All 16 bit quantities are kept in 3 byte slots
; so that 24 bit loads and stores can be used on them freely.
;
; Everything the replayer remembers lies between the labels pt3_ctx and
; pt3_ctx_end, so a program can play two modules at once by keeping a
; copy of that block for each and copying the right one in before each
; call. pt3ts.asm does this for TurboSound tunes.

; ---------------------------------------------------------------------
; Per-channel state, addressed through IX.
; ---------------------------------------------------------------------
	; 3: address of next byte of pattern data
PT3C_ADDR:		equ	0
PT3C_ORNPTR:	equ	3	; 3: address of ornament values
PT3C_SAMPTR:	equ	6	; 3: address of sample records
	; 3: tone period sent to the AY (12 bits)
PT3C_TON:		equ	9
PT3C_TSLIDE:	equ	12	; 3: current tone slide (signed)
PT3C_TSSTEP:	equ	15	; 3: tone slide step (signed)
PT3C_TDELTA:	equ	18	; 3: portamento distance (signed)
PT3C_TACC:		equ	21	; 3: tone accumulator
PT3C_ORNLOOP:	equ	24	; 1: ornament loop position
PT3C_ORNLEN:	equ	25	; 1: ornament length
PT3C_ORNPOS:	equ	26	; 1: position in ornament
PT3C_SAMLOOP:	equ	27	; 1: sample loop position
PT3C_SAMLEN:	equ	28	; 1: sample length
PT3C_SAMPOS:	equ	29	; 1: position in sample
PT3C_VOL:		equ	30	; 1: channel volume, 0 to 15
PT3C_SKIPN:	equ	31	; 1: rows between notes
PT3C_SKIPCNT:	equ	32	; 1: rows left until next note
PT3C_NOTE:		equ	33	; 1: note number, 0 to 95
PT3C_SLIDETO:	equ	34	; 1: portamento target note
	; 1: amplitude sent to the AY (bit 4 = envelope)
PT3C_AMP:		equ	35
PT3C_ENVEN:	equ	36	; 1: non-zero if the envelope is in use
PT3C_ENABLED:	equ	37	; 1: non-zero if the channel is sounding
PT3C_GLISS:	equ	38	; 1: non-zero for a plain slide, 0 = portamento
PT3C_AMPSL:	equ	39	; 1: amplitude slide (signed)
PT3C_NOISESL:	equ	40	; 1: noise slide
PT3C_ENVSL:	equ	41	; 1: envelope slide
PT3C_TSCNT:	equ	42	; 1: frames until next tone slide step
PT3C_TSDELAY:	equ	43	; 1: frames between tone slide steps
PT3C_ONOFF:	equ	44	; 1: frames until on/off toggle, 0 = unused
PT3C_ONOFFD:	equ	45	; 1: frames to stay on
PT3C_OFFOND:	equ	46	; 1: frames to stay off
PT3C_ORNNUM:	equ	47	; 1: ornament number (for display)
PT3C_SAMNUM:	equ	48	; 1: sample number (for display)
PT3C_SIZE:		equ	49

; Offsets into the module header.
PT3_VERSION:	equ	13	; ASCII digit of the tracker sub-version
PT3_TABLE:	equ	99	; tone table number
PT3_DELAY:	equ	100	; frames per row
PT3_NUMPOS:	equ	101	; number of positions
PT3_LOOPPOS:	equ	102	; position to loop back to
PT3_PATPTR:	equ	103	; word: offset of the pattern pointer table
PT3_SAMPTRS:	equ	105	; 32 words: offsets of the samples
PT3_ORNPTRS:	equ	169	; 16 words: offsets of the ornaments
PT3_POSLIST:	equ	201	; position list, each byte is pattern * 3

; ---------------------------------------------------------------------
; Variables. Everything between pt3_vars and pt3_vars_end is cleared by
; pt3_init.
; ---------------------------------------------------------------------
pt3_ctx:
pt3_notetab:		ds	3	; address of the tone table in use
pt3_voltab:		ds	3	; address of the volume table in use
pt3_base:		ds	3	; address of the module being played
pt3_patgen:		ds	1	; goes up whenever a new pattern starts
pt3_mirror:		ds	1	; patterns in a mirrored module, else 0
pt3_vars:
pt3_chA:			ds	PT3C_SIZE
pt3_chB:			ds	PT3C_SIZE
pt3_chC:			ds	PT3C_SIZE
pt3_envbase:		ds	3	; envelope period set by the pattern
pt3_envslide:		ds	3	; accumulated envelope slide
pt3_envslide_add:	ds	3	; envelope slide step
pt3_prslide:		ds	3	; tone slide before the current row
pt3_envdelay:		ds	1	; frames between envelope slide steps
pt3_envdelay_cur:	ds	1	; frames until the next step
pt3_noise_base:		ds	1	; noise period set by the pattern
pt3_addnoise:		ds	1	; noise offset from the samples
pt3_addenv:		ds	1	; envelope offset from the samples
pt3_mixer:		ds	1	; mixer value being assembled
pt3_delay:		ds	1	; frames per row
pt3_delay_cnt:		ds	1	; frames until the next row
pt3_pos:		ds	1	; current position
pt3_numpos:		ds	1	; number of positions
pt3_looppos:		ds	1	; loop position
pt3_patx3:		ds	1	; current pattern number * 3
pt3_row:		ds	1	; row within the pattern (for display)
pt3_frames:		ds	3	; pt3_length: frames counted
pt3_loops:		ds	1	; times the end of the song was reached
pt3_version:		ds	1	; tracker sub-version
pt3_prnote:		ds	1	; note before the current row
pt3_fx:			ds	10	; when each effect was announced
pt3_fxcount:		ds	1	; effects announced on this row
pt3_vars_end:
pt3_ctx_end:
pt3_rows:	ds	3	; length: rows counted
pt3_rowlim:	ds	3	; bytes left in this pattern row

; ---------------------------------------------------------------------
; pt3_init: prepare to play a module from the beginning.
; In: HL = address of the module, which may be anywhere in memory and
; must stay there while it is being played.
; ---------------------------------------------------------------------
pt3_init:
	xor	a
pt3_init_m:
	ld	(pt3_mirror),a
	ld	(pt3_base),hl
	ld	hl,pt3_vars		; clear all replayer state
	ld	de,pt3_vars+1
	ld	bc,pt3_vars_end-pt3_vars-1
	ld	(hl),0
	ldir
	ld	ix,(pt3_base)
	ld	a,(ix+PT3_VERSION)	; sub-version digit, default 6
	sub	'0'
	cp	10
	jr	c,@gotver
	ld	a,6
@gotver:
	ld	(pt3_version),a
	; Choose the tone table. Tables 0, 2 and 3 each exist in an old
	; form (sub-version 3 and below) and a new form.
	ld	a,(ix+PT3_TABLE)
	ld	b,a
	ld	hl,pt3_nt_st
	cp	1
	jr	z,@gottab
	ld	a,(pt3_version)
	cp	4
	ld	a,b
	jr	c,@oldtab
	ld	hl,pt3_nt_pt_34
	or	a
	jr	z,@gottab
	ld	hl,pt3_nt_asm_34
	cp	2
	jr	z,@gottab
	ld	hl,pt3_nt_real_34
	jr	@gottab
@oldtab:
	ld	hl,pt3_nt_pt_33
	or	a
	jr	z,@gottab
	ld	hl,pt3_nt_asm_33
	cp	2
	jr	z,@gottab
	ld	hl,pt3_nt_real_33
@gottab:
	ld	(pt3_notetab),hl
	ld	hl,pt3_vt_35		; volume table also depends on version
	ld	a,(pt3_version)
	cp	5
	jr	nc,@gotvol
	ld	hl,pt3_vt_33
@gotvol:
	ld	(pt3_voltab),hl
	ld	a,(ix+PT3_DELAY)
	ld	(pt3_delay),a
	ld	a,1
	ld	(pt3_delay_cnt),a
	ld	a,(ix+PT3_NUMPOS)
	ld	(pt3_numpos),a
	ld	a,(ix+PT3_LOOPPOS)
	ld	(pt3_looppos),a
	ld	a,0FFh			; becomes row 0 on the first frame
	ld	(pt3_row),a
	ld	ix,pt3_chA
	call	pt3_chinit
	ld	ix,pt3_chB
	call	pt3_chinit
	ld	ix,pt3_chC
	call	pt3_chinit
	xor	a
	; fall through to start at position 0

; ---------------------------------------------------------------------
; pt3_setpos: point the three channels at the patterns for a position.
; In: A = position number. Corrupts IX.
; ---------------------------------------------------------------------
pt3_setpos:
	ld	(pt3_pos),a
	ld	hl,pt3_patgen		; tell anyone displaying the pattern
	inc	(hl)
	ld	hl,0
	ld	l,a
	ld	de,PT3_POSLIST
	add	hl,de
	ld	de,(pt3_base)
	add	hl,de
	ld	c,(hl)			; pattern number * 3
	ld	a,(pt3_mirror)		; mirrored: pattern (count - 1 - n)
	or	a
	jr	z,@plain
	ld	b,a
	add	a,a
	add	a,b
	sub	3
	sub	c
	ld	c,a
@plain:
	ld	a,c
	ld	(pt3_patx3),a
	ld	hl,0
	ld	l,a
	add	hl,hl			; * 6 = offset into pointer table
	ex	de,hl
	push	de
	ld	hl,(pt3_base)
	ld	de,PT3_PATPTR
	add	hl,de
	pop	de
	call	lib_getw
	add	hl,de
	ld	de,(pt3_base)
	add	hl,de			; HL -> three pattern offsets
	ld	ix,pt3_chA
	call	pt3_setaddr
	ld	ix,pt3_chB
	call	pt3_setaddr
	ld	ix,pt3_chC
pt3_setaddr:
	push	hl
	call	lib_getw
	ld	de,(pt3_base)
	add	hl,de
	ld	(ix+PT3C_ADDR),hl
	pop	hl
	inc	hl
	inc	hl
	ret

; Give a channel its starting values: ornament 0, sample 1, volume 15.
pt3_chinit:
	xor	a
	call	pt3_setorn
	ld	a,1
	call	pt3_setsam
	ld	(ix+PT3C_VOL),15
	ld	(ix+PT3C_SKIPN),1
	ld	(ix+PT3C_SKIPCNT),1
	ret

; ---------------------------------------------------------------------
; pt3_setorn / pt3_setsam: select an ornament or a sample.
; In: A = number, IX = channel. Preserves HL and BC.
; Both kinds of data start with a loop position and a length byte.
; ---------------------------------------------------------------------
pt3_setorn:
	push	hl
	ld	(ix+PT3C_ORNNUM),a
	ld	hl,0
	ld	l,a
	add	hl,hl
	ld	de,PT3_ORNPTRS
	add	hl,de
	ld	de,(pt3_base)
	add	hl,de
	call	lib_getw
	ld	de,(pt3_base)
	add	hl,de
	ld	a,(hl)
	ld	(ix+PT3C_ORNLOOP),a
	inc	hl
	ld	a,(hl)
	ld	(ix+PT3C_ORNLEN),a
	inc	hl
	ld	(ix+PT3C_ORNPTR),hl
	pop	hl
	ret

pt3_setsam:
	push	hl
	ld	(ix+PT3C_SAMNUM),a
	ld	hl,0
	ld	l,a
	add	hl,hl
	ld	de,PT3_SAMPTRS
	add	hl,de
	ld	de,(pt3_base)
	add	hl,de
	call	lib_getw
	ld	de,(pt3_base)
	add	hl,de
	ld	a,(hl)
	ld	(ix+PT3C_SAMLOOP),a
	inc	hl
	ld	a,(hl)
	ld	(ix+PT3C_SAMLEN),a
	inc	hl
	ld	(ix+PT3C_SAMPTR),hl
	pop	hl
	ret

; ---------------------------------------------------------------------
; pt3_notefreq: look up the tone period of a note.
; In: A = note number 0 to 95. Out: HL = period. Preserves BC, DE.
; ---------------------------------------------------------------------
pt3_notefreq:
	push	de
	ld	hl,0
	ld	l,a
	add	hl,hl
	ld	de,(pt3_notetab)
	add	hl,de
	call	lib_getw
	pop	de
	ret

; Restart the per-note state when a note starts or is cut.
pt3_newnote:
	xor	a
	ld	(ix+PT3C_SAMPOS),a
	ld	(ix+PT3C_AMPSL),a
	ld	(ix+PT3C_NOISESL),a
	ld	(ix+PT3C_ENVSL),a
	ld	(ix+PT3C_ORNPOS),a
	ld	(ix+PT3C_TSCNT),a
	ld	(ix+PT3C_ONOFF),a
	ld	de,0
	ld	(ix+PT3C_TSLIDE),de
	ld	(ix+PT3C_TACC),de
	ret

; Start the hardware envelope. In: A = shape, HL -> command byte, which
; is followed by the period, high byte first. Out: HL -> last byte used.
pt3_setenv:
	ld	(ay_regs+13),a
	inc	hl
	ld	a,(hl)
	ld	(pt3_envbase+1),a
	inc	hl
	ld	a,(hl)
	ld	(pt3_envbase),a
	ld	(ix+PT3C_ENVEN),1
	ld	de,0
	ld	(pt3_envslide),de
	xor	a
	ld	(pt3_envdelay_cur),a
	ret

; ---------------------------------------------------------------------
; pt3_interp: read one row of pattern data for the channel at IX.
;
; A row is a run of command bytes ended by a note, a note-off (0C0h) or
; an "empty" marker (0D0h). Effect commands (01h to 09h) only announce
; that parameters follow the end of the row; they are fetched afterwards
; in the reverse of the order in which the effects were announced.
; ---------------------------------------------------------------------
pt3_interp:
	ld	hl,pt3_fx		; forget the effects of the last row
	ld	b,11
@clr:
	ld	(hl),0
	inc	hl
	djnz	@clr
	ld	a,(ix+PT3C_NOTE)		; remembered for portamento
	ld	(pt3_prnote),a
	ld	hl,(ix+PT3C_TSLIDE)
	ld	(pt3_prslide),hl
	ld	hl,(ix+PT3C_ADDR)
	push	hl			; (bytes read in this row: see @loop)
	ld	hl,0
	ld	(pt3_rowlim),hl
	pop	hl
@loop:
	push	hl			; a damaged pattern: give up on the
	ld	hl,(pt3_rowlim)	; row after 65536 bytes
	dec	hl
	ld	(pt3_rowlim),hl
	ld	a,h
	or	l
	pop	hl
	jp	z,@quit
	ld	a,(hl)
	cp	0F0h
	jp	nc,@ornsam
	cp	0D1h
	jp	nc,@sample
	cp	0D0h
	jp	z,@quit
	cp	0C1h
	jp	nc,@volume
	cp	0C0h
	jp	z,@noteoff
	cp	0B2h
	jp	nc,@envelope
	cp	0B1h
	jp	z,@skip
	cp	0B0h
	jp	z,@envoff
	cp	050h
	jp	nc,@note
	cp	040h
	jp	nc,@ornament
	cp	020h
	jp	nc,@noise
	cp	010h
	jp	nc,@envsam
	or	a			; 01h-05h, 08h, 09h announce effects
	jr	z,@next
	cp	10
	jr	nc,@next
	cp	6
	jr	z,@next
	cp	7
	jr	z,@next
	push	hl
	ld	hl,pt3_fxcount
	inc	(hl)
	ld	c,(hl)
	ld	hl,pt3_fx
	ld	de,0
	ld	e,a
	add	hl,de
	ld	(hl),c			; remember when it was announced
	pop	hl
@next:
	inc	hl
	jp	@loop

@ornsam:				; 0F0h+n: ornament n, then sample*2
	sub	0F0h
	call	pt3_setorn
	inc	hl
	ld	a,(hl)
	srl	a
	call	pt3_setsam
	ld	(ix+PT3C_ENVEN),0
	ld	(ix+PT3C_ORNPOS),0
	jr	@next
@sample:				; 0D1h-0EFh: sample
	sub	0D0h
	call	pt3_setsam
	jr	@next
@volume:				; 0C1h-0CFh: volume
	sub	0C0h
	ld	(ix+PT3C_VOL),a
	jr	@next
@noteoff:				; 0C0h: silence the channel
	call	pt3_newnote
	ld	(ix+PT3C_ENABLED),0
	jr	@quit
@envelope:				; 0B2h-0BFh: envelope shape + period
	sub	0B1h
	call	pt3_setenv
	ld	(ix+PT3C_ORNPOS),0
	jr	@next
@skip:					; 0B1h: rows between notes
	inc	hl
	ld	a,(hl)
	ld	(ix+PT3C_SKIPN),a
	jr	@next
@envoff:				; 0B0h: envelope off
	ld	(ix+PT3C_ENVEN),0
	ld	(ix+PT3C_ORNPOS),0
	jr	@next
@note:					; 050h-0AFh: note
	sub	050h
	ld	(ix+PT3C_NOTE),a
	call	pt3_newnote
	ld	(ix+PT3C_ENABLED),1
	jr	@quit
@ornament:				; 040h-04Fh: ornament
	sub	040h
	call	pt3_setorn
	ld	(ix+PT3C_ORNPOS),0
	jr	@next
@noise:					; 020h-03Fh: noise period
	sub	020h
	ld	(pt3_noise_base),a
	jr	@next
@envsam:				; 010h-01Fh: envelope, then sample*2
	cp	010h
	jr	nz,@es1
	ld	(ix+PT3C_ENVEN),0
	jr	@es2
@es1:
	sub	010h
	call	pt3_setenv
@es2:
	inc	hl
	ld	a,(hl)
	srl	a
	call	pt3_setsam
	ld	(ix+PT3C_ORNPOS),0
	jp	@next

@quit:
	inc	hl			; step past the byte that ended the row
@fx:					; fetch effect parameters, last first
	ld	a,(pt3_fxcount)
	or	a
	jp	z,@done
	ld	c,a
	ld	a,(pt3_fx+1)
	cp	c
	jr	z,@fx1
	ld	a,(pt3_fx+2)
	cp	c
	jr	z,@fx2
	ld	a,(pt3_fx+3)
	cp	c
	jp	z,@fx3
	ld	a,(pt3_fx+4)
	cp	c
	jp	z,@fx4
	ld	a,(pt3_fx+5)
	cp	c
	jp	z,@fx5
	ld	a,(pt3_fx+8)
	cp	c
	jp	z,@fx8
	ld	a,(pt3_fx+9)
	cp	c
	jp	z,@fx9
@fxnext:
	push	hl
	ld	hl,pt3_fxcount
	dec	(hl)
	pop	hl
	jr	@fx

@fx1:					; tone slide: delay, step (word)
	ld	a,(hl)
	inc	hl
	ld	(ix+PT3C_TSDELAY),a
	ld	(ix+PT3C_TSCNT),a
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	inc	hl
	call	pt3_putstep
	ld	(ix+PT3C_GLISS),1
	ld	(ix+PT3C_ONOFF),0
	or	a
	jr	nz,@fxnext
	ld	a,(pt3_version)		; a zero delay still slides in
	cp	7			; sub-version 7 and later
	jr	c,@fxnext
	inc	(ix+PT3C_TSCNT)
	jr	@fxnext

@fx2:					; portamento: delay, 2 unused, step
	ld	(ix+PT3C_GLISS),0
	ld	(ix+PT3C_ONOFF),0
	ld	a,(hl)
	ld	(ix+PT3C_TSDELAY),a
	ld	(ix+PT3C_TSCNT),a
	inc	hl
	inc	hl
	inc	hl
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	inc	hl
	bit	7,d			; the step is used as a magnitude
	jr	z,@f2pos
	xor	a
	sub	e
	ld	e,a
	sbc	a,a
	sub	d
	ld	d,a
@f2pos:
	call	pt3_putstep
	push	hl
	ld	a,(pt3_prnote)		; distance from the old note's pitch
	call	pt3_notefreq		; to the new one
	ex	de,hl
	ld	a,(ix+PT3C_NOTE)
	call	pt3_notefreq
	or	a
	sbc	hl,de
	ld	(ix+PT3C_TDELTA),hl
	ld	a,(ix+PT3C_NOTE)		; keep playing the old note and
	ld	(ix+PT3C_SLIDETO),a	; slide towards the new one
	ld	a,(pt3_prnote)
	ld	(ix+PT3C_NOTE),a
	ld	a,(pt3_version)
	cp	6
	jr	c,@f2a
	ld	de,(pt3_prslide)	; carry on from any slide in progress
	ld	(ix+PT3C_TSLIDE),de
@f2a:
	ld	de,(ix+PT3C_TSLIDE)
	or	a
	sbc	hl,de			; which way do we need to go?
	jp	p,@f2b
	ld	hl,0
	ld	de,(ix+PT3C_TSSTEP)
	or	a
	sbc	hl,de
	ld	(ix+PT3C_TSSTEP),hl
@f2b:
	pop	hl
	jp	@fxnext

@fx3:					; sample position
	ld	a,(hl)
	inc	hl
	ld	(ix+PT3C_SAMPOS),a
	jp	@fxnext
@fx4:					; ornament position
	ld	a,(hl)
	inc	hl
	ld	(ix+PT3C_ORNPOS),a
	jp	@fxnext
@fx5:					; periodic on/off: on time, off time
	ld	a,(hl)
	inc	hl
	ld	(ix+PT3C_ONOFFD),a
	ld	(ix+PT3C_ONOFF),a
	ld	a,(hl)
	inc	hl
	ld	(ix+PT3C_OFFOND),a
	ld	(ix+PT3C_TSCNT),0
	ld	de,0
	ld	(ix+PT3C_TSLIDE),de
	jp	@fxnext
@fx8:					; envelope slide: delay, step (word)
	ld	a,(hl)
	inc	hl
	ld	(pt3_envdelay),a
	ld	(pt3_envdelay_cur),a
	ld	a,(hl)
	inc	hl
	ld	(pt3_envslide_add),a
	ld	a,(hl)
	inc	hl
	ld	(pt3_envslide_add+1),a
	jp	@fxnext
@fx9:					; song speed
	ld	a,(hl)
	inc	hl
	ld	(pt3_delay),a
	jp	@fxnext

@done:
	ld	a,(ix+PT3C_SKIPN)
	ld	(ix+PT3C_SKIPCNT),a
	ld	(ix+PT3C_ADDR),hl
	ret

; Store DE as the signed 24 bit tone slide step. Preserves A and HL.
pt3_putstep:
	push	af
	ld	(ix+PT3C_TSSTEP),e
	ld	(ix+PT3C_TSSTEP+1),d
	ld	a,d
	rla
	sbc	a,a
	ld	(ix+PT3C_TSSTEP+2),a
	pop	af
	ret

; ---------------------------------------------------------------------
; pt3_chreg: work out this frame's tone and amplitude for the channel at
; IX from its sample, ornament and slides, and add its bits to the mixer.
; ---------------------------------------------------------------------
pt3_chreg:
	ld	a,(ix+PT3C_ENABLED)
	or	a
	jp	z,@off
	ld	hl,0			; find the 4 byte sample record
	ld	l,(ix+PT3C_SAMPOS)
	add	hl,hl
	add	hl,hl
	ld	de,(ix+PT3C_SAMPTR)
	add	hl,de
	ld	b,(hl)			; B = flags: slides, envelope mask
	inc	hl
	ld	c,(hl)			; C = flags: mixer, amplitude
	inc	hl
	call	lib_getw			; HL = tone offset
	ld	de,(ix+PT3C_TACC)
	add	hl,de
	bit	6,c			; accumulate the offset?
	jr	z,@noacc
	ld	(ix+PT3C_TACC),hl
@noacc:
	push	hl
	ld	hl,0			; note + ornament, kept in range
	ld	l,(ix+PT3C_ORNPOS)
	ld	de,(ix+PT3C_ORNPTR)
	add	hl,de
	ld	a,(ix+PT3C_NOTE)
	add	a,(hl)
	jp	p,@notneg
	xor	a
	jr	@gotnote
@notneg:
	cp	96
	jr	c,@gotnote
	ld	a,95
@gotnote:
	call	pt3_notefreq
	pop	de
	add	hl,de
	ld	de,(ix+PT3C_TSLIDE)
	add	hl,de
	ld	(ix+PT3C_TON),l
	ld	a,h
	and	0Fh
	ld	(ix+PT3C_TON+1),a

	ld	a,(ix+PT3C_TSCNT)		; tone slide / portamento
	or	a
	jr	z,@noslide
	dec	a
	ld	(ix+PT3C_TSCNT),a
	jr	nz,@noslide
	ld	hl,(ix+PT3C_TSLIDE)
	ld	de,(ix+PT3C_TSSTEP)
	add	hl,de
	ld	(ix+PT3C_TSLIDE),hl
	ld	a,(ix+PT3C_TSDELAY)
	ld	(ix+PT3C_TSCNT),a
	ld	a,(ix+PT3C_GLISS)
	or	a
	jr	nz,@noslide
	ld	de,(ix+PT3C_TDELTA)	; portamento: has it arrived?
	bit	7,(ix+PT3C_TSSTEP+2)
	jr	nz,@down
	or	a
	sbc	hl,de
	jp	m,@noslide
	jr	@arrived
@down:
	or	a
	sbc	hl,de
	jr	z,@arrived
	jp	p,@noslide
@arrived:
	ld	a,(ix+PT3C_SLIDETO)
	ld	(ix+PT3C_NOTE),a
	ld	(ix+PT3C_TSCNT),0
	ld	de,0
	ld	(ix+PT3C_TSLIDE),de
@noslide:

	ld	a,c			; amplitude, with its slide
	and	0Fh
	ld	e,a
	bit	7,b
	jr	z,@ampdone
	ld	a,(ix+PT3C_AMPSL)
	bit	6,b
	jr	z,@ampdown
	cp	15
	jr	z,@ampdone
	inc	a
	jr	@ampset
@ampdown:
	cp	0F1h			; -15
	jr	z,@ampdone
	dec	a
@ampset:
	ld	(ix+PT3C_AMPSL),a
@ampdone:
	ld	a,(ix+PT3C_AMPSL)
	add	a,e
	jp	p,@amppos
	xor	a
@amppos:
	cp	16
	jr	c,@ampok
	ld	a,15
@ampok:
	ld	e,a			; scale by the channel volume
	ld	a,(ix+PT3C_VOL)
	add	a,a
	add	a,a
	add	a,a
	add	a,a
	or	e
	ld	hl,0
	ld	l,a
	ld	de,(pt3_voltab)
	add	hl,de
	ld	d,(hl)
	bit	0,b			; sample allows the envelope here?
	jr	nz,@noenv
	ld	a,(ix+PT3C_ENVEN)
	or	a
	jr	z,@noenv
	set	4,d
@noenv:
	ld	(ix+PT3C_AMP),d

	bit	7,c			; the 5 bit field in B is either an
	jr	z,@isnoise		; envelope offset or a noise offset
	ld	a,b
	srl	a
	bit	5,b
	jr	z,@envplus
	or	0F0h
	jr	@envadd
@envplus:
	and	0Fh
@envadd:
	add	a,(ix+PT3C_ENVSL)
	bit	5,c
	jr	z,@envkeep
	ld	(ix+PT3C_ENVSL),a
@envkeep:
	ld	hl,pt3_addenv
	add	a,(hl)
	ld	(hl),a
	jr	@mixer
@isnoise:
	ld	a,b
	srl	a
	add	a,(ix+PT3C_NOISESL)
	ld	(pt3_addnoise),a
	bit	5,c
	jr	z,@mixer
	ld	(ix+PT3C_NOISESL),a
@mixer:
	ld	a,c			; tone and noise mask bits
	srl	a
	and	048h
	ld	hl,pt3_mixer
	or	(hl)
	ld	(hl),a

	; step through sample and ornament
	ld	a,(ix+PT3C_SAMPOS)
	inc	a
	cp	(ix+PT3C_SAMLEN)
	jr	c,@samok
	ld	a,(ix+PT3C_SAMLOOP)
@samok:
	ld	(ix+PT3C_SAMPOS),a
	ld	a,(ix+PT3C_ORNPOS)
	inc	a
	cp	(ix+PT3C_ORNLEN)
	jr	c,@ornok
	ld	a,(ix+PT3C_ORNLOOP)
@ornok:
	ld	(ix+PT3C_ORNPOS),a
	jr	@fin
@off:
	ld	(ix+PT3C_AMP),0
@fin:
	ld	hl,pt3_mixer		; make room for the next channel
	srl	(hl)
	ld	a,(ix+PT3C_ONOFF)		; periodic on/off effect
	or	a
	ret	z
	dec	a
	ld	(ix+PT3C_ONOFF),a
	ret	nz
	ld	a,(ix+PT3C_ENABLED)
	xor	1
	ld	(ix+PT3C_ENABLED),a
	ld	a,(ix+PT3C_ONOFFD)
	jr	nz,@onoff
	ld	a,(ix+PT3C_OFFOND)
@onoff:
	ld	(ix+PT3C_ONOFF),a
	ret

; ---------------------------------------------------------------------
; pt3_play: advance the song by one frame and fill ay_regs.
; ---------------------------------------------------------------------
pt3_play:
	ld	a,0FFh
	ld	(ay_regs+13),a
	ld	hl,pt3_delay_cnt
	dec	(hl)
	call	z,pt3_rowtick
	xor	a
	ld	(pt3_addenv),a
	ld	(pt3_mixer),a
	ld	ix,pt3_chA
	call	pt3_chreg
	ld	ix,pt3_chB
	call	pt3_chreg
	ld	ix,pt3_chC
	call	pt3_chreg

	; copy the results to the registers
	ld	hl,(pt3_chA+PT3C_TON)
	ld	(ay_regs+0),hl
	ld	hl,(pt3_chB+PT3C_TON)
	ld	(ay_regs+2),hl
	ld	hl,(pt3_chC+PT3C_TON)
	ld	a,l
	ld	(ay_regs+4),a
	ld	a,h
	ld	(ay_regs+5),a
	ld	a,(pt3_noise_base)
	ld	hl,pt3_addnoise
	add	a,(hl)
	and	31
	ld	(ay_regs+6),a
	ld	a,(pt3_mixer)
	ld	(ay_regs+7),a
	ld	a,(pt3_chA+PT3C_AMP)
	ld	(ay_regs+8),a
	ld	a,(pt3_chB+PT3C_AMP)
	ld	(ay_regs+9),a
	ld	a,(pt3_chC+PT3C_AMP)
	ld	(ay_regs+10),a
	ld	a,(pt3_addenv)		; envelope period = base + sample
	ld	l,a			; offsets (signed) + slide
	rla
	sbc	a,a
	ld	h,a
	ld	de,(pt3_envbase)
	add	hl,de
	ld	de,(pt3_envslide)
	add	hl,de
	ld	a,l
	ld	(ay_regs+11),a
	ld	a,h
	ld	(ay_regs+12),a

	ld	a,(pt3_envdelay_cur)	; envelope slide effect
	or	a
	ret	z
	ret	m
	dec	a
	ld	(pt3_envdelay_cur),a
	ret	nz
	ld	a,(pt3_envdelay)
	ld	(pt3_envdelay_cur),a
	ld	hl,(pt3_envslide)
	ld	de,(pt3_envslide_add)
	add	hl,de
	ld	(pt3_envslide),hl
	ret

; pt3_rowtick: the work done once per row: read the next row for each
; channel whose turn it is, moving to the next pattern when one ends.
pt3_rowtick:
	ld	hl,pt3_row
	inc	(hl)
	ld	ix,pt3_chA
	dec	(ix+PT3C_SKIPCNT)
	jr	nz,@chb
	ld	hl,(ix+PT3C_ADDR)
	ld	a,(hl)
	or	a			; 0 in channel A ends the pattern
	jr	nz,@cha
	ld	a,(pt3_numpos)
	ld	b,a
	ld	a,(pt3_pos)
	inc	a
	cp	b
	jr	nz,@newpos
	ld	hl,pt3_loops		; end of song: note it and loop
	inc	(hl)
	ld	a,(pt3_looppos)
@newpos:
	call	pt3_setpos
	xor	a
	ld	(pt3_noise_base),a
	ld	(pt3_row),a
	ld	ix,pt3_chA
@cha:
	call	pt3_interp
@chb:
	ld	ix,pt3_chB
	dec	(ix+PT3C_SKIPCNT)
	call	z,pt3_interp
	ld	ix,pt3_chC
	dec	(ix+PT3C_SKIPCNT)
	call	z,pt3_interp
	ld	a,(pt3_delay)
	ld	(pt3_delay_cnt),a
	ret

; pt3_length: how many frames the song lasts before it ends. Call it
; straight after pt3_init, and call pt3_init again afterwards, because
; it works by running through all the song's rows (without the per-frame
; work, so it is quick). Out: HL = frames.
pt3_length:
	ld	hl,0
	ld	(pt3_frames),hl
	ld	(pt3_rows),hl	; (rows counted)
@row:
	call	pt3_rowtick
	ld	hl,(pt3_rows)		; give up after 65536 rows, which no
	inc	hl			; real song needs (a damaged one that
	ld	(pt3_rows),hl		; never ends)
	ld	a,(pt3_rows+2)
	or	a
	jp	nz,@done
	ld	a,(pt3_loops)
	or	a
	jr	nz,@done
	ld	de,0
	ld	a,(pt3_delay)
	ld	e,a
	or	a
	jr	nz,@add
	inc	d			; a speed of 0 lasts 256 frames
@add:
	ld	hl,(pt3_frames)
	add	hl,de
	ld	(pt3_frames),hl
	ld	a,(pt3_frames+2)	; give up on a song that never ends
	cp	10h			; (about 5 hours)
	jr	c,@row
@done:
	ld	hl,(pt3_frames)
	ret