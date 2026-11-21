; ayvdp.asm - plays AY-3-8912 register values on the Agon VDP.
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
; Part of the reusable agonAMP sound library in lib/. It plays whatever
; is in the table ay_regs, which a replayer (pt3play.asm, psgplay.asm or
; your own code) fills in each frame. Needs libsupport.asm.
;
; Entry points:
;   ay_init      set up VDP channels 0 to 4; call once before playing
;   ay_update    play the current contents of ay_regs; call every frame
;   ay_silence   turn the sound down (pause); ay_update turns it up again
;   ay_reset     silence, and forget the envelope state (new song)
;   ay_shutdown  silence, and release the two extra channels
;   ay_volume    A = 0 to 127: set the VDP's overall volume (see below)
;   ay_chips     A = 1 or 2: how many AY chips to play (see below)
;   ay_clock     HL = the AY clock in Hz that the registers are meant for
;                (kept between 1000000 and 2000000; ay_init sets 1773400,
;                the ZX Spectrum's)
;
; Muting: set bits in the byte ay_mute to silence parts of the sound.
; Bits 0, 1 and 2 mute AY channels A, B and C (their tone and their
; share of the noise and envelope sound); bit 3 mutes all noise; bit 4
; mutes the envelope "buzz" channel. ay_init clears it.
;
; Two chips (TurboSound): after ay_chips with A = 2, ay_update also
; plays the table ay_regs2 as a second AY chip, on VDP channels 5 to 9,
; and ay_mute2 mutes its parts as ay_mute does the first chip's. The
; two chips then share the VDP's volume range, so each plays at half the
; level a single chip does. ay_chips with A = 1 goes back to one chip
; and gives the extra channels up. ay_init starts with one.
;
; Everything remembered about a chip lies between the labels ay_ctx and
; ay_ctx_end. The second chip's copy is at ay_ctx2, so its envelope
; state, for instance, is at ay_ctx2 + (ay_env_shape - ay_ctx).
;
; All registers are used, including IX and IY.
;
; The Agon has no AY chip, so each frame's registers are translated into
; VDP audio commands (VDU 23,0,&85,...). Five VDP channels are used:
;
;   0-2  square waves for AY tone channels A, B and C
;   3    "VIC noise" for the AY noise generator
;   4    sawtooth or triangle for the AY envelope when it repeats fast
;        enough to be heard as a pitch (the "buzzer" sound)
;
; Points that shaped this, all taken from the VDP 2.16.0 source:
;
; * The VDP rescales the whole mix whenever the channel volumes add up
;   to more than 127, which would make voices pump. So the three AY
;   channels share 126 between them: 42 each at full volume (21 each
;   when two chips are playing).
; * VDP frequencies are whole Hz, too coarse for bass notes. Telling one
;   channel that the sample rate is 65535 Hz while the VDP really runs
;   at 16384 Hz makes that channel play at a quarter of the frequency
;   asked for, which gives quarter-Hz steps. This "fine" mode only works
;   up to 8191 (the VDP's sum overflows above that), so high notes go
;   back to the true rate and whole Hz.
; * Slow envelopes are followed in software and become volume changes.
; * An AY channel with tone and noise both on is the logical AND of the
;   two signals. The VDP cannot do that, so the two are simply played
;   together at half volume each. This is an approximation.
;
; Everything that depends on the AY clock is worked out by ay_clock, so
; music written for other machines (the Atari ST's 2 MHz, for instance)
; plays at the right pitch.

AY_VOICES:	equ	5
AY_V_NOISE:	equ	3
AY_V_BUZZ:		equ	4
AY_RATE_TRUE:	equ	16384
AY_RATE_FINE:	equ	65535

; What the VDP was last told (sh_) and what is wanted now (nw_). The
; order and spacing of these four blocks is relied on by ay_update.
ay_ctx:
ay_sh_vol:		ds	AY_VOICES
ay_sh_fine:	ds	AY_VOICES
ay_nw_vol:		ds	AY_VOICES
ay_nw_fine:	ds	AY_VOICES
ay_sh_freq:	ds	AY_VOICES*2
ay_nw_freq:	ds	AY_VOICES*2
ay_regs:	ds	16	; AY registers R0 to R13, filled in by a
				; replayer each frame (2 spare bytes)
ay_mute:	ds	1	; what is muted (see the top of the file)
ay_buzzwave:	ds	1	; waveform the buzz channel has now
ay_i:		ds	1	; loop counter
ay_mix:		ds	1	; mixer bits still to be looked at
ay_amp:		ds	1	; current channel's VDP volume
ay_tp:		ds	3	; current channel's tone period
ay_nz_vol:		ds	1	; noise channel volume being added up
ay_bz_vol:		ds	1	; buzz channel volume being added up
ay_env_shape:	ds	1	; envelope shape, 8 to 15
ay_env_buzz:	ds	1	; non-zero if the envelope is audio rate
ay_env_wave:	ds	1	; waveform for it: 2 sawtooth, 1 triangle
ay_env_level:	ds	1	; software envelope level, 0 to 15
ay_env_ep:		ds	3	; envelope period
ay_env_pos:	ds	3	; software envelope position
ay_ctx_end:
ay_chbase:	ds	1	; 0, or 5 while the second chip is dealt with
ay_nchips:	ds	1	; chips being played: 1 or 2
ay_amps:	ds	3	; address of the amplitude table in use
; Values that depend on the AY clock (see ay_clock).
ay_clk:		ds	3	; the clock, Hz
ay_k_fine:	ds	3	; 4 * clock / 16
ay_k_coarse:	ds	3	; clock / 16
ay_k_saw:	ds	3	; 4 * clock / 256
ay_k_tri:	ds	3	; 4 * clock / 512
ay_k_env:	ds	3	; clock * 256 / 800
ay_k_low:	ds	3	; shortest tone period played
ay_k_bzsaw:	ds	3	; envelope periods below these are buzz
ay_k_bztri:	ds	3
ay_k_noise:	ds	1	; 16 * 554 kHz / (clock / 16)
; the first chip's state, put aside
ay_save:	ds	ay_ctx_end-ay_ctx
ay_ctx2:	ds	ay_ctx_end-ay_ctx	; the second chip's state
; Places in the second chip's state.
ay_regs2:	equ	ay_ctx2+ay_regs-ay_ctx
ay_mute2:	equ	ay_ctx2+ay_mute-ay_ctx

; ay_cmd: queue the start of an audio command.
; In: C = channel (0 to 4 of the chip being dealt with, or 255 for the
; whole sound system), B = command number. Preserves BC, DE, HL.
ay_cmd:
	ld	a,23
	call	vdu_put
	xor	a
	call	vdu_put
	ld	a,085h
	call	vdu_put
	; (5 while the second chip is dealt with)
	ld	a,(ay_chbase)
	add	a,c
	call	vdu_put
	ld	a,b
	jp	vdu_put

; ay_init: set up the five VDP channels of one chip. Channels 0 to 2
; exist at boot; 3 and 4 have to be enabled.
ay_init:
	ld	hl,1773400
	call	ay_clock
	xor	a
	ld	(ay_mute),a
	ld	(ay_mute2),a
	ld	(ay_chbase),a
	inc	a
	ld	(ay_nchips),a
	ld	hl,ay_amptab
	ld	(ay_amps),hl
	call	ay_setup
	jp	ay_reset

; ay_setup: enable the channels of the chip being dealt with and give
; them their waveforms.
ay_setup:
	ld	c,AY_V_NOISE
	ld	a,(ay_chbase)		; the second chip's all need enabling
	or	a
	jr	z,@enable
	ld	c,0
@enable:
	ld	b,8			; enable channel
	call	ay_cmd
	inc	c
	ld	a,c
	cp	AY_VOICES
	jr	nz,@enable
	ld	c,0
@wave:
	ld	b,4			; set waveform
	call	ay_cmd
	ld	hl,ay_waves
	ld	de,0
	ld	e,c
	add	hl,de
	ld	a,(hl)
	call	vdu_put
	inc	c
	ld	a,c
	cp	AY_VOICES
	jr	nz,@wave
	ld	a,2
	ld	(ay_buzzwave),a
	jp	vdu_flush

; ay_clock: set the AY clock the registers are meant for, and work out
; the values that depend on it. In: HL = clock in Hz. Clocks below
; 1000000 are taken as 1000000 and above 2000000 as 2000000 (the fine
; frequency mode would overflow above that; real machines lie between).
; Corrupts AF, BC, DE, HL.
ay_clock:
	ld	de,1000000
	or	a
	sbc	hl,de
	add	hl,de
	jr	nc,@notlow
	ex	de,hl
@notlow:
	ld	de,2000001
	or	a
	sbc	hl,de
	add	hl,de
	jr	c,@nothigh
	ld	hl,2000000
@nothigh:
	ld	(ay_clk),hl
	ld	de,4			; 4 * clock / 16, for fine mode tones
	ld	bc,2
	call	ay_cdiv
	ld	(ay_k_fine),hl
	ld	de,16			; clock / 16, for true rate tones
	ld	bc,8
	call	ay_cdiv
	ld	(ay_k_coarse),hl
	ld	de,64			; 4 * clock / 256: sawtooth envelope
	ld	bc,32
	call	ay_cdiv
	ld	(ay_k_saw),hl
	ld	de,128			; 4 * clock / 512: triangle envelope
	ld	bc,64
	call	ay_cdiv
	ld	(ay_k_tri),hl
	ld	de,25			; 256 * envelope steps per frame * EP,
	ld	bc,12			; that is clock * 256 / (16 * 50)
	call	ay_cdiv
	add	hl,hl
	add	hl,hl
	add	hl,hl
	ld	(ay_k_env),hl
	; Noise: the VIC generator counts up to 127 at 554 kHz, the AY
	; shifts at clock / 16 / period, so the VIC value is 128 less
	; period * 8864000 / (clock / 16), kept here in 1/16ths.
	ld	de,32			; (clock / 32 added to round)
	ld	bc,0
	call	ay_cdiv
	ld	bc,8864000
	add	hl,bc
	push	hl
	ld	de,16
	ld	bc,0
	call	ay_cdiv
	ex	de,hl			; DE = clock / 16
	pop	hl
	call	lib_div24
	ld	a,l
	ld	(ay_k_noise),a
	ld	de,131072		; tone periods below clock / 131072 + 1
	call	@plus1			; are too high for the VDP
	ld	(ay_k_low),hl
	ld	de,5120			; envelope periods below these repeat
	call	@plus1			; at 20 Hz or more: sawtooth,
	ld	(ay_k_bzsaw),hl
	ld	de,10240		; and triangle
	call	@plus1
	ld	(ay_k_bztri),hl
	ret
@plus1:
	ld	bc,0
	call	ay_cdiv
	inc	hl
	ret

; ay_cdiv: HL = (ay_clk + BC) / DE. Preserves DE.
ay_cdiv:
	ld	hl,(ay_clk)
	add	hl,bc
	jp	lib_div24

; ay_chips: choose how many AY chips ay_update plays. In: A = 1 or 2.
ay_chips:
	ld	hl,ay_nchips
	cp	(hl)
	ret	z
	cp	2
	jr	z,@two
	ld	hl,ay_silence1		; back to one: give channels 5 to 9 up
	call	ay_on2
	ld	a,AY_VOICES
	ld	(ay_chbase),a
	ld	c,0
@off:
	ld	b,9			; disable channel
	call	ay_cmd
	inc	c
	ld	a,c
	cp	AY_VOICES
	jr	nz,@off
	xor	a
	ld	(ay_chbase),a
	inc	a
	ld	(ay_nchips),a
	ld	hl,ay_amptab
	ld	(ay_amps),hl
	jp	ay_reset		; (the volumes must all be sent again)
@two:
	ld	hl,ay_setup
	call	ay_on2
	ld	a,2
	ld	(ay_nchips),a
	ld	hl,ay_amptab2
	ld	(ay_amps),hl
	jp	ay_reset

; ay_on2: call the routine at HL with the second chip's state in place
; of the first chip's, and its channels in place of channels 0 to 4.
ay_on2:
	push	hl
	ld	hl,ay_ctx
	ld	de,ay_save
	ld	bc,ay_ctx_end-ay_ctx
	ldir
	ld	hl,ay_ctx2
	ld	de,ay_ctx
	ld	bc,ay_ctx_end-ay_ctx
	ldir
	ld	a,AY_VOICES
	ld	(ay_chbase),a
	pop	hl
	call	@call
	xor	a
	ld	(ay_chbase),a
	ld	hl,ay_ctx
	ld	de,ay_ctx2
	ld	bc,ay_ctx_end-ay_ctx
	ldir
	ld	hl,ay_save
	ld	de,ay_ctx
	ld	bc,ay_ctx_end-ay_ctx
	ldir
	ret
@call:
	jp	(hl)

; ay_both: call the routine at HL for the first chip, and for the second
; as well if two are being played.
ay_both:
	push	hl
	call	@call
	pop	hl
	ld	a,(ay_nchips)
	cp	2
	ret	nz
	jp	ay_on2
@call:
	jp	(hl)

; ay_reset: start afresh for a new song.
ay_reset:
	ld	hl,ay_reset1
	jp	ay_both
ay_reset1:
	ld	hl,ay_sh_vol		; make every shadow value "unknown"
	ld	b,AY_VOICES*2		; (volumes and rate modes)
@inv:
	ld	(hl),0FFh
	inc	hl
	djnz	@inv
	ld	hl,ay_sh_freq		; (frequencies)
	ld	b,AY_VOICES*2
@invf:
	ld	(hl),0FFh
	inc	hl
	djnz	@invf
	ld	hl,0
	ld	(ay_env_pos),hl
	ld	a,9
	ld	(ay_env_shape),a
	xor	a
	ld	(ay_env_buzz),a
	ld	(ay_env_level),a
	; fall through to silence everything

; (ay_silence1: turn the five channels of one chip down.)
ay_silence1:
	ld	c,0
@v:
	ld	b,2			; set volume
	call	ay_cmd
	xor	a
	call	vdu_put
	ld	hl,ay_sh_vol
	ld	de,0
	ld	e,c
	add	hl,de
	ld	(hl),0
	inc	c
	ld	a,c
	cp	AY_VOICES
	jr	nz,@v
	jp	vdu_flush

; ay_silence: turn all the channels down (used for pause and stop).
ay_silence:
	ld	hl,ay_silence1
	jp	ay_both

; ay_volume: set the VDP's overall sound volume. In: A = 0 to 127.
; This is the VDP's own master volume (VDP 2.5.0 and later). It applies
; to every channel, not only the five used here, and it stays as set
; after the program ends, so a program that changes it should put the
; old value back when it finishes.
ay_volume:
	push	af
	; "channel" 255 = the whole sound system
	ld	c,255
	ld	b,2			; set volume
	call	ay_cmd
	pop	af
	call	vdu_put
	jp	vdu_flush

; ay_shutdown: give the sound system back as it was found.
ay_shutdown:
	ld	a,1
	call	ay_chips
	call	ay_silence
	ld	c,0
@r:
	ld	b,10			; reset channel
	call	ay_cmd
	inc	c
	ld	a,c
	cp	3
	jr	nz,@r
	ld	c,AY_V_NOISE
	ld	b,9			; disable channel
	call	ay_cmd
	ld	c,AY_V_BUZZ
	ld	b,9
	call	ay_cmd
	jp	vdu_flush

; ---------------------------------------------------------------------
; ay_update: translate ay_regs (and ay_regs2, if two chips are being
; played) into VDP commands. Call once per frame.
; ---------------------------------------------------------------------
ay_update:
	ld	hl,ay_update1
	jp	ay_both
ay_update1:
	; A write to R13 restarts the envelope. Shapes 0-7 behave like
	; shapes 9 and 15, so fold them.
	ld	a,(ay_regs+13)
	cp	0FFh
	jr	z,@noshape
	and	0Fh
	cp	4
	jr	nc,@s1
	ld	a,9
	jr	@s2
@s1:
	cp	8
	jr	nc,@s2
	ld	a,15
@s2:
	ld	(ay_env_shape),a
	ld	hl,0
	ld	(ay_env_pos),hl
@noshape:
	ld	hl,ay_regs+11		; envelope period, 0 counts as 1
	call	lib_getw
	ld	a,h
	or	l
	jr	nz,@ep
	inc	hl
@ep:
	ld	(ay_env_ep),hl

	; Is the envelope a repeating shape fast enough to be a pitch
	; (20 Hz or more)? Then it is played as a waveform.
	xor	a
	ld	(ay_env_buzz),a
	ld	a,(ay_env_shape)
	bit	0,a			; odd shapes hold, they never repeat
	jr	nz,@soft
	ld	de,(ay_k_bzsaw)		; sawtooth: 8 and 12
	ld	b,2
	bit	1,a
	jr	z,@lim
	ld	de,(ay_k_bztri)		; triangle: 10 and 14
	ld	b,1
@lim:
	or	a
	sbc	hl,de
	jr	nc,@soft
	ld	a,1
	ld	(ay_env_buzz),a
	ld	a,b
	ld	(ay_env_wave),a
@soft:
	; Follow the envelope in software as well: 16 steps per ramp,
	; position kept in 1/256 steps, 4096 units per ramp.
	ld	hl,(ay_k_env)
	ld	de,(ay_env_ep)
	call	lib_div24
	ld	de,(ay_env_pos)
	add	hl,de
	ld	de,12288		; from the third ramp on, wrap within
	; ramps 1 and 2 so the count stays small
	or	a
	sbc	hl,de
	jr	c,@nowrap
	ld	a,h
	and	1Fh
	ld	h,a
	ld	(lib_divnum),hl		; clear the top byte: low 13 bits only
	xor	a
	ld	(lib_divnum+2),a
	ld	hl,(lib_divnum)
	ld	de,4096
	add	hl,de
	jr	@wrapped
@nowrap:
	add	hl,de
@wrapped:
	ld	(ay_env_pos),hl
	; level from shape and position: H = pos / 256
	ld	a,h
	and	0Fh
	ld	e,a			; E = step within the ramp
	ld	a,h
	rrca
	rrca
	rrca
	rrca
	and	0Fh
	ld	d,a			; D = ramp number
	ld	a,(ay_env_shape)
	ld	c,a
	ld	a,d
	or	a
	jr	z,@ramp			; first ramp: direction from "attack"
	bit	0,c			; hold?
	jr	z,@repeat
	; held level: 15 if attack and alternate differ, else 0
	ld	a,c
	rrca
	xor	c
	and	2
	ld	a,0
	jr	z,@gotlevel
	ld	a,15
	jr	@gotlevel
@repeat:
	bit	1,c			; alternate: odd ramps run backwards
	jr	z,@ramp
	bit	0,d
	jr	z,@ramp
	ld	a,c
	xor	4
	ld	c,a
@ramp:
	ld	a,e
	bit	2,c
	jr	nz,@gotlevel
	ld	a,15
	sub	e
@gotlevel:
	ld	(ay_env_level),a

	; Share out each AY channel's amplitude between its tone, the
	; noise channel and the buzz channel.
	xor	a
	ld	(ay_nz_vol),a
	ld	(ay_bz_vol),a
	ld	(ay_i),a
	ld	a,(ay_regs+7)
	ld	(ay_mix),a
@chan:
	ld	a,(ay_i)
	ld	de,0
	ld	e,a
	ld	hl,ay_regs+8
	add	hl,de
	ld	a,(hl)			; amplitude register
	ld	b,0			; B bit 2 = uses the buzz channel
	bit	4,a
	jr	z,@fixed
	ld	a,(ay_env_buzz)
	or	a
	ld	a,(ay_env_level)
	jr	z,@fixed
	ld	b,4
	ld	a,15
@fixed:
	and	0Fh
	ld	e,a
	ld	hl,(ay_amps)
	add	hl,de
	ld	c,(hl)
	ld	a,(ay_i)		; a muted channel plays at volume 0
	ld	e,a
	ld	hl,ay_chbits
	add	hl,de
	ld	a,(ay_mute)
	and	(hl)
	ld	a,c
	jr	z,@heard
	xor	a
@heard:
	ld	(ay_amp),a
	ld	a,(ay_i)		; tone period of this channel
	add	a,a
	ld	e,a
	ld	hl,ay_regs
	add	hl,de
	call	lib_getw
	ld	a,h
	and	0Fh
	ld	h,a
	ld	(ay_tp),hl
	ld	a,(ay_mix)
	bit	0,a			; mixer bit clear = tone on
	jr	nz,@notone
	ld	de,(ay_k_low)		; periods too short for the VDP to
	or	a			; reproduce count as no tone
	sbc	hl,de
	jr	c,@notone
	set	0,b
@notone:
	ld	a,(ay_mix)
	bit	3,a
	jr	nz,@nonoise
	set	1,b
@nonoise:
	; B = components in use: bit 0 tone, bit 1 noise, bit 2 buzz.
	ld	a,(ay_amp)
	ld	d,a			; D = share for tone and noise
	ld	e,a			; E = share for buzz
	ld	a,b
	cp	7
	jr	nz,@not3
	srl	d			; all three: quarter, quarter, half
	srl	d
	srl	e
	jr	@shared
@not3:
	cp	3
	jr	z,@half
	cp	5
	jr	z,@half
	cp	6
	jr	nz,@shared
@half:
	srl	d
	srl	e
@shared:
	ld	a,(ay_i)
	ld	hl,ay_nw_vol
	push	de
	ld	de,0
	ld	e,a
	add	hl,de
	pop	de
	ld	(hl),0
	bit	0,b
	jr	z,@t0
	ld	(hl),d
@t0:
	bit	1,b
	jr	z,@n0
	ld	a,(ay_nz_vol)
	add	a,d
	ld	(ay_nz_vol),a
@n0:
	bit	2,b
	jr	z,@b0
	ld	a,(ay_bz_vol)
	add	a,e
	ld	(ay_bz_vol),a
@b0:
	bit	0,b
	jr	z,@nofreq
	; Tone frequency: fine mode for periods of 64 and more.
	ld	hl,(ay_tp)
	ld	de,64
	or	a
	sbc	hl,de
	ld	hl,(ay_k_fine)
	ld	c,1
	jr	nc,@div
	ld	hl,(ay_k_coarse)
	ld	c,0
@div:
	ld	de,(ay_tp)
	call	lib_div24
	ex	de,hl			; DE = frequency value
	ld	a,(ay_i)
	call	ay_setnew
@nofreq:
	ld	hl,ay_mix
	srl	(hl)
	ld	hl,ay_i
	inc	(hl)
	ld	a,(hl)
	cp	3
	jp	nz,@chan

	; Noise: the VIC generator counts from the value up to 127 at
	; 554 kHz, the AY shifts at clock / 16 / period, so 128 - period *
	; ay_k_noise / 16 gives the same rate (128 - 5 * period at the
	; Spectrum's clock).
	ld	a,(ay_mute)
	bit	3,a
	ld	a,(ay_nz_vol)
	jr	z,@nzon
	xor	a
@nzon:
	ld	(ay_nw_vol+AY_V_NOISE),a
	ld	a,(ay_regs+6)
	and	31
	jr	nz,@np
	inc	a
@np:
	ld	b,a
	ld	a,(ay_k_noise)
	ld	c,a
	mlt	bc
	ld	a,c			; BC / 16
	srl	b
	rra
	srl	b
	rra
	srl	b
	rra
	srl	b
	rra
	ld	c,a
	ld	a,b			; (more than 255: none left)
	or	a
	jr	nz,@nz0
	ld	a,128
	sub	c
	jr	nc,@nf
@nz0:
	xor	a
@nf:
	ld	de,0
	ld	e,a
	ld	c,0FFh			; never change this channel's rate
	ld	a,AY_V_NOISE
	call	ay_setnew

	; Buzz: sawtooth at clock / (256 * EP), triangle at half that.
	ld	a,(ay_mute)
	bit	4,a
	ld	a,(ay_bz_vol)
	jr	z,@bzon
	xor	a
@bzon:
	ld	(ay_nw_vol+AY_V_BUZZ),a
	or	a
	jr	z,@send
	ld	a,(ay_env_wave)
	ld	hl,ay_buzzwave
	cp	(hl)
	jr	z,@samewave
	ld	(hl),a
	ld	c,AY_V_BUZZ
	ld	b,4
	call	ay_cmd
	ld	a,(ay_buzzwave)
	call	vdu_put
	ld	a,0FFh			; a new waveform starts with defaults
	ld	(ay_sh_vol+AY_V_BUZZ),a
	ld	(ay_sh_fine+AY_V_BUZZ),a
	ld	(AY_V_BUZZ*2+ay_sh_freq),a
	ld	(AY_V_BUZZ*2+1+ay_sh_freq),a
@samewave:
	ld	hl,(ay_env_ep)
	ld	de,4
	or	a
	sbc	hl,de
	add	hl,de
	jr	nc,@epok
	ex	de,hl			; keep the result within range
@epok:
	ex	de,hl			; DE = EP
	ld	hl,(ay_k_saw)
	ld	a,(ay_env_wave)
	cp	2
	jr	z,@bdiv
	ld	hl,(ay_k_tri)
@bdiv:
	call	lib_div24
	ex	de,hl
	ld	c,1
	ld	a,AY_V_BUZZ
	call	ay_setnew

@send:
	; Tell the VDP whatever changed. Silent channels are only told
	; their volume; rate and frequency follow when they next sound.
	xor	a
	ld	(ay_i),a
@voice:
	ld	a,(ay_i)
	ld	c,a
	ld	de,0
	ld	e,a
	ld	ix,ay_sh_vol
	add	ix,de			; IX+0 old volume, IX+5 old mode,
					; IX+10 new volume, IX+15 new mode
	ld	iy,ay_sh_freq
	add	iy,de
	add	iy,de			; IY+0 old frequency, IY+10 new
	ld	a,(ix+10)
	or	a
	jr	z,@vol
	ld	a,(ix+15)		; rate mode
	cp	0FFh
	jr	z,@freq
	cp	(ix+5)
	jr	z,@freq
	ld	(ix+5),a
	ld	b,13			; set sample rate
	call	ay_cmd
	ld	hl,AY_RATE_TRUE
	ld	a,(ix+5)
	or	a
	jr	z,@rate
	ld	hl,AY_RATE_FINE
@rate:
	ld	a,l
	call	vdu_put
	ld	a,h
	call	vdu_put
	ld	(iy+0),0FFh		; the frequency must be sent again
	ld	(iy+1),0FFh
@freq:
	ld	a,(iy+10)
	cp	(iy+0)
	jr	nz,@fsend
	ld	a,(iy+11)
	cp	(iy+1)
	jr	z,@vol
@fsend:
	ld	b,3			; set frequency
	call	ay_cmd
	ld	a,(iy+10)
	ld	(iy+0),a
	call	vdu_put
	ld	a,(iy+11)
	ld	(iy+1),a
	call	vdu_put
@vol:
	ld	a,(ix+10)
	cp	(ix+0)
	jr	z,@nextv
	ld	(ix+0),a
	ld	b,2			; set volume
	call	ay_cmd
	ld	a,(ix+0)
	call	vdu_put
@nextv:
	ld	hl,ay_i
	inc	(hl)
	ld	a,(hl)
	cp	AY_VOICES
	jp	nz,@voice
	jp	vdu_flush

; ay_setnew: record a channel's wanted frequency and rate mode.
; In: A = VDP channel, DE = frequency value, C = mode (0 true rate,
; 1 fine, 0FFh leave alone).
ay_setnew:
	push	de
	ld	de,0
	ld	e,a
	ld	hl,ay_nw_fine
	add	hl,de
	ld	(hl),c
	ld	hl,ay_nw_freq
	add	hl,de
	add	hl,de
	pop	de
	ld	(hl),e
	inc	hl
	ld	(hl),d
	ret

; Mute bits of AY channels A, B and C.
ay_chbits:	db	1,2,4
; Waveforms for the five channels: square x 3, VIC noise, sawtooth.
ay_waves:	db	0,0,0,5,2
; AY amplitude (0-15) to VDP volume. The AY steps are 3 dB apart; full
; volume is 42 so that three channels fit in the VDP's 127.
ay_amptab:	db	0,1,1,1,1,2,2,3,4,5,7,11,15,21,30,42
; The same at half the level, for when two chips share the 127.
ay_amptab2:	db	0,1,1,1,1,1,1,2,2,3,4,6,8,11,15,21