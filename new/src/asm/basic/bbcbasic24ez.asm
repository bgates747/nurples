; --- Begin mos_api.inc ---
;
; Title:	AGON MOS - API for user projects
; Author:	Dean Belfield
; Created:	03/08/2022
; Last Updated:	11/11/2023
;
; Modinfo:
; 05/08/2022:	Added mos_feof
; 09/08/2022:	Added system variables: cursorX, cursorY
; 18/08/2022:	Added system variables: scrchar, scrpixel, audioChannel, audioSuccess, vpd_pflags
; 05/09/2022:	Added mos_ren, vdp_pflag_mode
; 24/09/2022:	Added mos_getError, mos_mkdir
; 13/10/2022:	Added mos_oscli
; 23/02/2023:	Added more sysvars, fixed typo in sysvar_audioSuccess, offsets for sysvar_scrCols, sysvar_scrRows
; 04/03/2023:	Added sysvar_scrpixelIndex
; 08/03/2023:	Renamed sysvar_keycode to sysvar_keyascii, added sysvar_vkeycode
; 15/03/2023:	Added mos_copy, mos_getrtc, mos_setrtc, rtc, vdp_pflag_rtc
; 21/03/2023:	Added mos_setintvector, sysvars for keyboard status, vdu codes for vdp
; 22/03/2023:	The VDP commands are now indexed from 0x80
; 29/03/2023:	Added mos_uopen, mos_uclose, mos_ugetc, mos_uputc
; 13/04/2023:	Added FatFS file structures (FFOBJID, FIL, DIR, FILINFO)
; 15/04/2023:	Added mos_getfil, mos_fread, mos_fwrite and mos_flseek
; 19/05/2023:	Added sysvar_scrMode
; 05/06/2023:	Added sysvar_rtcEnable
; 03/08/2023:	Added mos_setkbvector
; 10/08/2023:	Added mos_getkbmap
; 11/11/2023:	Added mos_i2c_open, mos_i2c_close, mos_i2c_write and mos_i2c_read

; VDP control (VDU 23, 0, n)
;
vdp_gp:			EQU 	80h
vdp_keycode:		EQU 	81h
vdp_cursor:		EQU	82h
vdp_scrchar:		EQU	83h
vdp_scrpixel:		EQU	84h
vdp_audio:		EQU	85h
vdp_mode:		EQU	86h
vdp_rtc:		EQU	87h
vdp_keystate:		EQU	88h
vdp_logicalcoords:	EQU	C0h
vdp_terminalmode:	EQU	FFh

; MOS high level functions
;
mos_getkey:		EQU	00h
mos_load:		EQU	01h
mos_save:		EQU	02h
mos_cd:			EQU	03h
mos_dir:		EQU	04h
mos_del:		EQU	05h
mos_ren:		EQU	06h
mos_mkdir:		EQU	07h
mos_sysvars:		EQU	08h
mos_editline:		EQU	09h
mos_fopen:		EQU	0Ah
mos_fclose:		EQU	0Bh
mos_fgetc:		EQU	0Ch
mos_fputc:		EQU	0Dh
mos_feof:		EQU	0Eh
mos_getError:		EQU	0Fh
mos_oscli:		EQU	10h
mos_copy:		EQU	11h
mos_getrtc:		EQU	12h
mos_setrtc:		EQU	13h
mos_setintvector:	EQU	14h
mos_uopen:		EQU	15h
mos_uclose:		EQU	16h
mos_ugetc:		EQU	17h
mos_uputc:		EQU 	18h
mos_getfil:		EQU	19h
mos_fread:		EQU	1Ah
mos_fwrite:		EQU	1Bh
mos_flseek:		EQU	1Ch
mos_setkbvector:	EQU	1Dh
mos_getkbmap:		EQU	1Eh
mos_i2c_open:		EQU	1Fh
mos_i2c_close:		EQU	20h
mos_i2c_write:		EQU	21h
mos_i2c_read:		EQU	22h


; FatFS file access functions
;
ffs_fopen:		EQU	80h
ffs_fclose:		EQU	81h
ffs_fread:		EQU	82h
ffs_fwrite:		EQU	83h
ffs_flseek:		EQU	84h
ffs_ftruncate:		EQU	85h
ffs_fsync:		EQU	86h
ffs_fforward:		EQU	87h
ffs_fexpand:		EQU	88h
ffs_fgets:		EQU	89h
ffs_fputc:		EQU	8Ah
ffs_fputs:		EQU	8Bh
ffs_fprintf:		EQU	8Ch
ffs_ftell:		EQU	8Dh
ffs_feof:		EQU	8Eh
ffs_fsize:		EQU	8Fh
ffs_ferror:		EQU	90h

; FatFS directory access functions
;
ffs_dopen:		EQU	91h
ffs_dclose:		EQU	92h
ffs_dread:		EQU	93h
ffs_dfindfirst:		EQU	94h
ffs_dfindnext:		EQU	95h

; FatFS file and directory management functions
;
ffs_stat:		EQU	96h
ffs_unlink:		EQU	97h
ffs_rename:		EQU	98h
ffs_chmod:		EQU	99h
ffs_utime:		EQU	9Ah
ffs_mkdir:		EQU	9Bh
ffs_chdir:		EQU	9Ch
ffs_chdrive:		EQU	9Dh
ffs_getcwd:		EQU	9Eh

; FatFS volume management and system configuration functions
;
ffs_mount:		EQU	9Fh
ffs_mkfs:		EQU	A0h
ffs_fdisk:		EQU	A1h
ffs_getfree:		EQU	A2h
ffs_getlabel:		EQU	A3h
ffs_setlabel:		EQU	A4h
ffs_setcp:		EQU	A5h
	
; File access modes
;
fa_read:		EQU	01h
fa_write:		EQU	02h
fa_open_existing:	EQU	00h
fa_create_new:		EQU	04h
fa_create_always:	EQU	08h
fa_open_always:		EQU	10h
fa_open_append:		EQU	30h
	
; System variable indexes for api_sysvars
; Index into _sysvars in globals.asm
;
sysvar_time:		EQU	00h	; 4: Clock timer in centiseconds (incremented by 2 every VBLANK)
sysvar_vpd_pflags:	EQU	04h	; 1: Flags to indicate completion of VDP commands
sysvar_keyascii:	EQU	05h	; 1: ASCII keycode, or 0 if no key is pressed
sysvar_keymods:		EQU	06h	; 1: Keycode modifiers
sysvar_cursorX:		EQU	07h	; 1: Cursor X position
sysvar_cursorY:		EQU	08h	; 1: Cursor Y position
sysvar_scrchar:		EQU	09h	; 1: Character read from screen
sysvar_scrpixel:	EQU	0Ah	; 3: Pixel data read from screen (R,B,G)
sysvar_audioChannel:	EQU	0Dh	; 1: Audio channel 
sysvar_audioSuccess:	EQU	0Eh	; 1: Audio channel note queued (0 = no, 1 = yes)
sysvar_scrWidth:	EQU	0Fh	; 2: Screen width in pixels
sysvar_scrHeight:	EQU	11h	; 2: Screen height in pixels
sysvar_scrCols:		EQU	13h	; 1: Screen columns in characters
sysvar_scrRows:		EQU	14h	; 1: Screen rows in characters
sysvar_scrColours:	EQU	15h	; 1: Number of colours displayed
sysvar_scrpixelIndex:	EQU	16h	; 1: Index of pixel data read from screen
sysvar_vkeycode:	EQU	17h	; 1: Virtual key code from FabGL
sysvar_vkeydown:	EQU	18h	; 1: Virtual key state from FabGL (0=up, 1=down)
sysvar_vkeycount:	EQU	19h	; 1: Incremented every time a key packet is received
sysvar_rtc:		EQU	1Ah	; 6: Real time clock data
sysvar_spare:		EQU	20h	; 2: Spare, previously used by rtc
sysvar_keydelay:	EQU	22h	; 2: Keyboard repeat delay
sysvar_keyrate:		EQU	24h	; 2: Keyboard repeat reat
sysvar_keyled:		EQU	26h	; 1: Keyboard LED status
sysvar_scrMode:		EQU	27h	; 1: Screen mode
sysvar_rtcEnable:	EQU	28h	; 1: RTC enable flag (0: disabled, 1: use ESP32 RTC)
sysvar_mouseX:		EQU	29h	; 2: Mouse X position
sysvar_mouseY:		EQU	2Bh	; 2: Mouse Y position
sysvar_mouseButtons:	EQU	2Dh	; 1: Mouse button state
sysvar_mouseWheel:	EQU	2Eh	; 1: Mouse wheel delta
sysvar_mouseXDelta:	EQU	2Fh	; 2: Mouse X delta
sysvar_mouseYDelta:	EQU	31h	; 2: Mouse Y delta
	
; Flags for the VPD protocol
;
vdp_pflag_cursor:	EQU	00000001b
vdp_pflag_scrchar:	EQU	00000010b
vdp_pflag_point:	EQU	00000100b
vdp_pflag_audio:	EQU	00001000b
vdp_pflag_mode:		EQU	00010000b
vdp_pflag_rtc:		EQU	00100000b
vdp_pflag_mouse:	EQU	01000000b
; vdp_pflag_buffered:	EQU	10000000b

;
; FatFS structures
; These mirror the structures contained in src_fatfs/ff.h in the MOS project
;
; Object ID and allocation information (FFOBJID)
;
; FFOBJID	.STRUCT
; 	fs:		DS	3	; Pointer to the hosting volume of this object
; 	id:		DS	2	; Hosting volume mount ID
; 	attr:		DS	1	; Object attribute
; 	stat:		DS	1	; Object chain status (b1-0: =0:not contiguous, =2:contiguous, =3:fragmented in this session, b2:sub-directory stretched)
; 	sclust:		DS	4	; Object data start cluster (0:no cluster or root directory)
; 	objsize:	DS	4	; Object size (valid when sclust != 0)
; FFOBJID_SIZE .ENDSTRUCT FFOBJID
; ;
; ; File object structure (FIL)
; ;
; FIL .STRUCT
; 	obj:		.TAG	FFOBJID	; Object identifier
; 	flag:		DS	1	; File status flags
; 	err:		DS	1	; Abort flag (error code)
; 	fptr:		DS	4	; File read/write pointer (Zeroed on file open)
; 	clust:		DS	4	; Current cluster of fpter (invalid when fptr is 0)
; 	sect:		DS	4	; Sector number appearing in buf[] (0:invalid)
; 	dir_sect:	DS	4	; Sector number containing the directory entry
; 	dir_ptr:	DS	3	; Pointer to the directory entry in the win[]
; FIL_SIZE .ENDSTRUCT FIL
; ;
; ; Directory object structure (DIR)
; ; 
; DIR .STRUCT
; 	obj:		.TAG	FFOBJID	; Object identifier
; 	dptr:		DS	4	; Current read/write offset
; 	clust:		DS	4	; Current cluster
; 	sect:		DS	4	; Current sector (0:Read operation has terminated)
; 	dir:		DS	3	; Pointer to the directory item in the win[]
; 	fn:		DS	12	; SFN (in/out) {body[8],ext[3],status[1]}
; 	blk_ofs:	DS	4	; Offset of current entry block being processed (0xFFFFFFFF:Invalid)
; DIR_SIZE .ENDSTRUCT DIR
; ;
; ; File information structure (FILINFO)
; ;
; FILINFO .STRUCT
; 	fsize:		DS 	4	; File size
; 	fdate:		DS	2	; Modified date
; 	ftime:		DS	2	; Modified time
; 	fattrib:	DS	1	; File attribute
; 	altname:	DS	13	; Alternative file name
; 	fname:		DS	256	; Primary file name
; FILINFO_SIZE .ENDSTRUCT FILINFO

; FFOBJID offsets
FFOBJID.fs:       EQU 0    ; Pointer to the hosting volume of this object
FFOBJID.id:       EQU 3    ; Hosting volume mount ID
FFOBJID.attr:     EQU 5    ; Object attribute
FFOBJID.stat:     EQU 6    ; Object chain status
FFOBJID.sclust:   EQU 7    ; Object data start cluster
FFOBJID.objsize:  EQU 11   ; Object size
FFOBJID_SIZE:     EQU 15   ; Total size of FFOBJID structure

; FIL offsets (including FFOBJID fields)
FIL.obj:          EQU 0                  ; Object identifier (FFOBJID fields start here)
FIL.flag:         EQU FFOBJID_SIZE       ; File status flags
FIL.err:          EQU FFOBJID_SIZE + 1   ; Abort flag (error code)
FIL.fptr:         EQU FFOBJID_SIZE + 2   ; File read/write pointer
FIL.clust:        EQU FFOBJID_SIZE + 6   ; Current cluster of fptr
FIL.sect:         EQU FFOBJID_SIZE + 10  ; Sector number appearing in buf[]
FIL.dir_sect:     EQU FFOBJID_SIZE + 14  ; Sector number containing the directory entry
FIL.dir_ptr:      EQU FFOBJID_SIZE + 18  ; Pointer to the directory entry in the win[]
FIL_SIZE:         EQU FFOBJID_SIZE + 21  ; Total size of FIL structure

; DIR offsets (including FFOBJID fields)
DIR.obj:          EQU 0                  ; Object identifier (FFOBJID fields start here)
DIR.dptr:         EQU FFOBJID_SIZE       ; Current read/write offset
DIR.clust:        EQU FFOBJID_SIZE + 4   ; Current cluster
DIR.sect:         EQU FFOBJID_SIZE + 8   ; Current sector
DIR.dir:          EQU FFOBJID_SIZE + 12  ; Pointer to the directory item in the win[]
DIR.fn:           EQU FFOBJID_SIZE + 15  ; SFN (in/out) {body[8],ext[3],status[1]}
DIR.blk_ofs:      EQU FFOBJID_SIZE + 27  ; Offset of current entry block being processed
DIR_SIZE:         EQU FFOBJID_SIZE + 31  ; Total size of DIR structure

; FILINFO offsets
FILINFO.fsize:    EQU 0    ; File size
FILINFO.fdate:    EQU 4    ; Modified date
FILINFO.ftime:    EQU 6    ; Modified time
FILINFO.fattrib:  EQU 8    ; File attribute
FILINFO.altname:  EQU 9    ; Alternative file name
FILINFO.fname:    EQU 22   ; Primary file name
FILINFO_SIZE:     EQU 278  ; Total size of FILINFO structure

;
; Macro for calling the API
; Parameters:
; - function: One of the function numbers listed above
;
			MACRO MOSCALL	function
			LD	A, function
			RST.LIS	08h
			ENDMACRO 	; --- End mos_api.inc ---

; --- Begin macros.inc ---
	; Title:	BBC Basic Interpreter - Z80 version
	;		Useful macros
	; Author:	Dean Belfield
	; Created:	12/05/2023
	; Last Updated:	11/06/2023
	;
	; Modinfo:
	; 11/06/2023:	Modified to run in ADL mode
	; 11/06/2024:   Make compatible with ez80asm by Brandon R. Gates

	MACRO EXREG	rp1, rp2
		PUSH	rp1 
		POP	rp2
	ENDMACRO 

	; MACRO ADD8U_DE	reg
		MACRO ADD8U_DE
		ADD	A, E 
		LD	E, A 
		ADC	A, D
		SUB	E
		LD	D, A 
	ENDMACRO 

	; MACRO ADD8U_HL	reg
	MACRO ADD8U_HL
		ADD	A, L 
		LD	L, A 
		ADC	A, H
		SUB	L
		LD	H, A 
	ENDMACRO 

	MACRO VDU	val
		LD	A, val
		CALL	OSWRCH
	ENDMACRO

	MACRO SET_GPIO	reg, val
		IN0	A, (reg)
		OR	val
		OUT0	(reg), A
	ENDMACRO

	MACRO RES_GPIO	reg, val
		PUSH	BC
		LD	A, val
		CPL
		LD	C, A
		IN0	A, (reg)
		AND	C
		OUT0	(reg), A
		POP	BC
	ENDMACRO
; --- End macros.inc ---

; --- Begin equs.inc ---
;
; Title:	BBC Basic for AGON - Equs
; Author:	Dean Belfield
; Created:	12/05/2023
; Last Updated:	08/06/2023
;
; Modinfo:
; 08/06/2023:	Added SIZEW

			; XREF		STAVAR
			; XREF		ACCS
			
RAM_Top:		EQU		0B0000h	; Initial value of HIMEM
;Stack_Top:		EQU		0000h	; Stack at top
SIZEW:			EQU		3	; Size of a word (3 for ADL mode)
	
; For GPIO
; PA not available on eZ80L92
;
PA_DR:			EQU		96h
PA_DDR:			EQU		97h
PA_ALT1:		EQU		98h
PA_ALT2:		EQU		99h
PB_DR:          	EQU		9Ah
PB_DDR:        	 	EQU		9Bh
PB_ALT1:        	EQU		9Ch
PB_ALT2:        	EQU		9Dh
PC_DR:          	EQU		9Eh
PC_DDR:         	EQU		9Fh
PC_ALT1:        	EQU		A0h
PC_ALT2:        	EQU		A1h
PD_DR:          	EQU		A2h
PD_DDR:			EQU		A3h
PD_ALT1:		EQU		A4h
PD_ALT2:		EQU		A5h
	
GPIOMODE_OUT:		EQU		0	; Output
GPIOMODE_IN:		EQU		1	; Input
GPIOMODE_DIO:		EQU		2	; Open Drain IO
GPIOMODE_SIO:		EQU		3	; Open Source IO
GPIOMODE_INTD:		EQU		4	; Interrupt, Dual Edge
GPIOMODE_ALTF:		EQU		5;	; Alt Function
GPIOMODE_INTAL:		EQU		6	; Interrupt, Active Low
GPIOMODE_INTAH:		EQU		7	; Interrupt, Active High
GPIOMODE_INTFE:		EQU		8	; Interrupt, Falling Edge
GPIOMODE_INTRE:		EQU		9	; Interrupt, Rising Edge
	
; ; Originally in ram.asm
; ;
; OC:			EQU     STAVAR+15*4     ; CODE ORIGIN (O%)
; PC:			EQU     STAVAR+16*4     ; PROGRAM COUNTER (P%)
; VDU_BUFFER:		EQU	ACCS		; Storage for VDU commands

; Originally in main.asm
;
CR:			EQU     0DH
LF:			EQU     0AH
ESC:			EQU     1BH
; --- End equs.inc ---

; --- Begin init.asm ---
;
; Title:	BBC Basic ADL for AGON - Initialisation Code
;		Initialisation Code
; Author:	Dean Belfield
; Created:	12/05/2023
; Last Updated:	26/11/2023
;
; Modinfo:
; 11/07/2023:	Fixed *BYE for ADL mode
; 26/11/2023:	Moved the ram clear routine into here

			; SEGMENT CODE

			; XDEF	_end			
			
			; XREF	_main				; In main.asm
			
			; XREF	RAM_START			; In ram.asm
			; XREF	RAM_END
			
			; .ASSUME	ADL = 1
				
			; INCLUDE	"equs.inc"
			
argv_ptrs_max:		EQU	16				; Maximum number of arguments allowed in argv
			
;
; Start in ADL mode
;
			JP	_start				; Jump to start
;
; The header stuff is from byte 64 onwards
;
_exec_name:		DB	"BBCBASIC.BIN", 0		; The executable name, only used in argv	

			ALIGN	64
			
			DB	"MOS"				; Flag for MOS - to confirm this is a valid MOS command
			DB	00h				; MOS header version 0
			DB	01h				; Flag for run mode (0: Z80, 1: ADL)
;
; And the code follows on immediately after the header
;
_start:			PUSH		AF			; Preserve the rest of the registers
			PUSH		BC
			PUSH		DE
			PUSH		IX
			PUSH		IY

			LD		(_sps), SP 		; Preserve the 24-bit stack pointer (SPS)

			LD		IX, _argv_ptrs		; The argv array pointer address
			PUSH		IX
			CALL		_parse_params		; Parse the parameters
			POP		IX			; IX: argv
			LD		B, 0			;  C: argc
			CALL		_clear_ram
			JP		_main			; Start user code
;
; This bit of code is called from STAR_BYE and returns us safely to MOS
;			
_end:			LD		SP, (_sps)		; Restore the stack pointer

			POP		IY			; Restore the registers
			POP		IX			
			POP		DE
			POP		BC
			POP		AF
			RET					; Return to MOS

;Clear the application memory
;
_clear_ram:		PUSH		BC
			LD		HL, RAM_START		
			LD		DE, RAM_START + 1
			LD		BC, RAM_END - RAM_START - 1
			XOR		A
			LD		(HL), A
			LDIR
			POP		BC
			RET
						
; Parse the parameter string into a C array
; Parameters
; - HL: Address of parameter string
; - IX: Address for array pointer storage
; Returns:
; -  C: Number of parameters parsed
;
_parse_params:		LD	BC, _exec_name
			LD	(IX+0), BC		; ARGV[0] = the executable name
			INC	IX
			INC	IX
			INC	IX
			CALL	_skip_spaces		; Skip HL past any leading spaces
;
			LD	BC, 1			; C: ARGC = 1 - also clears out top 16 bits of BCU
			LD	B, argv_ptrs_max - 1	; B: Maximum number of argv_ptrs
;
_parse_params_1:	
			PUSH	BC			; Stack ARGC	
			PUSH	HL			; Stack start address of token
			CALL	_get_token		; Get the next token
			LD	A, C			; A: Length of the token in characters
			POP	DE			; Start address of token (was in HL)
			POP	BC			; ARGC
			OR	A			; Check for A=0 (no token found) OR at end of string
			RET	Z
;
			LD	(IX+0), DE		; Store the pointer to the token
			PUSH	HL			; DE=HL
			POP	DE
			CALL	_skip_spaces		; And skip HL past any spaces onto the next character
			XOR	A
			LD	(DE), A			; Zero-terminate the token
			INC	IX
			INC	IX
			INC	IX			; Advance to next pointer position
			INC	C			; Increment ARGC
			LD	A, C			; Check for C >= A
			CP	B
			JR	C, _parse_params_1	; And loop
			RET

; Get the next token
; Parameters:
; - HL: Address of parameter string
; Returns:
; - HL: Address of first character after token
; -  C: Length of token (in characters)
;
_get_token:		LD	C, 0			; Initialise length
@@:			LD	A, (HL)			; Get the character from the parameter string
			OR	A			; Exit if 0 (end of parameter string in MOS)
			RET 	Z
			CP	13			; Exit if CR (end of parameter string in BBC BASIC)
			RET	Z
			CP	' '			; Exit if space (end of token)
			RET	Z
			INC	HL			; Advance to next character
			INC 	C			; Increment length
			JR	@B
	
; Skip spaces in the parameter string
; Parameters:
; - HL: Address of parameter string
; Returns:
; - HL: Address of next none-space character
;    F: Z if at end of string, otherwise NZ if there are more tokens to be parsed
;
_skip_spaces:		LD	A, (HL)			; Get the character from the parameter string	
			CP	' '			; Exit if not space
			RET	NZ
			INC	HL			; Advance to next character
			JR	_skip_spaces		; Increment length	

; Storage
;
_sps:			DS	3			; Storage for the stack pointer
_argv_ptrs:		BLKP	argv_ptrs_max, 0	; Storage for the argv array pointers; --- End init.asm ---

; --- Begin eval.asm ---
;
; Title:	BBC Basic Interpreter - Z80 version
;		Expression Evaluation & Arithmetic Module - "EVAL"
; Author:	(C) Copyright  R.T.Russell  1984
; Modified By:	Dean Belfield
; Created:	12/05/2023
; Last Updated:	17/08/2023
;
; Modinfo:
; 07/06/2023:	Modified to run in ADL mode
; 26/06/2023:	Fixed HEX and HEXSTR
; 13/08/2023:	Added INKEY(-n) support (requires MOS 1.04)
; 17/08/2023:	Added binary constants

			; .ASSUME	ADL = 1

			; INCLUDE	"equs.inc"
			; INCLUDE "macros.inc"
			; INCLUDE "mos_api.inc"	; In MOS/src

			; SEGMENT CODE
				
			; XDEF	EXPR
			; XDEF	EXPRN
			; XDEF	EXPRI
			; XDEF	EXPRS
			; XDEF	ITEMI
			; XDEF	LOADN
			; XDEF	LOAD4
			; XDEF	CONS
			; XDEF	LOADS
			; XDEF	SFIX
			; XDEF	VAL0
			; XDEF	SEARCH
			; XDEF	SWAP
			; XDEF	TEST
			; XDEF	DECODE
			; XDEF	HEXSTR
			; XDEF	STR
			; XDEF	ZERO
			; XDEF	PUSHS
			; XDEF	POPS
			; XDEF	COMMA
			; XDEF	BRAKET
			; XDEF	NXT
			; XDEF	COUNT0
				
			; XREF	ADVAL
			; XREF	FN_EX
			; XREF	POINT
			; XREF	USR
			; XREF	SYNTAX
			; XREF	ERROR_
			; XREF	CHECK
			; XREF	GETVAR
			; XREF	LISTON
			; XREF	RANGE
			; XREF	FPP
			; XREF	GETCSR
			; XREF	CHANEL
			; XREF	OSSTAT
			; XREF	OSBGET
			; XREF	LOMEM
			; XREF	HIMEM
			; XREF	PAGE_
			; XREF	TOP
			; XREF	ERL
			; XREF	ERR
			; XREF	COUNT
			; XREF	OSOPEN
			; XREF	GETEXT
			; XREF	GETPTR
			; XREF	GETIME
			; XREF	GETIMS
			; XREF	LEXAN2
			; XREF	RANDOM
			; XREF	STORE5
			; XREF	GETSCHR
			; XREF	OSRDCH
			; XREF	OSKEY
			; XREF	INKEY1
			; XREF	EXTERR
;
; BINARY FLOATING POINT REPRESENTATION:
;    32 BIT SIGN-MAGNITUDE NORMALIZED MANTISSA
;     8 BIT EXCESS-128 SIGNED EXPONENT
;    SIGN BIT REPLACES MANTISSA MSB (IMPLIED "1")
;    MANTISSA=0 & EXPONENT=0 IMPLIES VALUE IS ZERO.
;
; BINARY INTEGER REPRESENTATION:
;    32 BIT 2'S-COMPLEMENT SIGNED INTEGER
;     "EXPONENT" BYTE = 0 (WHEN PRESENT)
;
; NORMAL REGISTER ALLOCATION: MANTISSA - HLH'L'
;                             EXPONENT - C
;

;
; Table of addresses for functions
;
FUNTOK:			EQU	8DH			; First token number
;
FUNTBL:			DW24	DECODE			; Line number
			DW24	OPENIN			; OPENIN
			DW24	PTR_EV			; PTR
			DW24	PAGEV			; PAGE
			DW24	TIMEV			; TIME
			DW24	LOMEMV			; LOMEM
			DW24	HIMEMV			; HIMEM
			DW24	ABSV			; ABS
			DW24	ACS			; ACS
			DW24	ADVAL			; ADVAL
			DW24	ASC			; ASC
			DW24	ASN			; ASN
			DW24	ATN			; ATN
			DW24	BGET			; BGET
			DW24	COS			; COS
			DW24	COUNTV			; COUNT
			DW24	DEG			; DEG
			DW24	ERLV			; ERL
			DW24	ERRV			; ERR
			DW24	EVAL_			; EVAL
			DW24	EXP			; EXP
			DW24	EXT			; EXT
			DW24	ZERO			; FALSE
			DW24	FN_EX			; FN
			DW24	GET			; GET
			DW24	INKEY			; INKEY
			DW24	INSTR			; INSTR(
			DW24	INT_			; INT
			DW24	LEN			; LEN
			DW24	LN			; LN
			DW24	LOG			; LOG
			DW24	NOTK			; NOT
			DW24	OPENUP			; OPENUP
			DW24	OPENOT			; OPENOUT
			DW24	PI			; PI
			DW24	POINT			; POINT(
			DW24	POS			; POS
			DW24	RAD			; RAD
			DW24	RND			; RND
			DW24	SGN			; SGN
			DW24	SIN			; SIN
			DW24	SQR			; SQR
			DW24	TAN			; TAN
			DW24	TOPV			; TO(P)
			DW24	TRUE			; TRUE
			DW24	USR			; USR
			DW24	VAL			; VAL
			DW24	VPOS			; VPOS
			DW24	CHRS			; CHRS
			DW24	GETS			; GETS
			DW24	INKEYS			; INKEYS
			DW24	LEFTS			; LEFTS(
			DW24	MIDS			; MIDS(
			DW24	RIGHTS			; RIGHTS(
			DW24	STRS			; STR$
			DW24	STRING_			; STRINGS(
			DW24	EOF			; EOF
;
FUNTBL_END:		EQU	$
; TCMD:			EQU     FUNTOK+(FUNTBL_END-FUNTBL)/3
TCMD_EV:			EQU     FUNTBL_END-FUNTBL/3+FUNTOK ; reorder because ez80asm doesn't do order of operations

ANDK:			EQU     80H
DIVK:			EQU     81H
EORK:			EQU     82H
MODK:			EQU     83H
ORK:			EQU     84H
;
SOPTBL:			DW24	SLE			; <= (STRING)
			DW24	SNE			; <>
			DW24	SGE			; >=
			DW24	SLT			; <
			DW24	SEQ			; =
			DW24	SGT			; >
;
; EXPR - VARIABLE-TYPE EXPRESSION EVALUATION
;     Expression type is returned in A'F':
;        Numeric - A' bit 7=0, F' sign bit cleared.
;         String - A' bit 7=1, F' sign bit set.
; Floating-point or integer result returned in HLH'L'C
; Integer result denoted by C=0 and HLH'L' non-zero.
; String result returned in string accumulator, DE set.
;
; Hierarchy is: (1) Variables, functions, constants, bracketed expressions.
;               (2) ^
;               (3) * / MOD DIV
;               (4) + -
;               (5) = <> <= >= > <
;               (6) AND
;               (7) EOR OR

;
; Level 7: EOR and OR
;
EXPR:			CALL    EXPR1			; Get first operator by calling Level 6
EXPR0A:			CP      EORK            	; Is operator EOR?
			JR      Z,EXPR0B		; Yes, so skip to next bit
			CP      ORK			; Is operator OR
			RET     NZ			; No, so return
;
EXPR0B:			CALL    SAVE_EV            	; Save first operand
			CALL    EXPR1           	; Get second operand
			CALL    DOIT            	; Do the operation
			JR      EXPR0A          	; And continue
;
; Level 6: AND
;
EXPR1:			CALL    EXPR2			; Get first operator by calling Level 5
EXPR1A:			CP      ANDK			; Is operator AND?
			RET     NZ			; No, so return
			CALL    SAVE_EV			; Save first operand
			CALL    EXPR2			; Get second operand
			CALL    DOIT			; Do the operation
			JR      EXPR1A			; And continue
;
; Level 5: Comparisons
;
EXPR2:			CALL    EXPR3			; Get first operator by calling Level 4
			CALL    RELOP?			; Is it ">", "=" or "<"?
			RET     NZ			; No, so return
			LD      B,A			; Store the first operator in B
			INC     IY              	; Bump over operator
			CALL    NXT			; 
			CALL    RELOP?          	; Is it a compound operator?
			JR      NZ,EXPR2B		; No, so skip next bit
			INC     IY			; Bump over operator
			CP      B			; Compare with first
			JP      Z,SYNTAX        	; Trap illegal combinations ">>", "==", "<<" (but not "><", "=>", "=<")
			ADD     A,B			
			LD      B,A			; B: Unique code for the compound operator
EXPR2B:			LD      A,B			; A: Code for the operator/compound operator
			EX      AF,AF'
			JP      M,EXPR2S		; If it is a string, then branch here to handle it
			EX      AF,AF'
			SUB     4
			CP      '>'-4
			JR      NZ,EXPR2C
			ADD     A,2
EXPR2C:			CALL    SAVE1
			CALL    EXPR3
			CALL    DOIT            	; NB: Must NOT be "JP DOIT"
			RET
;
EXPR2S:			EX      AF,AF'			; Handle string comparisons
			DEC     A
			AND     7
			CALL    PUSHS           	; Save string on the stack
			PUSH    AF              	; Save the operator
			CALL    EXPR3           	; Get the second string
			EX      AF,AF'
			JP      P,TYPE_EV_
			POP     AF
			LD      C,E             	; Length of string #2
			POP     DE
			LD      HL,0
			ADD     HL,SP
			LD      B,E             	; Length of string #1
			PUSH    DE
			LD      DE,ACCS
			EX      DE,HL
			CALL    DISPT2
			POP     DE
			EX      DE,HL
			LD	A,L
			LD	HL,0
			LD	L,A
			ADD     HL,SP
			LD      SP,HL
			EX      DE,HL
			XOR     A               	; Numeric marker
			LD      C,A             	; Integer marker
			EX      AF,AF'
			LD      A,(IY)
			RET
;
; Level 4: + and -
;
EXPR3:			CALL    EXPR4			; Get first operator by calling Level 3
EXPR3A:			CP      '-'			; Is it "-"?
			JR      Z,EXPR3B		; Yes, so skip the next bit
			CP      '+'			; Is it "+"?
			RET     NZ			; No, so return
			EX      AF,AF'			; Get the type
			JP      M,EXPR3S		; Branch here if string
			EX      AF,AF'
EXPR3B:			CALL    SAVE_EV			; Save the first operator
			CALL    EXPR4			; Fetch the second operator
			CALL    DOIT			; Do the operation
			JR      EXPR3A			; And continue
;
EXPR3S:			EX      AF,AF'			; Handle string concatenation
			INC     IY              	; Bump past the "+"
			CALL    PUSHS           	; Save the string on the stack
			CALL    EXPR4           	; Fetch the second operator
			EX      AF,AF'
			JP      P,TYPE_EV_			; If it is not a string, then Error: "Type mismatch"
			LD	BC, 0			; Clear BC
			LD      C,E             	; C: Length of the second string
			POP     DE
			PUSH    DE
			LD      HL,ACCS
; BEGIN MISSING FROM BINARY
			; LD	A,E			;  E: Length of the first string
			; LD      DE,ACCS
			; LD	E,A 			; DE: Pointer to the end of the first string
; END MISSING FROM BINARY
			LD		D,H ; ADDED FROM BINARY
			LD      A,C			
			OR      A
			JR      Z,EXP3S3
			LD      L,A             	; Source
			ADD     A,E
			LD      E,A             	; Destination
			LD      A,19
			JP      C,ERROR_         	; A carry indicates string > 255 bytes, so Error: "String too long"
			PUSH    DE
			DEC     E
			DEC     L
			LDDR                    	; Copy
			POP     DE
EXP3S3:			EXX
			POP     BC
			CALL    POPS            	; Restore from stack
			EXX
			OR      80H             	; Flag as a string
			EX      AF,AF'
			LD      A,(IY)			; Fetch the next character
			JR      EXPR3A			; And continue
;
; Level 3: * / MOD DIV
;
EXPR4:			CALL    EXPR5			; Get first operator by calling Level 2
EXPR4A:			CP      '*'			; "*" is valid
			JR      Z,EXPR4B
			CP      '/'			; "/" is valid
			JR      Z,EXPR4B
			CP      MODK			; MOD token is valid
			JR      Z,EXPR4B
			CP      DIVK			; DIV token is valid
			RET     NZ			; And return if it is anything else
EXPR4B:			CALL    SAVE_EV
			CALL    EXPR5
			CALL    DOIT
			JR      EXPR4A
;
; Level 2: ^
;
EXPR5:			CALL    ITEM			; Get variable
			OR      A               	; Test type
			EX      AF,AF'          	; Save type 
EXPR5A:			CALL    NXT			; Skip spaces
			CP      '^'			; Is the operator "^"?
			RET     NZ			; No, so return
			CALL    SAVE_EV			; Save first operand
			CALL    ITEM			; Get second operand
			OR      A			; Test type
			EX      AF,AF'			; Save type
			CALL    DOIT			; Do the operation
			JR      EXPR5A			; And continue
;
; Evaluate a numeric expression
;
EXPRN:			CALL    EXPR			; Evaluate expression
			EX      AF,AF'			; Get the type
			RET     P			; And return if it is a number
			JR      TYPE_EV_			; Otherwise Error: "Type mismatch"
;
; Evaluate a fixed-point expression 
;
EXPRI:			CALL    EXPR			; Evaluate the expression
			EX      AF,AF'			; Get the type
			JP      P,SFIX			; If it is numeric, then convert to fixed-point notation
			JR      TYPE_EV_			; Otherwise Error: "Type mismatch"
;	
; Evaluate a string expression
;	
EXPRS:			CALL    EXPR			; Evaluate the expression
			EX      AF,AF'			; Get the type
			RET     M			; And return if it is a string
			JR      TYPE_EV_			; Otherwise Error: "Type mismatch"
;
; Get a numeric variable
;
ITEMN:			CALL    ITEM			; Get the variable
			OR      A			; Test the type
			RET     P			; And return if it is a number
			JR      TYPE_EV_			; Otherwise Error: "Type mismatch"
;
; Get a fixed-point variable 
;
ITEMI:			CALL    ITEM			; Get the variable
			OR      A			; Test the type
			JP      P,SFIX			; If it is numeric, then convert to fixed-point notation
			JR      TYPE_EV_			; Otherwise Error: "Type mismatch"
;
; Get a string variable 
;
ITEMS:			CALL    ITEM			; Get the variable
			OR      A			; Test the type
			RET     M			; If it is a string, then return
;							; Otherwise
TYPE_EV_:			LD      A,6			; Error: "Type mismatch"
			JP      ERROR_           	
;
; Evaluate a bracketed expression
;
ITEM1:			CALL    EXPR            	; Evaluate the expression
			CALL    BRAKET			; Check for closing bracket
			EX      AF,AF'
			RET
;
; HEX - Get hexadecimal constant.
;   Inputs: ASCII string at (IY)
;  Outputs: Integer result in H'L'HL, C=0, A7=0.
;           IY updated (points to delimiter)
;
HEX:			CALL    ZERO			; Set result to 0
			CALL    HEXDIG			; Fetch the character from IY
			JR      C,BADHEX		; If invalid HEX character, then Error: "Bad HEX"
HEX1:			INC     IY			; Move pointer to next character
			AND     0FH			; Clear the top nibble
			LD      B,4			; Loop counter
;
HEX2:			EXX				; Shift the result left B (4) times. This makes
			ADD.S   HL,HL			; space for the incoming nibble in the least significant 4 bits
			EXX				; .
			ADC.S   HL,HL			; .
			DJNZ    HEX2			; And loop
			EXX
			OR      L			; OR in the digit
			LD      L,A
			EXX
;
			CALL    HEXDIG			; Fetch the next character
			JR      NC,HEX1			; If it is a HEX digit then loop
			XOR     A			; Clear A
			RET
;
BADHEX:			LD      A,28
			JP      ERROR_          	; Error: "Bad HEX"
;
; BIN - Get binary constant.
;   Inputs: ASCII string at (IY)
;  Outputs: Integer result in H'L'HL, C=0, A7=0.
;           IY updated (points to delimiter)
;
BIN:			CALL    ZERO			; Set result to 0
			CALL	BINDIG			; Fetch the character from IY
			JR	C,BADBIN		; If invalid BIN character then Error: "Bad Binary"
BIN1:			INC	IY			; Move pointer to next character
			RRCA				; Bit 0 of ASCII '0' is 0, and ASCII '1' is 1, so shift that bit into carry
			EXX				; 
			ADC.S	HL,HL			; And shift back into into H'L'HL (note the ADC)
			EXX
			ADC.S	HL,HL
			CALL	BINDIG			; Fetch the next character
			JR	NC,BIN1
			XOR	A			; Clear A
			RET
;
BADBIN:			LD	A, 28			; Error: "Bad Binary" - reuses same error code as Bad HEX
			CALL	EXTERR
			DB	"Bad Binary", 0
;
; MINUS - Unary minus.
;   Inputs: IY = text pointer
;  Outputs: Numeric result, same type as argument.
;           Result in H'L'HLC
;
MINUS:			CALL    ITEMN			; Get the numeric argument
MINUS0:			DEC     C			; Check exponent (C)
			INC     C			; If it is zero, then it's either a FP zero or an integer
			JR      Z,NEGATE_EV        	; So do an integer negation
;
			LD      A,H			; Do a FP negation by 
			XOR     80H             	; Toggling the sign bit (H)
			LD      H,A
			XOR     A               	; Numeric marker
			RET
;
NEGATE_EV:			EXX				; This section does a two's complement negation on H'L'HLC
			LD      A,H			; First do a one's complement by negating all the bytes
			CPL
			LD      H,A
			LD      A,L
			CPL
			LD      L,A
			EXX
			LD      A,H
			CPL
			LD      H,A
			LD      A,L
			CPL
			LD      L,A
ADD1:			EXX				; Then add 1
			INC     HL			
			LD      A,H
			OR      L
			EXX
			LD      A,0             	; Numeric marker
			RET     NZ
			INC     HL
			RET
;
; ITEM - VARIABLE TYPE NUMERIC OR STRING ITEM.
; Item type is returned in A:  Bit 7=0 numeric.
;                              Bit 7=1 string.
; Numeric item returned in HLH'L'C.
; String item returned in string accumulator,
;   DE addresses byte after last (E=length).
;
ITEM:			CALL    CHECK			; Check there's at least a page of free memory left and Error: "No room" if not
			CALL    NXT			; Skip spaces
			INC     IY			; Move to the prefix character
			CP      '&'			; If `&`
			JP      Z,HEX           	; Then get a HEX constant
			CP	'%'			; If '%'
			JR	Z,BIN			; Then get a BINARY constant
			CP      '-'			; If `-`
			JR      Z,MINUS         	; Then get a negative number
			CP      '+'			; If `+`
			JP      Z,ITEMN         	; Then just fetch the number (unary plus)
			CP      '('			; If `(`
			JP      Z,ITEM1         	; Start of a bracketed expression
			CP      34			; If `"`
			JR      Z,CONS          	; Start of a string constant
			CP      TCMD_EV			; Is it out of range of the function table?
			JP      NC,SYNTAX       	; Error: "Syntax Error"
			CP      FUNTOK			; If it is in range, then 
			JP      NC,DISPAT       	; It's a function
			DEC     IY			
			CP      ':'
			JR      NC,ITEM2		; VARIABLE?
			CP      '0'
			JP      NC,CON			; NUMERIC CONSTANT
			CP      '.'
			JP      Z,CON			; NUMERIC CONSTANT
ITEM2:			CALL    GETVAR			; VARIABLE
			JR      NZ,NOSUCH
			OR      A
			JP      M,LOADS			; STRING VARIABLE
LOADN:			OR      A
			JR      Z,LOAD1			; BYTE VARIABLE
			LD      C,0
			BIT     0,A
			JR      Z,LOAD4			; INTEGER VARIABLE
LOAD5:			LD      C,(IX+4)
LOAD4:			EXX
			LD	HL, 0			; TODO: Optimise
			LD      L,(IX+0)
			LD      H,(IX+1)
			EXX
			LD	HL, 0			; TODO: Optimise
			LD      L,(IX+2)
			LD      H,(IX+3)
			RET
;
LOAD1:			LD      HL,0
			EXX
			LD      HL,0			; TODO: Optimise
			LD      L,(IX+0)
			EXX
			LD      C,H
			RET
;
NOSUCH:			JP      C,SYNTAX
			LD      A,(LISTON)
			BIT     5,A
			LD      A,26
			JR      NZ,ERROR0_EV		; Throw "No such variable"
NOS1:			INC     IY
			CALL    RANGE
			JR      NC,NOS1
			LD      IX,PC
			XOR     A
			LD      C,A
			JR      LOAD4
;
;CONS - Get string constant from ASCII string.
;   Inputs: ASCII string at (IY)
;  Outputs: Result in string accumulator.
;           D = MS byte of ACCS, E = string length
;           A7 = 1 (string marker)
;           IY updated
;
CONS:			LD      DE,ACCS			; DE: Pointer to the string accumulator
CONS3:			LD      A,(IY)			; Fetch the first character and
			INC     IY			; Increment the pointer
			CP      '"'			; Check for start quote
			JR      Z,CONS2			; Yes, so jump to the bit that parses the string
;
CONS1:			LD      (DE),A			; Store the character in the string accumulator
			INC     E			; Increment the string accumulator pointer
			CP      CR			; Is it CR
			JR      NZ,CONS3		; No, so keep looping
;
			LD      A,9
ERROR0_EV:			JP      ERROR_           	; Throw error "Missing '"'
;
CONS2:			LD      A,(IY)			; Fetch the next character
			CP      '"'			; Check for end quote?
			INC     IY			; Increment the pointer
			JR      Z,CONS1			; It is the end of string marker so jump to the end routine
			DEC     IY			; 
			LD      A,80H           	; String marker
			RET
;
;CON - Get unsigned numeric constant from ASCII string.
;   Inputs: ASCII string at (IY).
;  Outputs: Variable-type result in HLH'L'C
;           IY updated (points to delimiter)
;           A7 = 0 (numeric marker)
;
CON:			PUSH    IY
			POP     IX
			LD      A,36
			CALL    FPP
			JR      C,ERROR0_EV
			PUSH    IX
			POP     IY
			XOR     A
			RET
;
LOADS:			LD      DE,ACCS			; Where to store the string
			RRA
			JR      NC,LOADS2       	; Skip if it is a fixed string
;
			EXX				; This block was a call to LOAD4
			LD      L,(IX+0)		; The length of the string currently stored in the allocated space
			LD      H,(IX+1)		; The maximum original string length
			EXX
			LD	HL,(IX+2)		; Address of the string (24-bit)
;
			EXX
			LD      A,L
			EXX
			OR      A
			LD	BC,0			; BC: Number of bytes to copy
			LD      C,A
			LD      A,80H           	; String marker
			RET     Z
			LDIR
			RET
LOADS2:			LD      A,(HL)
			LD      (DE),A
			INC     HL
			CP      CR
			LD      A,80H           	; String marker
			RET     Z
			INC     E
			JR      NZ,LOADS2
			RET                     	; Return null string
;
;VARIABLE-TYPE FUNCTIONS:
;
;Result returned in HLH'L'C (floating point)
;Result returned in HLH'L' (C=0) (integer)
;Result returned in string accumulator & DE (string)
;All registers destroyed.
;IY (text pointer) updated.
;Bit 7 of A indicates type: 0 = numeric, 1 = string.
;
;POS - horizontal cursor position.
;VPOS - vertical cursor position.
;EOF - return status of file.
;BGET - read byte from file.
;INKEY - as GET but wait only n centiseconds.
;GET - wait for keypress and return ASCII value.
;GET(n) - input from Z80 port n.
;ASC - ASCII value of string.
;LEN - length of string.
;LOMEM - location of dynamic variables.
;HIMEM - top of available RAM.
;PAGE - start of current text page.
;TOP - address of first free byte after program.
;ERL - line number where last error occurred.
;ERR - number of last error.
;COUNT - number of printing characters since CR.
;Results are integer numeric.
;
POS:			CALL    GETCSR			; Return the horizontal cursor position
			EX      DE,HL			;  L: The X cursor position
			JP      COUNT1			; Return an 8-bit value
;			
VPOS:			CALL    GETCSR			; Return the vertical cursor position
			JP      COUNT1			; Return an 8-bit value
;			
EOF:			CALL    CHANEL			; Check for EOF
			CALL    OSSTAT
			JP      Z,TRUE			; Yes, so return true
			JP      ZERO			; Otherwise return false (zero)
;			
BGET:			CALL    CHANEL          	; Channel number
			CALL    OSBGET
			LD      L,A
			JP      COUNT0			; Return an 8-bit value
;			
INKEY:			CALL    ITEMI			; Get the argument
			BIT	7, H			; Check the sign
			EXX				; HL: The argument
			JP	NZ, INKEYM		; It's negative, so do INKEY(-n)
			CALL	INKEY0 			; Do INKEY(n)
			JR      ASC0			; Return a numeric value
;			
GET:			CALL    NXT			; Skip whitespace
			CP      '('			; Is it GET(
			JR      NZ,GET0			; No, so get a keyboard character
			CALL    ITEMI           	; Yes, so fetch the port address
			EXX
			LD      B,H			; BC: The port address
			LD      C,L
			IN      L,(C)           	;  L: Input from port BC
			JR      COUNT0			; Return an 8-bit value
;
GET0:			CALL    GETS			; Read the keyboard character			
			JR      ASC1			; And return the value
;			
ASC:			CALL    ITEMS			; Get the string argument argument
ASC0:			XOR     A			; Quickly check the length of the string in ACCS
			CP      E			; Is the pointer 0
			JP      Z,TRUE          	; Yes, so return -1 as it is a null string
ASC1:			LD      HL,(ACCS)		;  L: The first character (H will be discarded in COUNT0
			JR      COUNT0			; An 8-bit value
;
LEN:			CALL    ITEMS			; Get the string argument
			EX      DE,HL			; HL: Pointer into ACCS
			JR      COUNT0			; Return L
;			
LOMEMV:			LD      HL,(LOMEM)		; Return the LOMEM system variable
			LD	A, (LOMEM+2)
			JR      COUNT2			; A 24-bit value
;			
HIMEMV:			LD      HL,(HIMEM)		; Return the HIMEM system variable
			LD	A, (HIMEM+2)
			JR      COUNT2			; A 24-bit value
;			
PAGEV:			LD    	HL,(PAGE_)		; Return the PAGE system variable
			LD	A, (PAGE_+2)		; A 24-bit value
			JR      COUNT2
;			
TOPV:			LD      A,(IY)			; Return the TOP system variable
			INC     IY              	; Skip "P"
			CP      'P'
			JP      NZ,SYNTAX       	; Throw "Syntax Error"
			LD      HL,(TOP)
			LD	A, (TOP+2)
			JR      COUNT2
;			
ERLV:			LD      HL,(ERL)		; Return the error line
			JR      COUNT1			; A 16-bit value
;			
ERRV:			LD      HL,(ERR)		; Return the error value
			JR      COUNT0			; An 8-bit value
;			
COUNTV:			LD      HL,(COUNT)		; Return the print position sysvar

COUNT0:			LD      H,0			; Return L
COUNT1:			EXX				; Return HL
			XOR     A
			LD      C,A             	; Integer marker
			LD      H,A
			LD      L,A
			RET
COUNT2:			EXX
			LD	L,A 
			XOR	A 
			LD	C,A			; Integer marker
			LD	H,A 
			RET
;
;OPENIN - Open a file for reading.
;OPENOT - Open a file for writing.
;OPENUP - Open a file for reading or writing.
;Result is integer channel number (0 if error)
;
OPENOT:			XOR     A			; Open for writing
			JR	OPENIN_1
;			
OPENUP:			LD      A,2			; Open for reading / writing
			JR	OPENIN_1
;
OPENIN:			LD      A,1			; Open for reading
;
OPENIN_1:		PUSH    AF              	; Save OPEN type
			CALL    ITEMS           	; Fetch the filename
			LD      A,CR
			LD      (DE),A
			POP     AF              	; Restore the OPEN type
			ADD     A,-1            	; Affect the flags
			LD      HL,ACCS
			CALL    OSOPEN			; Call the OS specific OPEN routine in patch.asm
			LD      L,A			; L: Channel number
			JR      COUNT0			; Return channel number to BASIC
;
;EXT - Return length of file.
;PTR_EV - Return current file pointer.
;Results are integer numeric.
;
EXT:			CALL    CHANEL
			CALL    GETEXT
			JR      TIME0
;
PTR_EV:			CALL    CHANEL
			CALL    GETPTR
			JR      TIME0
;
;TIME - Return current value of elapsed time.
;Result is integer numeric.
;
TIMEV:			LD      A,(IY)
			CP      '$'
			JR      Z,TIMEVS
			CALL    GETIME
TIME0:			PUSH    DE
			EXX
			POP     HL
			XOR     A
			LD      C,A
			RET
;
;TIME$ - Return date/time string.
;Result is string
;
TIMEVS:			INC     IY              ;SKIP $
			CALL    GETIMS
			LD      A,80H           ;MARK STRING
			RET
;
;String comparison:
;
SLT:			CALL    SCP
			RET     NC
			JR      TRUE
;
SGT:			CALL    SCP
			RET     Z
			RET     C
			JR      TRUE
;
SGE:			CALL    SCP
			RET     C
			JR      TRUE
;
SLE:			CALL    SCP
			JR      Z,TRUE
			RET     NC
			JR      TRUE
;
SNE:			CALL    SCP
			RET     Z
			JR      TRUE
;
SEQ:			CALL    SCP
			RET     NZ
TRUE:			LD      A,-1
			EXX
			LD      H,A
			LD      L,A
			EXX
			LD      H,A
			LD      L,A
			INC     A
			LD      C,A
			RET
;
;PI - Return PI (3.141592654)
;Result is floating-point numeric.
;
PI:			LD      A,35
			JR      FPP1
;
;ABS - Absolute value
;Result is numeric, variable type.
;
ABSV:			LD      A,16
			JR      FPPN
;
;NOT - Complement integer.
;Result is integer numeric.
;
NOTK:			LD      A,26
			JR      FPPN
;
;DEG - Convert radians to degrees
;Result is floating-point numeric.
;
DEG:			LD      A,21
			JR      FPPN
;
;RAD - Convert degrees to radians
;Result is floating-point numeric.
;
RAD:			LD      A,27
			JR      FPPN
;
;SGN - Return -1, 0 or +1
;Result is integer numeric.
;
SGN:			LD      A,28
			JR      FPPN
;
;INT - Floor function
;Result is integer numeric.
;
INT_:			LD      A,23
			JR      FPPN
;
;SQR - square root
;Result is floating-point numeric.
;
SQR:			LD      A,30
			JR      FPPN
;
;TAN - Tangent function
;Result is floating-point numeric.
;
TAN:			LD      A,31
			JR      FPPN
;
;COS - Cosine function
;Result is floating-point numeric.
;
COS:			LD      A,20
			JR      FPPN
;
;SIN - Sine function
;Result is floating-point numeric.
;
SIN:			LD      A,29
			JR      FPPN
;
;EXP - Exponential function
;Result is floating-point numeric.
;
EXP:			LD      A,22
			JR      FPPN
;
;LN - Natural log.
;Result is floating-point numeric.
;
LN:			LD      A,24
			JR      FPPN
;
;LOG - base-10 logarithm.
;Result is floating-point numeric.
;
LOG:			LD      A,25
			JR      FPPN
;
;ASN - Arc-sine
;Result is floating-point numeric.
;
ASN:			LD      A,18
			JR      FPPN
;
;ATN - arc-tangent
;Result is floating-point numeric.
;
ATN:			LD      A,19
			JR      FPPN
;
;ACS - arc-cosine
;Result is floating point numeric.
;
ACS:			LD      A,17
FPPN:			PUSH    AF
			CALL    ITEMN
			POP     AF
FPP1:			CALL    FPP
			JP      C,ERROR_
			XOR     A
			RET
;
;SFIX - Convert to fixed-point notation
;
SFIX:			LD      A,38
			JR      FPP1
;
;SFLOAT - Convert to floating-point notation
;
SFLOAT:			LD      A,39
			JR      FPP1
;
;VAL - Return numeric value of string.
;Result is variable type numeric.
;
VAL:			CALL    ITEMS
VAL0:			XOR     A
			LD      (DE),A
			LD      IX,ACCS
			LD      A,36
			JR      FPP1
;
;EVAL - Pass string to expression evaluator.
;Result is variable type (numeric or string).
;
EVAL_:			CALL    ITEMS
			LD      A,CR
			LD      (DE),A
			PUSH    IY
			LD      DE,ACCS
			LD      IY,ACCS
			LD      C,0
			CALL    LEXAN2          ;TOKENISE
			LD      (DE),A
			INC     DE
			XOR     A
			CALL    PUSHS           ;PUT ON STACK
			LD      IY,SIZEW	;WAS 2
			ADD     IY,SP
			CALL    EXPR
			POP     IY
			ADD     IY,SP
			LD      SP,IY           ;ADJUST STACK POINTER
			POP     IY
			EX      AF,AF'
			RET
;
;RND - Random number function.
; RND gives random integer 0-&FFFFFFFF
; RND(-n) seeds random number & returns -n.
; RND(0) returns last value in RND(1) form.
; RND(1) returns floating-point 0-0.99999999.
; RND(n) returns random integer 1-n.
;
RND:			LD      IX,RANDOM
			CALL    NXT
			CP      '('
			JR      Z,RND5          ;ARGUMENT FOLLOWS
			CALL    LOAD5
RND1:			RR      C
			LD      B,32
RND2:			EXX                     ;CALCULATE NEXT
			ADC.S   HL,HL
			EXX
			ADC.S   HL,HL
			BIT     3,L
			JR      Z,RND3
			CCF
RND3:			DJNZ    RND2
RND4:			RL      C               ;SAVE CARRY
			CALL    STORE5          ;STORE NEW NUMBER
			XOR     A
			LD      C,A
			RET
RND5:			CALL    ITEMI
			LD      IX,RANDOM
			BIT     7,H             ;NEGATIVE?
			SCF
			JR      NZ,RND4         ;SEED
			CALL    TEST
			PUSH    AF
			CALL    SWAP
			EXX
			CALL    LOAD5
			CALL    NZ,RND1         ;NEXT IF NON-ZERO
			EXX                     ;SCRAMBLE (CARE!)
			LD      C,7FH
RND6:			BIT     7,H             ;FLOAT
			JR      NZ,RND7
			EXX
			ADD.S   HL,HL
			EXX
			ADC.S   HL,HL
			DEC     C
			JR      NZ,RND6
RND7:			RES     7,H             ;POSITIVE 0-0.999999
			POP     AF
			RET     Z               ;ZERO ARGUMENT
			EXX
			LD      A,E
			DEC     A
			OR      D
			EXX
			OR      E
			OR      D
			RET     Z               ;ARGUMENT=1
			LD      B,0             ;INTEGER MARKER
			LD      A,10
			CALL    FPP             ;MULTIPLY
			JP      C,ERROR_
			CALL    SFIX
			JP      ADD1
;
; INSTR - String search.
; Result is integer numeric.
;
INSTR:			CALL    EXPRSC			; Get the first string expression
			CALL    PUSHS           	; Push the string onto the stack
			CALL    EXPRS           	; Get the second string expression
			POP     BC			;  C: String length, B: Value of A before PUSHS was called
			LD      HL,0
			ADD     HL,SP           	; HL: Pointer to main string
			PUSH    BC              	;  C: Main string length
			LD      B,E             	;  B: Sub-string length
			CALL    NXT			; Skip whitespace
			CP      ','			; Check if there is a comma for the third parameter
			LD      A,0			;  A: Default start position in string
			JR      NZ,INSTR1		; No, so skip the next bit
			INC     IY              	; Skip the comma
			PUSH    BC              	; Save the lengths
			PUSH    HL              	; Save the pointer to the main string
			CALL    PUSHS			; Push the string onto the stack
			CALL    EXPRI			; Get the third (numeric) parameter - the starting position
			POP     BC			;  C: String length, B: Value of A before PUSHS was called (discarded)
			CALL    POPS			; Pop the string off the stack
			POP     HL              	; Restore the pointer to the main string
			POP     BC              	; Restore the lengths
			EXX
			LD      A,L			; A: The start position in the  string
			EXX
			OR      A			; Set the flags
			JR      Z,INSTR1		; If it is zero, then skip
			DEC     A
INSTR1:			LD      DE,ACCS         	; DE: Pointer to the sub string
			CALL    SEARCH			; Do the search
			POP     DE
			JR      Z,INSTR2        	; NB: Carry cleared
			SBC     HL,HL
			ADD     HL,SP
INSTR2:			SBC     HL,SP
			EX      DE,HL
			LD	A,L
			LD      HL,0
			LD	L,A
			ADD     HL,SP
			LD      SP,HL
			EX      DE,HL
			CALL    BRAKET			; Check for closing bracket
			JP      COUNT1			; Return a numeric integer
;
; SEARCH - Search string for sub-string
;    Inputs: Main string at HL length C
;            Sub-string  at DE length B
;            Starting offset A
;   Outputs: NZ - not found
;            Z - found at location HL-1
;            Carry always cleared
;
SEARCH:			PUSH    BC			; Add the starting offset to HL
			LD      BC,0
			LD      C,A
			ADD     HL,BC           	; New start address
			POP     BC
			SUB     C			; If the starting offset > main string length, then do nothing
			JR      NC,SRCH4
			NEG
			LD      C,A             	; Remaining length
;
SRCH1:			PUSH    BC
			LD	A,C
			LD	BC,0
			LD	C,A
			LD      A,(DE)
			CPIR                    	; Find the first character
			LD      A,C
			POP     BC
			JR      NZ,SRCH4
			LD      C,A
;
; This block of four instructions was commented as a bug fix by R.T.Russell
;
			DEC     B			; Bug fix
			CP      B			; Bug fix
			INC     B			; Bug fix
			JR      C,SRCH4			; Bug fix
;			
			PUSH    BC
			PUSH    DE
			PUSH    HL
			DEC     B
			JR      Z,SRCH3         	; Found!
SRCH2:			INC     DE
			LD      A,(DE)
			CP      (HL)
			JR      NZ,SRCH3
			INC     HL
			DJNZ    SRCH2
SRCH3:			POP     HL
			POP     DE
			POP     BC
			JR      NZ,SRCH1
			XOR     A               	; Flags: Z, NC
			RET                     	; Found
;
SRCH4:			OR      0FFH            	; Flags: NZ, NC
			RET                     	; Not found
;
;CHRS - Return character with given ASCII value.
;Result is string.
;
CHRS:			CALL    ITEMI
			EXX
			LD      A,L
			JR      GET1
;
;GETS - Return key pressed as stringor character at position (X,Y).
;Result is string.
;
GETS:			CALL	NXT		;NEW CODE FOR GET$(X,Y)
			CP	'('
			JP	Z, GETSCHR	;CALL FUNCTION IN PATCH.Z80
			CALL    OSRDCH
GET1:			SCF
			JR      INKEY1
;
; INKEYS - Wait up to n centiseconds for keypress.
;          Return key pressed as string or null
;          string if time elapsed.
; Result is string.
;
INKEYS:			CALL    ITEMI			; Fetch the argument
			EXX
INKEY0:			CALL    OSKEY			; This is the entry point for INKEY(n)
INKEY1:			LD      DE,ACCS			; Store the result in the string accumulator
			LD      (DE),A
			LD      A,80H
			RET     NC
			INC     E
			RET
;
; INKEYM - Check immediately whether a given key is being pressed
; Result is integer numeric
;
INKEYM:			MOSCALL	mos_getkbmap		; Get the base address of the keyboard
			INC	HL			; Index from 0
			LD	A, L			; Negate the LSB of the answer
			NEG
			LD	C, A			;  E: The positive keycode value
			LD	A, 1			; Throw an "Out of range" error
			JP	M, ERROR_		; if the argument < - 128
;
			LD	HL, BITLOOKUP		; HL: The bit lookup table
			LD	DE, 0
			LD	A, C
			AND	00000111b		; Just need the first three bits
			LD	E, A			; DE: The bit number
			ADD	HL, DE
			LD	B, (HL)			;  B: The mask
;
			LD	A, C			; Fetch the keycode again
			AND	01111000b		; And divide by 8
			RRCA
			RRCA
			RRCA
			LD	E, A			; DE: The offset (the MSW has already been cleared previously)
			ADD	IX, DE			; IX: The address
			LD	A, B			;  B: The mask
			AND	(IX+0)			; Check whether the bit is set
			JP	Z, ZERO			; No, so return 0
			JP	TRUE			; Otherwise return -1
;
; A bit lookup table
;
BITLOOKUP:		DB	01h, 02h, 04h, 08h
			DB	10h, 20h, 40h, 80h
;
; MID$ - Return sub-string.
; Result is string.
;
MIDS:			CALL    EXPRSC			; Get the first string expression
			CALL    PUSHS           	; Push the string onto the stack from the string accumulator (ACCS)
			CALL    EXPRI			; Get the second expression
			POP     BC			; C: String length, B: Value of A before PUSHS was called
			CALL    POPS			; Pop the string back off the stack to the string accumulator
			EXX
			LD      A,L			; A: The start index
			EXX
			OR      A			; If the start index is 0, then we don't need to do the next bit
			JR      Z,MIDS1
			DEC     A			
			LD      L,A			; L: The start index - 1
			SUB     E			; Subtract from the string length
			LD      E,0			; Preemptively set the string length to 0
			JR      NC,MIDS1		; If the first parameter is greater than the string length, then do nothing
			NEG				; Negate the answer and
			LD      C,A			; C: Number of bytes to copy
			CALL    RIGHT1			; We can do a RIGHT$ at this point with the result
MIDS1:			CALL    NXT			; Skip whitespace
			CP      ','			; Check for a comma
			INC     IY			; Advance to the next character in the BASIC line
			JR      Z,LEFT1			; If there is a comma then we do a LEFT$ on the remainder
			DEC     IY			; Restore the BASIC program pointer
			CALL    BRAKET			; Check for a bracket
			LD      A,80H			; String marker
			RET
;
; LEFT$ - Return left part of string.
; Carry cleared if entire string returned.
; Result is string.
;
LEFTS:			CALL    EXPRSC			; Get the first string expression
LEFT1:			CALL    PUSHS           	; Push the string onto the stack from the string accumulator (ACCS)
			CALL    EXPRI			; Get the second expression
			POP     BC			; C: String length, B: Value of A before PUSHS was called
			CALL    POPS			; Pop the string back off the stack to the string accumulator (ACCS)
			CALL    BRAKET			; Check for closing bracket
			EXX
			LD      A,L			; L: The second parameter
			EXX
			CP      E			; Compare with the string length
			JR      NC,LEFT3		; If it is greater than or equal then do nothing
			LD      L,E             	; For RIGHTS, no effect in LEFTS
LEFT2:			LD      E,A			; E: The new length of string
LEFT3:			LD      A,80H           	; String marker
			RET
;
; RIGHT$ - Return right part of string.
; Result is string.
;
RIGHTS:			CALL    LEFTS			; Call LEFTS to get the string
			RET     NC			; Do nothing if the second parameter is >= string length
			INC     E			; Check for a zero length string
			DEC     E
			RET     Z			; Yes, so do nothing
			LD      C,E			;  C: Number of bytes to copy
			LD      A,L
			SUB     E
			LD      L,A			;  L: Index into the string
RIGHT1:			LD	A,C
			LD	BC,0
			LD	C,A			; BC: Number of bytes to copy (with top word cleared)
			LD	A,L
			LD	HL,ACCS
			LD	L,A			; HL: Source (in ACCS)
			LD      DE,ACCS			; DE: Destination (start of ACCS)
			LDIR                    	; Copy
			LD      A,80H			; String marker
			RET
;
; STRINGS - Return n concatenations of a string.
; Result is string.
;
STRING_:		CALL    EXPRI			; Get number of times to replicate
			CALL    COMMA			; Check for comma
			EXX
			LD      A,L			; L: Number of iterations of string
			EXX
			PUSH    AF
			CALL    EXPRS			; Get the string
			CALL    BRAKET			; Check for closing bracket
			POP     AF			; A: Number of iterations of string
			OR      A			; Set flags
			JR      Z,LEFT2         	; If iterations is 0, then this will return an empty string
			DEC     A
			LD      C,A			; C: Loop counter
			LD      A,80H			; String marker
			RET     Z
			INC     E			; Check for empty string
			DEC     E
			RET     Z              		; And return
			LD      B,E			; B: String length tally
			LD	HL,ACCS
STRIN1:			PUSH    BC
STRIN2:			LD      A,(HL)
			INC     HL
			LD      (DE),A
			INC     E
			LD      A,19
			JP      Z,ERROR_         	; Throw a "String too long" error
			DJNZ    STRIN2
			POP     BC
			DEC     C
			JR      NZ,STRIN1
			LD      A,80H
			RET
;
;SUBROUTINES
;
;SWAP - Swap arguments
;Exchanges DE,HL D'E',H'L' and B,C
;Destroys: A,B,C,D,E,H,L,D',E',H',L'
;
SWAP:			LD      A,C
			LD      C,B
			LD      B,A
			EX      DE,HL
			EXX
			EX      DE,HL
			EXX
			RET
;
;TEST - Test HLH'L' for zero
;Outputs: Z-flag set & A=0 if zero
;Destroys: A,F
;
TEST:			LD      A,H
			OR      L
			EXX
			OR      H
			OR      L
			EXX
			RET
;
;DECODE - Decode line number in pseudo-binary.
;   Inputs: IY = Text pointer.
;   Outputs: HL=0, H'L'=line number, C=0.
;   Destroys: A,C,H,L,H',L',IY,F
;
DECODE:			EXX
			LD	HL, 0
			LD      A,(IY)
			INC     IY
			RLA
			RLA
			LD      H,A
			AND     0C0H
			XOR     (IY)
			INC     IY
			LD      L,A
			LD      A,H
			RLA
			RLA
			AND     0C0H
			XOR     (IY)
			INC     IY
			LD      H,A
			EXX
;			XOR     A
;			LD      C,A
;			LD      H,A
;			LD      L,A
			LD	HL, 0
			LD	C, L
			RET
;
;HEXSTR - convert numeric value to HEX string.
;   Inputs: HLH'L'C = integer or floating-point number
;  Outputs: String in string accumulator.
;           E = string length.  D = ACCS/256
;
HEXSTS:			INC     IY              ;SKIP TILDE
			CALL    ITEMN
			CALL    HEXSTR
			LD      A,80H
			RET
;
HEXSTR:			CALL    SFIX
			LD      BC,8
			LD      DE,ACCS
HEXST1:			PUSH    BC
			LD      B,4
			XOR     A
HEXST2:			EXX
			ADD.S	HL,HL
			EXX
			ADC.S	HL,HL
			RLA
			DJNZ    HEXST2
			POP     BC
			DEC     C
			RET     M
			JR      Z,HEXST3
			OR      A
			JR      NZ,HEXST3
			CP      B
			JR      Z,HEXST1
HEXST3:			ADD     A,90H
			DAA
			ADC     A,40H
			DAA
			LD      (DE),A
			INC     DE
			LD      B,A
			JR      HEXST1
;
;Function STR - convert numeric value to ASCII string.
;   Inputs: HLH'L'C = integer or floating-point number.
;  Outputs: String in string accumulator.
;           E = length, D = ACCS/256
;           A = 80H (type=string)
;
;First normalise for decimal output:
;
STRS:			CALL    NXT
			CP      '~'
			JR      Z,HEXSTS
			CALL    ITEMN
			LD      IX,STAVAR
			LD      A,(IX+3)
			OR      A
			LD      IX,G9-1         ;G9 FORMAT
			JR      Z,STR0
STR:			LD      IX,STAVAR
STR0:			LD      DE,ACCS
			LD      A,37
			CALL    FPP
			JP      C,ERROR_
			BIT     0,(IX+2)
STR1:			LD      A,80H           ;STRING MARKER
			RET     Z
			LD      A,C
			ADD     A,4
STR2:			CP      E
			JR      Z,STR1
			EX      DE,HL	
			LD      (HL),' '        ;TRAILING SPACE
			INC     HL
			EX      DE,HL
			JR      STR2
;
G9:			DW    9
;
;STRING COMPARE
;Compare string (DE) length B with string (HL) length C.
;Result preset to false.
;
SCP:			CALL	SCP0
;
ZERO:			LD      A,0
			EXX
			LD      H,A
			LD      L,A
			EXX
			LD      H,A
			LD      L,A
			LD      C,A
			RET
;
SCP0:			INC     B
			INC     C
SCP1:			DEC     B
			JR      Z,SCP2
			DEC     C
			JR      Z,SCP3
			LD      A,(DE)
			CP      (HL)
			RET     NZ
			INC     DE
			INC     HL
			JR      SCP1
SCP2:			OR      A
			DEC     C
			RET     Z
			SCF
			RET
SCP3:			OR      A
			INC     C
			RET
;
; PUSHS - SAVE STRING ON STACK.
;     Inputs: String in string accumulator.
;             E = string length.
;             A - saved on stack.
;   Destroys: B,C,D,E,H,L,IX,SP,F
;
PUSHS:			CALL    CHECK			; Check if there is sufficient space on the stack
			POP     IX              	; IX: Return address
			OR      A               	; Clear the carry flag
			LD	BC,0			; BC: Length of the string
			LD	C,E
			LD      HL,ACCS			; HL: Pointer to the string accumulator
			LD	DE,ACCS
			LD	E,C 			; DE: Pointer to the end of the string in the accumulator
			SBC     HL,DE			; HL: Number of bytes to reserve on the stack (a negative number)
			ADD     HL,SP			; Grow the stack
			LD      SP,HL
			LD      D,A			;  D: This needs to be set to A for some functions
; BEGIN MISSING FROM BINARY
			; LD	B,A			; Stack A and C (the string length)
			; PUSH    BC			; Note that this stacks 3 bytes, not 2; the MSB is irrelevant
			; LD	B,0			; Reset B to 0 for the LDIR in this function
; END MISSING FROM BINARY
			PUSH 	DE ; ADDED FROM BINARY
			JR      Z,PUSHS1        	; Is it zero length?
			LD      DE,ACCS			; DE: Destination
			EX      DE,HL			; HL: Destination, DE: Address on stack
			LDIR	                    	; Copy to stack
			CALL    CHECK			; Final check to see if there is sufficient space on the stack
PUSHS1:			JP      (IX)            	; Effectively "RET" (IX contains the return address)
;
; POPS - RESTORE STRING FROM STACK.
;     Inputs: C = string length.
;    Outputs: String in string accumulator.
;             E = string length.
;   Destroys: B,C,D,E,H,L,IX,SP,F
;
POPS:			POP     IX              	; IX: Return address
			LD	L,C			; Temporarily store string length in L
			LD	BC,0
			LD	C,L			; BC: Number of bytes to copy
			LD      HL,0			; HL: 0
			ADD     HL,SP			; HL: Stack address
			LD      DE,ACCS			; DE: Destination
			INC     C			; Quick check to see if this is a zero length string
			DEC     C
			JR      Z,POPS1         	; Yes it is, so skip
			LDIR                    	; No, so copy from the stack
POPS1:			LD      SP,HL			; Shrink the stack
			JP      (IX)            	; Effectively "RET" (IX contains the return address)
;
HEXDIG:			LD      A,(IY)
			CP      '0'
			RET     C
			CP      '9'+1
			CCF
			RET     NC
			CP      'A'
			RET     C
			SUB     'A'-10
			CP      16
			CCF
			RET
;
BINDIG:			LD	A,(IY)
			CP	'0'
			RET	C
			CP	'1'+1
			CCF
			RET
;
RELOP?:			CP      '>'
			RET     NC
			CP      '='
			RET     NC
			CP      '<'
			RET
;
EXPRSC:			CALL    EXPRS
COMMA:			CALL    NXT
			INC     IY
			CP      ','
			RET     Z
			LD      A,5
			JR      ERROR1_EV          ;"Missing ,"
;
BRAKET:			CALL    NXT
			INC     IY
			CP      ')'
			RET     Z
			LD      A,27
ERROR1_EV:			JP      ERROR_           ;"Missing )"
;
SAVE_EV:			INC     IY
SAVE1:			EX      AF,AF'
			JP      M,TYPE_EV_
			EX      AF,AF'
			EX      (SP),HL
			EXX
			PUSH    HL
			EXX
			PUSH    AF
			PUSH    BC
			JP      (HL)
;
DOIT:			EX      AF,AF'
			JP      M,TYPE_EV_
			EXX
			POP     BC              ;RETURN ADDRESS
			EXX
			LD      A,C
			POP     BC
			LD      B,A
			POP     AF              ;OPERATOR
			EXX
			EX      DE,HL
			POP     HL
			EXX
			EX      DE,HL
			POP     HL
			EXX
			PUSH    BC
			EXX
			AND     0FH
			CALL    FPP
			JR      C,ERROR1_EV
			XOR     A
			EX      AF,AF'          ;TYPE
			LD      A,(IY)
			RET
;
; Skip spaces
; - IY: String pointer
; Returns:
;  - A: The non-space character found
; - IY: Points to the character before that
; 
NXT:			LD      A,(IY)			; Fetch the character	
			CP      ' '			; If it is space, then return
			RET     NZ
			INC     IY			; Increment the pointer and
			JP      NXT			; Loop
;
DISPT2:			PUSH    HL
			LD      HL,SOPTBL
			JR      DISPT0
;
DISPAT:			PUSH    HL
			SUB     FUNTOK
			LD      HL,FUNTBL
DISPT0:			PUSH    BC
			
			LD	BC, 3
			LD	B, A
			MLT	BC
			ADD	HL, BC
			LD	HL, (HL)

;			ADD     A,A
;			LD      C,A
;			LD      B,0
;			ADD     HL,BC
;			LD      A,(HL)
;			INC     HL
;			LD      H,(HL)
;			LD      L,A

			POP     BC
			EX      (SP),HL
			RET                     ;OFF TO ROUTINE

; --- End eval.asm ---

; --- Begin exec.asm ---
;
; Title:	BBC Basic Interpreter - Z80 version
;		Statement Execution & Assembler Module - "EXEC"
; Author:	(C) Copyright  R.T.Russell  1984
; Modified By:	Dean Belfield
; Created:	12/05/2023
; Last Updated:	26/06/2023
;
; Modinfo:
; 27/01/1984:	Version 2.1
; 02/03/1987:	Version 3.0
; 11/06/1987:	Version 3.1
; 12/05/2023:	Modified by Dean Belfield
; 07/06/2023:	Modified to run in ADL mode
; 26/06/2023:	Fixed DIM, USR, and address output of inline assembler

			; .ASSUME	ADL = 1

			; INCLUDE	"equs.inc"

			; SEGMENT CODE
				
			; XDEF	XEQ
			; XDEF	CHAIN0
			; XDEF	RUN
			; XDEF	SYNTAX
			; XDEF	ESCAPE
			; XDEF	FN_EX
			; XDEF	USR
			; XDEF	STORE5
			; XDEF	STORE4
			; XDEF	CHECK
			; XDEF	TERMQ
			; XDEF	FILL
			; XDEF	X4OR5
			; XDEF	MUL16
			; XDEF	CHANEL
			; XDEF	ASSEM
				
			; XREF	AUTO
			; XREF	DELETE
			; XREF	LOAD
			; XREF	LIST_
			; XREF	NEW
			; XREF	OLD
			; XREF	RENUM
			; XREF	SAVE
			; XREF	SOUND
			; XREF	CLG
			; XREF	DRAW
			; XREF	ENVEL
			; XREF	GCOL
			; XREF	MODE
			; XREF	MOVE
			; XREF	PLOT
			; XREF	COLOUR
			; XREF	EXPRS
			; XREF	HIMEM
			; XREF	LOAD0
			; XREF	RANDOM
			; XREF	CLEAR
			; XREF	ERRTRP
			; XREF	PAGE_
			; XREF	DATAPTR
			; XREF	ERRLIN
			; XREF	TRAP
			; XREF	NXT
			; XREF	SETLIN
			; XREF	CLOOP
			; XREF	OSSHUT
			; XREF	WARM
			; XREF	TRACEN
			; XREF	OUTCHR
			; XREF	PBCDL
			; XREF	OSCLI
			; XREF	LISTON
			; XREF	GETVAR
			; XREF	PUTVAR
			; XREF	DATPTR
			; XREF	ERROR_
			; XREF	EXPR
			; XREF	CREATE
			; XREF	EXPRI
			; XREF	BRAKET
			; XREF	FREE
			; XREF	OSBPUT
			; XREF	COUNT
			; XREF	STR
			; XREF	HEXSTR
			; XREF	CRLF
			; XREF	ITEMI
			; XREF	FINDL
			; XREF	TEST
			; XREF	EXPRN
			; XREF	DLOAD5
			; XREF	DLOAD5_SPL
			; XREF	LOADN
			; XREF	FPP
			; XREF	SWAP
			; XREF	GETDEF
			; XREF	ZERO
			; XREF	OSBGET
			; XREF	BUFFER
			; XREF	CONS
			; XREF	VAL0
			; XREF	OSLINE
			; XREF	CLRSCN
			; XREF	TELL
			; XREF	SAYLN
			; XREF	REPORT
			; XREF	PUTPTR
			; XREF	PUTIME
			; XREF	PUTIMS
			; XREF	LOMEM
			; XREF	WIDTH
			; XREF	OSWRCH
			; XREF	COMMA
			; XREF	OSCALL
			; XREF	SFIX
			; XREF	LOAD4
			; XREF	PUSHS
			; XREF	POPS
			; XREF	LOADS
			; XREF	PUTCSR
			; XREF	OUT_
			; XREF	R0
;
; List of token values used in this module
;
TAND:			EQU     80H
TOR:			EQU     84H
TERROR_EX:			EQU     85H
LINE_EX_:			EQU     86H
OFF_:			EQU     87H
STEP:			EQU     88H
SPC:			EQU     89H
TAB:			EQU     8AH
ELSE_EX_:			EQU     8BH
THEN_EX_:			EQU     8CH
LINO_EX:			EQU     8DH
TO_EX:			EQU     B8H
TCMD_EX:			EQU     C6H
TCALL:			EQU     D6H
DATA_EX_:			EQU     DCH
DEF_:			EQU     DDH
TGOSUB:			EQU     E4H
TGOTO:			EQU     E5H
TON:			EQU     EEH
TPROC:			EQU     F2H
TSTOP:			EQU     FAH

; The command table
; Commands are tokens from C6H onwards; this lookup table is used to
; run the corresponding function; Note that DATA and DEF both use the same
; code as REM
;
CMDTAB:			DW24  AUTO			; C6H
			DW24  DELETE			; C7H
			DW24  LOAD			; C8H
			DW24  LIST_			; C9H
			DW24  NEW			; CAH
			DW24  OLD			; CBH
			DW24  RENUM			; CCH
			DW24  SAVE			; CDH
			DW24  PUT			; CEH
			DW24  PTR_EX			; CFH
			DW24  PAGEV_EX			; D0H
			DW24  TIMEV_EX			; D1H
			DW24  LOMEMV_EX			; D2H
			DW24  HIMEMV_EX			; D3H
			DW24  SOUND			; D4H
			DW24  BPUT			; D5H
			DW24  CALL_			; D6H
			DW24  CHAIN			; D7H
			DW24  CLR			; D8H
			DW24  CLOSE			; D9H
			DW24  CLG			; DAH
			DW24  CLS			; DBH
			DW24  REM_EX             		; DCH: DATA
			DW24  REM_EX             		; DDH: DEF
			DW24  DIM_EX			; DEH
			DW24  DRAW			; DFH
			DW24  END_			; E0H
			DW24  ENDPRO			; E1H
			DW24  ENVEL			; E2H
			DW24  FOR_EX			; E3H
			DW24  GOSUB_EX			; E4H
			DW24  GOTO_EX			; E5H
			DW24  GCOL			; E6H
			DW24  IF_			; E7H
			DW24  INPUT			; E8H
			DW24  LET			; E9H
			DW24  LOCAL_EX_			; EAH
			DW24  MODE			; EBH
			DW24  MOVE			; ECH
			DW24  NEXT_EX			; EDH
			DW24  ON_EX_			; EEH
			DW24  VDU			; EFH
			DW24  PLOT			; F0H
			DW24  PRINT_			; F1H
			DW24  PROC_EX			; F2H
			DW24  READ			; F3H
			DW24  REM_EX			; F4H
			DW24  REPEAT_EX			; F5H
			DW24  REPOR			; F6H
			DW24  RESTOR_EX			; F7H
			DW24  RETURN			; F8H
			DW24  RUN			; F9H
			DW24  STOP			; FAH
			DW24  COLOUR			; FBH
			DW24  TRACE_EX			; FCH
			DW24  UNTIL_EX			; FDH
			DW24  WIDTHV			; FEH
			DW24  CLI             		; FFH: OSCLI

; RUN 
; RUN "filename"
;
RUN:			CALL    TERMQ			; Standalone RUN command?
			JR      Z,RUN0			; Yes, so just RUN the code

; CHAIN "filename"
;
CHAIN:			CALL    EXPRS			; Get the filename
			LD      A,CR			; Terminate it with a CR
			LD      (DE),A
CHAIN0:			LD      SP,(HIMEM)		; Reset SP
			CALL    LOAD0			; And load the file in
;
RUN0:			LD      SP,(HIMEM)      	; Prepare for RUN
			LD      IX,RANDOM		; Pointer to the RANDOM sysvar
@@:			LD      A, R			; Use the R register to seed the random number generator
			JR      Z, @B			; Loop unti we get a non-zero value in A
			RLCA				; Rotate it
			RLCA
			LD      (IX+3),A		; And store
			SBC     A,A			; Depending upon the C flag, this will either be 00h or FFh
			LD      (IX+4),A		; And store
			CALL    CLEAR
			LD      HL,0			; Clear the error trap sysvar
			LD      (ERRTRP),HL
			LD      HL,(PAGE_)		; Load HL with the start of program memory (PAGE)
			LD      A,DATA_EX_			; The DATA token value
			CALL    SEARCH_EX          	; Search for the first DATA token in the tokenised listing
			LD      (DATPTR),HL     	; Set data pointer
			LD      IY,(PAGE_)		; Load IY with the start of program memory
;			
XEQ0:			CALL    NEWLIN
XEQ:			LD      (ERRLIN),IY     	; Error pointer
			CALL    TRAP           		; Check keyboard
XEQ1:			CALL    NXT
			INC     IY
			CP      ':'             	; Seperator
			JR      Z,XEQ1
			CP      CR
			JR      Z,XEQ0          	; New program line
			SUB     TCMD_EX
			JP      C,LET0          	; Implied "LET"
			
			LD	BC, 3
			LD	B, A 
			MLT	BC 
			LD	HL,CMDTAB
			ADD	HL, BC 
			LD	HL, (HL)		; Table entry

;			ADD     A,A
;			LD      C,A
;			LD      B,0
;			LD      HL,CMDTAB
;			ADD     HL,BC
;			LD      A,(HL)          	; Table entry
;			INC     HL
;			LD      H,(HL)
;			LD      L,A

			CALL    NXT
			JP      (HL)            	; Execute the statement

;END
;
END_:			CALL    SETLIN          ;FIND CURRENT LINE
			LD      A,H
			OR      L               ;DIRECT?
			JP      Z,CLOOP
			LD      E,0
			CALL    OSSHUT          ;CLOSE ALL FILES
			JP      WARM            ;"Ready"
;
NEWLIN:			LD      A,(IY+0)        ;A=LINE LENGTH
			LD      BC,3
			ADD     IY,BC
			OR      A
			JR      Z,END_           ;LENGTH=0, EXIT
			LD      HL,(TRACEN)
			LD      A,H
			OR      L
			RET     Z
			LD	DE, 0		;Clear DE
			LD      D,(IY-1)        ;DE = LINE NUMBER
			LD      E,(IY-2)
			SBC     HL,DE
			RET     C
			EX      DE,HL
			LD      A,'['           ;TRACE
			CALL    OUTCHR
			CALL    PBCDL
			LD      A,']'
			CALL    OUTCHR
			LD      A,' '
			JP      OUTCHR

; Routines for each statement -------------------------------------------------

; OSCLI
;
CLI:			CALL    EXPRS
			LD      A,CR
			LD      (DE),A
			LD      HL,ACCS
			CALL    OSCLI
			JP      XEQ

; REM, *
;
EXT_EX:			PUSH    IY
			POP     HL
			CALL    OSCLI
REM_EX:			PUSH    IY
			POP     HL
			LD      A,CR
			LD      B,A
			CPIR                    ;FIND LINE END
			PUSH    HL
			POP     IY
			JP      XEQ0

; [LET] var = expr
;
LET0:			CP      ELSE_EX_-TCMD_EX
			JR      Z,REM_EX
			; CP      ('*'-TCMD) & 0FFH
			; JR      Z,EXT_EX
			; CP      ('='-TCMD) & 0FFH
			; JR      Z,FNEND
			; CP      ('['-TCMD) & 0FFH
			; ez80asm doesn't like () in expressions
			CP      '*'-TCMD_EX & 0FFH
			JR      Z,EXT_EX
			CP      '='-TCMD_EX & 0FFH
			JR      Z,FNEND
			CP      '['-TCMD_EX & 0FFH
			JR      Z,ASM
			DEC     IY
LET:			CALL    ASSIGN			; Assign the variable
			JP      Z,XEQ			; Return if Z as it is a numeric variable that has been assigned in ASSIGN
			JR      C,SYNTAX        	; Return if C as it is an illegal variable
;
			PUSH    AF              	; At this point we're dealing with a string type (A=81h)
			CALL    EQUALS			; Check if the variable is followed by an '=' symbol; this will throw a 'Mistake' error if not
			PUSH    HL			; HL: Address of the variable
			CALL    EXPRS
			POP     IX			; IX: Address of the variable
			POP     AF			; AF: The variable type
			CALL    STACCS			; Copy the string from ACCS to the variable area
XEQR:			JP      XEQ
;
ASM0:			CALL    NEWLIN
ASM:			LD      (ERRLIN),IY
			CALL    TRAP
			CALL    ASSEM
			JR      C,SYNTAX
			CP      CR
			JR      Z,ASM0
			LD      HL,LISTON
			LD      A,(HL)
			AND     0FH
			OR      B0H
			LD      (HL),A
			JR      XEQR
;
VAR_:			CALL    GETVAR
			RET     Z
			JP      NC,PUTVAR
SYNTAX:			LD      A,16            ;"Syntax error"
			JR	ERROR0_EX
ESCAPE:			LD      A,17            ;"Escape"
ERROR0_EX:			JP      ERROR_

; =
;
FNEND:			CALL    EXPR            ;FUNCTION RESULT
			LD      B,E
			EX      DE,HL
			EXX                     ;SAVE RESULT
			EX      DE,HL           ; IN DEB'C'D'E'
FNEND5:			POP     BC
			LD      HL,LOCCHK
			OR      A
			SBC     HL,BC
			JR      Z,FNEND0        ;LOCAL VARIABLE
			LD      HL,FNCHK
			OR      A
			SBC     HL,BC
			LD      A,7
			JR      NZ,ERROR0_EX       ;"No FN"
			POP     IY
			LD      (ERRLIN),IY     ;IN CASE OF ERROR
			EX      DE,HL
			EXX
			EX      DE,HL
			LD      DE,ACCS
			LD      E,B
			EX      AF,AF'
			RET
;
FNEND0:			POP     IX
			POP     BC
			LD      A,B
			OR      A
			JP      M,FNEND1        ;STRING
			POP     HL
			EXX
			POP     HL
			EXX
			CALL    STORE
			JR      FNEND5
FNEND1:			LD      HL,0
			ADD     HL,SP
			PUSH    DE
			LD      E,C
			CALL    STORES
			POP     DE
			LD      SP,HL
			JR      FNEND5

; DIM var(dim1[,dim2[,...]])[,var(...]
; DIM var expr[,var expr...]
;
DIM_EX:			CALL    GETVAR          	; Get the variable
			JP      C,BADDIM		; Throw a "Bad Dim" error
			JP      Z,DIM4			; If Z then the command is DIM var% expr, so don't need to create an entity
			CALL    CREATE			; Create a new entity
			PUSH    HL			; HL: Address of the entity
			POP     IX			; IX: Address of the entity
			LD      A,(IY)			; Fetch the next character from the tokenised string
			CP      '('			; Check for opening brackets
			LD      A,D			;  A: The dimension variable type (04h = Integer, 05h = Float, 81h = String)
			JR      NZ,DIM4			; It is not a bracket; the command is DIM var expr
;
; At this point we're reserving a variable array
;
			PUSH    HL			; HL: Address of the entity
			PUSH    AF           	   	;  A: Entity type (04h = Integer, 05h = Float, 81h = String)
			LD      DE,1			; DE: Total size of array accumulator (important for multi-dimensioned arrays)
			LD      B,D			;  B: The number of dimensions in the array
;
DIM1:			INC     IY			; Skip to the next token
			PUSH    BC			; Stack the dimension counter
			PUSH    DE			; Stack the total size of array accumulator
			PUSH    IX			; Stack the entity address
			CALL    EXPRI           	; Fetch the size of this dimension
			BIT     7,H			; If it is negative then
			JR      NZ,BADDIM		; Throw a "Bad Dim" error
			EXX
			INC     HL			; HL: Size of this dimension; increment (BBC BASIC DIMs are always one bigger)
			POP     IX			; IX: The entity address
			INC     IX				
			LD      (IX),L          	; Save the size of this dimension in the entity
			INC     IX
			LD      (IX),H
			POP     BC
			CALL    MUL16           	; HL = HL * BC (Number of Dimensions * Total size of array accumulator)
			JR      C,NOROOM        	; Throw a "No Room" error if overflow
			EX      DE,HL           	; DE: The new total size of array accumulator
			POP     BC
			INC     B               	;  B: The dimension counter; increment
			LD      A,(IY)			; Fetch the nex token
			CP      ','             	; Check for another dimension in the array
			JR      Z,DIM1			; And loop
;
			CALL    BRAKET          	; Check for closing bracket
			POP     AF              	; Restore the type
			INC     IX
			EX      (SP),IX
			LD      (IX),B          	; Number of dimensions
			CALL    X4OR5           	; Dimension Accumulator Value * 4 or * 5 depending on type
			POP     HL			; Restore the entity address
			JR      C,NOROOM		; Throw a "No Room" error if there is an overflow
;
; We now allocate the memory for the array
;			
DIM3:			ADD     HL,DE
			JR      C,NOROOM
			PUSH    HL
			INC     H
			JR      Z,NOROOM
			SBC     HL,SP
			JR      NC,NOROOM       	; Throw an "Out of Space" error
			POP     HL
			LD      (FREE),HL
DIM2:			LD      A,D
			OR      E
			JR      Z,DIM5
			DEC     HL
			LD      (HL),0         		; Initialise the array
			DEC     DE
			JR      DIM2
DIM5:			CALL    NXT
			CP      ','            		; Another variable?
			JP      NZ,XEQ
			INC     IY
			CALL    NXT
			JP      DIM_EX
;
; DIM errors
;
BADDIM:			LD      A,10            	; Throw a "Bad DIM" error
			JR	ERROR1_EX			
NOROOM:			LD      A,11            	; Throw a "DIM space" error
ERROR1_EX:			JP      ERROR_
;
; At this point we're reserving a block of memory, i.e.
; DIM var expr[,var expr...]
;
DIM4:			OR      A			;  A: The dimension variable type 
			JR      Z,BADDIM		; Throw "Bad Dim" if variable is an 8-bit indirection
			JP      M,BADDIM        	; or a string
			LD      B,A			; Temporarily store the dimension variable type in B
			LD      A,(IY-1)		; Get the last character but one
			CP      ')'			; Check if it is a trailing bracket
			JR      Z,BADDIM		; And throw a "Bad Dim" error if there is a trailing bracket
;
			LD	HL,0			; Clear HL
			LD	A,(FREE+0)		; HL: Lower 16 bits of FREE
			LD	L,A
			LD	A,(FREE+1)
			LD	H,A
			LD	A,B			; Restore the dimension variable type
			EXX
			LD	HL,0			; Clear HL
			LD	B,A			; Temporarily store the dimension variable type in B
			LD	A,(FREE+2)		; HL: Upper 8 bits of FREE (bits 16-23)
			LD	L,A
			LD	A,B			; Restore the dimension variable type
			LD	C,H
			CALL    STORE           	; Store the address
			CALL    EXPRI			; Get the number of bytes to store
			EXX
			INC     HL			; Add one to it
			EX      DE,HL
			LD      HL,(FREE)
			JR      DIM3			; Continue with the DIM

; PRINT list...
; PRINT #channel,list...
;
PRINT_:			CP      '#'
			JR      NZ,PRINT0
			CALL    CHNL            ;CHANNEL NO. = E
PRNTN1:			CALL    NXT
			CP      ','
			JP      NZ,XEQ
			INC     IY
			PUSH    DE
			CALL    EXPR            ;ITEM TO PRINT
			EX      AF,AF'
			JP      M,PRNTN2        ;STRING
			POP     DE
			PUSH    BC
			EXX
			LD      A,L
			EXX
			CALL    OSBPUT
			EXX
			LD      A,H
			EXX
			CALL    OSBPUT
			LD      A,L
			CALL    OSBPUT
			LD      A,H
			CALL    OSBPUT
			POP     BC
			LD      A,C
			CALL    OSBPUT
			JR      PRNTN1
PRNTN2:			LD      C,E
			POP     DE
			LD      HL,ACCS
			INC     C
PRNTN3:			DEC     C
			JR      Z,PRNTN4
			LD      A,(HL)
			INC     HL
			PUSH    BC
			CALL    OSBPUT
			POP     BC
			JR      PRNTN3
PRNTN4:			LD      A,CR
			CALL    OSBPUT
			JR      PRNTN1
;
PRINT6:			LD      B,2
			JR      PRINTC
PRINT8:			LD      BC,100H
			JR      PRINTC
PRINT9:			LD      HL,STAVAR
			XOR     A
			CP      (HL)
			JR      Z,PRINT0
			LD      A,(COUNT)
			OR      A
			JR      Z,PRINT0
PRINTA:			SUB     (HL)
			JR      Z,PRINT0
			JR      NC,PRINTA
			NEG
			CALL    FILL
PRINT0:			LD      A,(STAVAR)
			LD      C,A             ;PRINTS
			LD      B,0             ;PRINTF
PRINTC:			CALL    TERMQ
			JR      Z,PRINT4
			RES     0,B
			INC     IY
			CP      '~'
			JR      Z,PRINT6
			CP      ';'
			JR      Z,PRINT8
			CP      ','
			JR      Z,PRINT9
			CALL    FORMAT          ;SPC, TAB, '
			JR      Z,PRINTC
			DEC     IY
			PUSH    BC
			CALL    EXPR            ;VARIABLE TYPE
			EX      AF,AF'
			JP      M,PRINT3        ;STRING
			POP     DE
			PUSH    DE
			BIT     1,D
			PUSH    AF
			CALL    Z,STR           ;DECIMAL
			POP     AF
			CALL    NZ,HEXSTR       ;HEX
			POP     BC
			PUSH    BC
			LD      A,C
			SUB     E
			CALL    NC,FILL         ;RIGHT JUSTIFY
PRINT3:			POP     BC
			CALL    PTEXT           ;PRINT
			JR      PRINTC
PRINT4:			BIT     0,B
			CALL    Z,CRLF
			JP      XEQ

; ON ERROR statement [:statement...]
; ON ERROR OFF
;
ONERR:			INC     IY              ;SKIP "ERROR"
			LD      HL,0
			LD      (ERRTRP),HL
			CALL    NXT
			CP      OFF_
			INC     IY
			JP      Z,XEQ
			DEC     IY
			LD      (ERRTRP),IY
			JP      REM_EX

; ON expr GOTO line[,line...] [ELSE statement]
; ON expr GOTO line[,line...] [ELSE line]
; ON expr GOSUB line[,line...] [ELSE statement]
; ON expr GOSUB line[,line...] [ELSE line]
; ON expr PROCone [,PROCtwo..] [ELSE PROCotherwise]
;
ON_EX_:			CP      TERROR_EX
			JR      Z,ONERR         ;"ON ERROR"
			CALL    EXPRI
			LD      A,(IY)
			INC     IY
			LD      E,','           ;SEPARATOR
			CP      TGOTO
			JR      Z,ON1
			CP      TGOSUB
			JR      Z,ON1
			LD      E,TPROC
			CP      E
			LD      A,39
			JR      NZ,ERROR2_EX       ;"ON syntax"
ON1:			LD      D,A
			EXX
			PUSH    HL
			EXX
			POP     BC              ;ON INDEX
			LD      A,B
			OR      H
			OR      L
			JR      NZ,ON4          ;OUT OF RANGE
			OR      C
			JR      Z,ON4
			DEC     C
			JR      Z,ON3           ;INDEX=1
ON2:			CALL    TERMQ
			JR      Z,ON4           ;OUT OF RANGE
			INC     IY              ;SKIP DELIMITER
			CP      E
			JR      NZ,ON2
			DEC     C
			JR      NZ,ON2
ON3:			LD      A,E
			CP      TPROC
			JR      Z,ONPROC
			PUSH    DE
			CALL    ITEMI           ;LINE NUMBER
			POP     DE
			LD      A,D
			CP      TGOTO
			JR      Z,GOTO2
			CALL    SPAN            ;SKIP REST OF LIST
			JR      GOSUB1
;
ON4:			LD      A,(IY)
			INC     IY
			CP      ELSE_EX_
			JP      Z,IF1           ;ELSE CLAUSE
			CP      CR
			JR      NZ,ON4
			LD      A,40
ERROR2_EX:			JP      ERROR_           ;"ON range"
;
ONPROC:			LD      A,TON
			JP      PROC_EX

; GOTO line
;
GOTO_EX:			CALL    ITEMI           	; Fetch the line number
GOTO1:			CALL    TERMQ			; Check for terminator
			JP      NZ,SYNTAX		; Throw a "Syntax Error" if not found
GOTO2:			EXX
			CALL    FINDL			; HL: Line number - Find the line
			PUSH    HL			; HL: Address of the line
			POP     IY			; IY = HL
			JP      Z,XEQ0			; If the line is found, then continue execution at that point
			LD      A,41			; Otherwise throw a "No such line" error
			JR      ERROR2_EX

; GOSUB line
; This pushes the following data onto the execution stack
; - 3 bytes: Current execution address
; - 3 bytes: Marker (the address of label GOSCHK)
;
GOSUB_EX:			CALL    ITEMI			; Fetch the line number
GOSUB1:			PUSH    IY              	; Push the current execution address onto the execution stack
			CALL    CHECK           	; Check there is enough room
			CALL    GOTO1           	; Push the marker (address of GOSCHK) onto the execution stack and GOTO the line number
GOSCHK:			EQU     $

; RETURN
; This pops the following data off the execution stack as pushed by GOSUB
; - 3 bytes: Marker (should be the address of label GOSCHK)
; - 3 bytes: The return execution address
;
RETURN:			POP     DE			; Pop the marker off the execution stack
			LD      HL,GOSCHK		; Compare with GOSCHK
			OR      A
			SBC     HL,DE
			POP     IY			; Pop the return address off the execution stack
			JP      Z,XEQ			; Provided this has been called by a GOSUB then continue execution at the return address
			LD      A,38			; Otherwise throw a "No GOSUB" error
			JR      ERROR2_EX

; REPEAT
; This pushes the following data onto the execution stack
; - 3 bytes: Current execution address
; - 3 bytes: Marker (the address of label REPCHK)
;
REPEAT_EX:			PUSH    IY			; Push the current execution address onto the execution stack
			CALL    CHECK			; Check if there is enough room
			CALL    XEQ			; Push the marker (address of REPCHK) onto the execution stack and continue execution
REPCHK:			EQU     $

; UNTIL expr
; This pops the following data off the execution stack
; - 3 bytes: Marker (should be the address of label REPCHK)
; - 3 bytes: The address of the REPEAT instruction
; It also ensures that the data is pushed back on for subsequent UNTIL instructions
;
UNTIL_EX:			POP     BC			; Fetch the marker
			PUSH    BC			; And push it back onto the execution stack
			LD      HL,REPCHK		; Compare with REPCHK
			OR      A
			SBC     HL,BC
			LD      A,43
			JR      NZ,ERROR2_EX		; Throw a "No REPEAT" if this value does not match
			CALL    EXPRI			; Fetch the expression
			CALL    TEST			; Test if the expression evaluates to zero		
			POP     BC			; Pop the marker
			POP     DE			; Pop the address of the REPEAT instruction
			JR      NZ,XEQ2         	; If it is TRUE, then continue execution after the UNTIL instruction (we're done looping)
			PUSH    DE			; Push the address of the REPEAT instruction back on the stack
			PUSH    BC			; Push the marker back on the stack
			PUSH    DE			; IY = DE
			POP     IY			; This sets the execution address back to the REPEAT instruction
XEQ2:			JP      XEQ			; Continue execution

; FOR var = expr TO expr [STEP expr]
; This pushes the following data onto the execution stack
; - 3 bytes: The limit value
; - 3 bytes: The step value
; - 3 bytes: The current execution address
; - 3 bytes: The address of the loop variable
; - 3 bytes: Marker (the address of FORCHK)
;
FORVAR:			LD      A,34
			JR      ERROR2_EX          	; Throw "FOR variable" error
;
FOR_EX:			CALL    ASSIGN			; Assign the START expression value to a variable
			JR      NZ,FORVAR       	; If the variable is a string, or invalid, then throw a "FOR variable" error
			PUSH    AF              	; Save the variable type
			LD      A,(IY)			; Check the next token
			CP      TO_EX			; Compare with the token value for "TO"
			LD      A,36			; Set the error code to 36 ("No TO")
			JP      NZ,ERROR2_EX       	; And throw the error if that token is missing
			INC     IY			; Skip to the next token
;
			PUSH    IX
			CALL    EXPRN           	; Fetch the LIMIT expression value
			POP     IX
			POP     AF
			LD      B,A             	; B: LIMIT value type (04h = Integer, 05h = Float)
			PUSH    BC              	; Stack the LIMIT value
			PUSH    HL
			LD      HL,0
			LD      C,H
			EXX
			PUSH    HL
;			
			LD      HL,1            	; The preset STEP value is 1
			EXX
			LD      A,(IY)			; Fetch the next token
			CP      STEP			; Compare with the token value for "STEP"
			JR      NZ,FOR1			; If there is no STEP token, then skip the next bit
;
			INC     IY			; Skip past the STEP token
			PUSH    IX
			CALL    EXPRN          		; Fetch the STEP expression value
			POP     IX
;			
FOR1:			PUSH    BC			; Stack the STEP value
			PUSH    HL
			EXX
			PUSH    HL
			EXX
;			
			PUSH    IY              	; Stack the current execution address
			PUSH    IX              	; Stack the loop variable
			CALL    CHECK
			CALL    XEQ
FORCHK:			EQU     $

; NEXT [var[,var...]]
; This pops the following data off the execution stack
; - 3 bytes: Marker (the address of FORCHK)
; - 3 bytes: The address of the loop variable
; - 3 bytes: The current execution address
; - 3 bytes: The step value
; - 3 bytes: The limit value
; It also ensures that the data is pushed back on for subsequent NEXT instructions
;
NEXT_EX:			POP     BC              	; Pop the marker off the execution stack
			LD      HL,FORCHK		; Compare with FORCHK
			OR      A
			SBC     HL,BC
			LD      A,32
			JP      NZ,ERROR3_EX      		; If this does not match, throw a "No FOR" error
			CALL    TERMQ			; Check for terminator (a NEXT without a variable)
			POP     HL			; Pop the address of the loop variable off the execution stack
			PUSH    HL			; Push it back onto the execution stack
			PUSH    BC			; Push the marker back onto the execution stack
			PUSH    HL			; HL: Address of the loop variable off the stack
			CALL    NZ,GETVAR       	; If there is no terminator, get the variable from the args
			POP     DE			; DE: Address of the loop variable off the stack
			EX      DE,HL			; HL: Address of the loop variable off the stack, DE: Address of the variable from args
			OR      A
NEXT0:			SBC     HL,DE			; Compare to make sure that the variables match
			JR      NZ,NEXT1		; They don't, so jump to NEXT1
			PUSH    DE
			LD      IX,9+3			; IX: Address of the STEP value on the execution stack
			ADD     IX,SP
			CALL    DLOAD5_SPL      	; Load the STEP value
			LD      A,(IX+16)       	; Get the STEP type
			POP     IX
			CALL    LOADN           	; Load the LOOP variable
			BIT     7,D             	; Check the sign
			PUSH    AF
			LD      A,'+' & 0FH
			CALL    FPP             	; Add the STEP
			JR      C,ERROR3_EX
			POP     AF              	; Restore TYPE
			PUSH    AF
			CALL    STORE           	; Update the variable
			LD      IX,18+3			; IX: Address of the LIMIT value on the execution stack
			ADD     IX,SP
			CALL    DLOAD5_SPL      	; Load the LIMIT value
			POP     AF
			CALL    Z,SWAP			; Swap the arguments if the sign is ?
			; LD      A,0+('<'-4) & 0FH
			LD      A,0+'<'-4 & 0FH ; ez80asm doesn't do () in expressions
			CALL    FPP             	; Test against the limit
			JR      C,ERROR3_EX		; Throw an error if FPP returns bad
			INC     H
			JR      NZ,LOOP_        	; Keep looping
			LD      HL,27			; Adjust the stack
			ADD     HL,SP
			LD      SP,HL
			CALL    NXT
			CP      ','			; Check for multiple variables
			JP      NZ,XEQ			; No, so we are done at ths point
			INC     IY			; Increment to the next variable
			JR      NEXT_EX			; And continue
;
LOOP_:			POP     BC
			POP     DE
			POP     IY
			PUSH    IY
			PUSH    DE
			PUSH    BC
			JP      XEQ
;
NEXT1:			LD      HL,27			; TODO: What does this do?	
			ADD     HL,SP
			LD      SP,HL			; Adjust the stack
			POP     BC
			LD      HL,FORCHK
			SBC     HL,BC
			POP     HL              	; Variable pointer
			PUSH    HL
			PUSH    BC
			JR      Z,NEXT0
;			
			LD      A,33
ERROR3_EX:			JP      ERROR_           	; Throw the error "Can't match FOR"

; FNname
; N.B. ENTERED WITH A <> TON
;
FN_EX:			PUSH    AF              	; Push A onto the stack; this'll be checked for the token ON (TON) in PROC5
			CALL    PROC1
FNCHK:			EQU     $			; This will never fall through as PROC1 will do a JP XEQ

; PROCname
; N.B. ENTERED WITH A = ON PROC FLAG (EEh or the first character of the token name)
; This pushes the following data onto the execution stack
; - 3 bytes: The return address for ENDPROC (initially the ON PROC FLAG)
; - 3 bytes: Marker (the address of PROCHK)
;
PROC_EX:			PUSH    AF			; Push A onto the stack; this'll be checked for the token ON (TON) in PROC5,
			CALL    PROC1			; and is also space reserved on the stack for the return address
PROCHK:			EQU     $			; This will never fall through as PROC1 will do a JP XEQ
;
PROC1:			CALL    CHECK			; Check there is space for this
			DEC     IY			; Decrement IY to the PROC token
			PUSH    IY			; Stack the pointer
			CALL    GETDEF			; Search for this PROC/FN entry in the dynamic area
			POP     BC			; BC = IY
			JR      Z,PROC4			; If found in the dynamic area then skip to PROC4
			LD      A,30
			JR      C,ERROR3_EX        	; Throw error "Bad call" if invalid PROC/FN call
;
; At this point the PROC/FN has not yet been registered in the dynamic area
; So we need to search through the listing and find where the DEFPROC/FN is and save the address
;			
			PUSH    BC			; BC: Still pointing to the PROC token in the tokenised line
			LD      HL,(PAGE_)		; HL: Start of program memory
;
PROC2:			LD      A,DEF_			;  A: The token to search for
			CALL    SEARCH_EX          	; Look for "DEF" as the first token in a program line
			JR      C,PROC3			; Not found, so jump to PROC3
			PUSH    HL			; HL: Points to the DEF token in the DEFPROC
			POP     IY			; IY = HL
			INC     IY              	; Skip the DEF token
			CALL    NXT			; And any whitespace
			CALL    GETDEF			; Search for this PROC/FN entry in the dynamic area
			PUSH    IY
			POP     DE			; DE: Points to the PROC/FN token in tokenised line of the DEFPROC
			JR      C,PROC6			; Skip if there is an error (neither FN or PROC first)
			CALL    NZ,CREATE		; Create an entity in the dynamic area
			PUSH    IY			; IY: Pointer to the DEFPROC/FN arguments
			POP     DE			; DE = IY
			LD	(HL),DE			; Save address
;
PROC6:			EX      DE,HL			; HL: Address of the procedure
			LD      A,CR			; The character to search for
			LD	BC,100h			; Only need to search 256 bytes or so ahead; maximum line length
			CPIR                    	; Skip to next line
			JR      PROC2			; Rinse, lather and repeat
;
; At this point a DEF has not been found for the PROC/FN
;
PROC3:			POP     IY              	; Restore the execution address
			CALL    GETDEF			; Search for this PROC/FN entry in the dynamic area
			LD      A,29
			JR      NZ,ERROR3_EX      		; Throw error "No such FN/PROC" if not found
;
; At this point we have a PROC/FN entry in the dynamic area
; 			
PROC4:			LD	DE,(HL)			; HL: Address of pointer; fetch entity address in DE
			LD	HL,3
			ADD     HL,SP
			CALL    NXT             	; Allow space before "("
			PUSH    DE              	; Exchange DE and IY
			EX      (SP),IY
			CP      '('             	; Arguments?
			POP     DE			; NB: This has been moved after the compare otherwise DE gets corrupted later? IDK why!?!
			JR      NZ,PROC5
			CALL    NXT             	; Allow space before "("
			CP      '('
			JP      NZ,SYNTAX       	; Throw "Syntax error"
			PUSH    IY
			POP     BC              	; Save IY in BC
			EXX
			CALL    SAVLOC          	; Save local parameters
			CALL    BRAKET          	; Closing bracket
			EXX
			PUSH    BC
			POP     IY              	; Restore IY
			PUSH    HL
			CALL    ARGUE           	; Transfer arguments
			POP     HL
;
PROC5:			INC	HL			; Increment to the ON PROC flag address
			LD	A, (HL)			; And fetch the value
			DEC 	HL
			LD	(HL), DE		; Save the ENDPROC return address pointer in the BASIC listing
			CP	TON			; Was it "ON PROC"?
			JP	NZ, XEQ			; No, so back to XEQ
			PUSH    DE			; Exchange DE and IY
			EX      (SP),IY
			CALL    SPAN            	; Skip rest of ON list
			EX      (SP),IY			; Exchange DE and IY
			POP     DE
			LD	(HL), DE		; Save the return address
			JP      XEQ

; LOCAL var[,var...]
;
LOCAL_EX_:			POP     BC			; BC: The current check marker (on the stack)
			PUSH    BC
			LD      HL,FNCHK		; Check if we are in a FN
			OR      A
			SBC     HL,BC
			JR      Z,LOCAL1		; Yes, so all good, we can use local			
			LD      HL,PROCHK		; Now check if we are in a PROC
			OR      A
			SBC     HL,BC
			JR      Z,LOCAL1		; Again, all good, we can use local
			LD      HL,LOCCHK		; Finally check for the local parameters marker
			OR      A
			SBC     HL,BC			; If it is not present, then
			LD      A,12
			JP      NZ,ERROR_        	; Then throw a "Not LOCAL" errr
;
; At this point we are adding a local variable into a PROC or FN
;
LOCAL1:			PUSH    IY			; IY: BASIC pointer
			POP     BC			; BC: Copy of the BASIC pointer
			EXX
			DEC     IY
			CALL    SAVLOC
			EXX
			PUSH    BC
			POP     IY
;			
LOCAL2:			CALL    GETVAR			; Get the variable location
			JP      NZ,SYNTAX
			OR      A               	; Check the variable type (80h = string)
			EX      AF,AF'
			CALL    ZERO			; Zero the variable anyway
			EX      AF,AF'
			PUSH    AF
			CALL    P,STORE         	; Call STORE if it is not a string
			POP     AF
			LD      E,C
			CALL    M,STORES		; Call STORES if it is a string
			CALL    NXT			; Skip to the next character in the expression
			CP      ','			; Is it a comma?
			JP      NZ,XEQ			; No, so we're done, carry on executing
			INC     IY			; Yes, so skip the comma
			CALL    NXT			; And any whitespace
			JR      LOCAL2			; Then loop back and handle any further local variables

; ENDPROC
;
ENDPRO:			POP     BC			; Pop the check value off the stack
			LD      HL,LOCCHK		; Check if it is the LOCAL Marker
			OR      A
			SBC     HL,BC
			JR      Z,UNSTK         	; Yes, it is, so first need to unstack the local variables
;
			LD      HL,PROCHK       	; Check if it is the PROC marker
			OR      A
			SBC     HL,BC
			POP     IY
			JP      Z,XEQ			; Yes, it is, so carry on, all is good
			LD      A,13			; Otherwise throw the "No PROC" error
			JP      ERROR_
;
UNSTK:			POP     IX			; Unstack a single local variable
			POP     BC
			LD      A,B
			OR      A
			JP      M,UNSTK1        	; Jump here if it is a string? (80h)
			POP     HL			; Unstack a normal variable
			EXX
			POP     HL
			EXX
			CALL    STORE			; TODO: Not sure why or where it is being stored at this point
			JR      ENDPRO			; And loop back to ENDPRO
;
UNSTK1:			LD      HL,0			; Unstack a string
			ADD     HL,SP
			LD      E,C			
			CALL    STORES			; TODO: Not sure why or where it is being stored at this point
			LD      SP,HL
			JR      ENDPRO

; INPUT #channel,var,var...
;
INPUTN:			CALL    CHNL            ;E = CHANNEL NUMBER
INPN1:			CALL    NXT
			CP      ','
			JP      NZ,XEQ
			INC     IY
			CALL    NXT
			PUSH    DE
			CALL    VAR_
			POP     DE
			PUSH    AF              ;SAVE TYPE
			PUSH    HL              ;VARPTR
			OR      A
			JP      M,INPN2         ;STRING
			CALL    OSBGET
			EXX
			LD      L,A
			EXX
			CALL    OSBGET
			EXX
			LD      H,A
			EXX
			CALL    OSBGET
			LD      L,A
			CALL    OSBGET
			LD      H,A
			CALL    OSBGET
			LD      C,A
			POP     IX
			POP     AF              ;RESTORE TYPE
			PUSH    DE              ;SAVE CHANNEL
			CALL    STORE
			POP     DE
			JR      INPN1
INPN2:			LD      HL,ACCS
INPN3:			CALL    OSBGET
			CP      CR
			JR      Z,INPN4
			LD      (HL),A
			INC     L
			JR      NZ,INPN3
INPN4:			POP     IX
			POP     AF
			PUSH    DE
			EX      DE,HL
			CALL    STACCS
			POP     DE
			JR      INPN1

; INPUT ['][SPC(x)][TAB(x[,y])]["prompt",]var[,var...]
; INPUT LINE [SPC(x)][TAB(x[,y])]["prompt",]var[,var...]
;
INPUT:			CP      '#'
			JR      Z,INPUTN
			LD      C,0             ;FLAG PROMPT
			CP      LINE_EX_
			JR      NZ,INPUT0
			INC     IY              ;SKIP "LINE"
			LD      C,80H
INPUT0:			LD      HL,BUFFER
			LD      (HL),CR         ;INITIALISE EMPTY
INPUT1:			CALL    TERMQ
			JP      Z,XEQ           ;DONE
			INC     IY
			CP      ','
			JR      Z,INPUT3        ;SKIP COMMA
			CP      ';'
			JR      Z,INPUT3
			PUSH    HL              ;SAVE BUFFER POINTER
			CP      34		;ASCII ""
			JR      NZ,INPUT6
			PUSH    BC
			CALL    CONS
			POP     BC
			CALL    PTEXT           ;PRINT PROMPT
			JR      INPUT9
INPUT6:			CALL    FORMAT          ;SPC, TAB, '
			JR      NZ,INPUT2
INPUT9:			POP     HL
			SET     0,C             ;FLAG NO PROMPT
			JR      INPUT0
INPUT2:			DEC     IY
			PUSH    BC
			CALL    VAR_
			POP     BC
			POP     HL
			PUSH    AF              ;SAVE TYPE
			LD      A,(HL)
			INC     HL
			CP      CR              ;BUFFER EMPTY?
			CALL    Z,REFILL
			BIT     7,C
			PUSH    AF
			CALL    NZ,LINES
			POP     AF
			CALL    Z,FETCHS
			POP     AF              ;RESTORE TYPE
			PUSH    BC
			PUSH    HL
			OR      A
			JP      M,INPUT4        ;STRING
			PUSH    AF
			PUSH    IX
			CALL    VAL0
			POP     IX
			POP     AF
			CALL    STORE
			JR      INPUT5
INPUT4:			CALL    STACCS
INPUT5:			POP     HL
			POP     BC
INPUT3:			RES     0,C
			JR      INPUT1
;
REFILL:			BIT     0,C
			JR      NZ,REFIL0       ;NO PROMPT
			LD      A,'?'
			CALL    OUTCHR          ;PROMPT
			LD      A,' '
			CALL    OUTCHR
REFIL0:			LD      HL,BUFFER
			PUSH    BC
			PUSH    HL
			PUSH    IX
			CALL    OSLINE
			POP     IX
			POP     HL
			POP     BC
			LD      B,A             ;POS AT ENTRY
			XOR     A
			LD      (COUNT),A
			CP      B
			RET     Z
REFIL1:			LD      A,(HL)
			CP      CR
			RET     Z
			INC     HL
			DJNZ    REFIL1
			RET

; READ var[,var...]
;
READ:			CP      '#'
			JP      Z,INPUTN
			LD      HL,(DATPTR)
READ0:			LD      A,(HL)
			INC     HL              ;SKIP COMMA OR "DATA"
			CP      CR              ;END OF DATA STMT?
			CALL    Z,GETDAT
			PUSH    HL
			CALL    VAR_
			POP     HL
			OR      A
			JP      M,READ1         ;STRING
			PUSH    HL
			EX      (SP),IY
			PUSH    AF              ;SAVE TYPE
			PUSH    IX
			CALL    EXPRN
			POP     IX
			POP     AF
			CALL    STORE
			EX      (SP),IY
			JR      READ2
READ1:			CALL    FETCHS
			PUSH    HL
			CALL    STACCS
READ2:			POP     HL
			LD      (DATPTR),HL
			CALL    NXT
			CP      ','
			JP      NZ,XEQ
			INC     IY
			CALL    NXT
			JR      READ0
;
GETDAT:			LD      A,DATA_EX_
			CALL    SEARCH_EX
			INC     HL
			RET     NC
			LD      A,42
ERROR4:			JP      ERROR_           ;"Out of DATA"

; IF expr statement
; IF expr THEN statement [ELSE statement]
; IF expr THEN line [ELSE line]
;
IF_:			CALL    EXPRI
			CALL    TEST
			JR      Z,IFNOT         ;FALSE
			LD      A,(IY)
			CP      THEN_EX_
			JP      NZ,XEQ
			INC     IY              ;SKIP "THEN"
IF1:			CALL    NXT
			CP      LINO_EX
			JP      NZ,XEQ          ;STATEMENT FOLLOWS
			JP      GOTO_EX            ;LINE NO. FOLLOWS
IFNOT:			LD      A,(IY)
			CP      CR
			INC     IY
			JP      Z,XEQ0          ;END OF LINE
			CP      ELSE_EX_
			JR      NZ,IFNOT
			JR      IF1

; CLS
;
CLS:		CALL    CLRSCN
			XOR     A
			LD      (COUNT),A
			JP      XEQ

; STOP
;
STOP:			CALL    TELL
			DB	CR
			DB	LF
			DB	TSTOP
			DB	0
			CALL    SETLIN          ;FIND CURRENT LINE
			CALL    SAYLN
			CALL    CRLF
			JP      CLOOP

; REPORT
;
REPOR:			CALL    REPORT
			JP      XEQ

; CLEAR
;
CLR:			CALL    CLEAR
			LD      HL,(PAGE_)
			JR      RESTR1

; RESTORE [line]
;
RESTOR_EX:			LD      HL,(PAGE_)
			CALL    TERMQ
			JR      Z,RESTR1
			CALL    ITEMI
			EXX
			CALL    FINDL           ;SEARCH FOR LINE
			LD      A,41
			JP      NZ,ERROR4       ;"No such line"
RESTR1:			LD      A,DATA_EX_
			CALL    SEARCH_EX
			LD      (DATPTR),HL
			JP      XEQ

; PTR#channel=expr
; PAGE=expr
; TIME=expr
; LOMEM=expr
; HIMEM=expr
;
PTR_EX:			CALL    CHANEL
			CALL    EQUALS
			LD      A,E
			PUSH    AF
			CALL    EXPRI
			PUSH    HL
			EXX
			POP     DE
			POP     AF
			CALL    PUTPTR
			JP      XEQ
;
PAGEV_EX:			CALL    EQUALS
			CALL    EXPRI
			EXX
			LD      L,0
			LD      (PAGE_),HL
			JP      XEQ
;
TIMEV_EX:			CP      '$'
			JR      Z,TIMEVS_EX
			CALL    EQUALS
			CALL    EXPRI
			PUSH    HL
			EXX
			POP     DE
			CALL    PUTIME
			JP      XEQ
;
TIMEVS_EX:			INC     IY              ;SKIP '$'
			CALL    EQUALS
			CALL    EXPRS
			CALL    PUTIMS
			JP      XEQ
;
LOMEMV_EX:			CALL    EQUALS
			CALL    EXPRI
			CALL    CLEAR
			EXX
			LD      (LOMEM),HL
			LD      (FREE),HL
			JP      XEQ
;
HIMEMV_EX:			CALL    EQUALS			; Check for '=' and throw an error if not found
			CALL    EXPRI			; Load the expression into registers
; BEGIN MISSING FROM BINARY
			; LD	A,L			;  A: The MSB of the 24-bit value
			; EXX				; HL: The LSW of the 24-bit value
			; LD	(R0),HL
			; LD	(R0+2),A
			; LD	HL,(FREE)
			; LD      DE,256
			; ADD	HL,DE 
			; EX	DE,HL			; DE: FREE + 256
			; LD	HL,(R0)			; HL: The passed expression
; END MISSING FROM BINARY
; BEGIN ADDED FROM BINARY
			exx
			ld de,(FREE)
			inc d
; END ADDED FROM BINARY
			XOR     A
			SBC     HL,DE
			ADD     HL,DE			; Do a bounds check
			JP      C,ERROR_         	; Throw the error: "No room"
			LD      DE,(HIMEM)
			LD      (HIMEM),HL
			EX      DE,HL
			SBC     HL,SP			; Adjust the stack
			JP      NZ,XEQ
			EX      DE,HL
			LD      SP,HL           	; Load the SP
			JP      XEQ

; WIDTH expr
;
WIDTHV:			CALL    EXPRI
			EXX
			LD      A,L
			LD      (WIDTH),A
			JP      XEQ

; TRACE ON
; TRACE OFF
; TRACE line
;
TRACE_EX:			INC     IY
			LD      HL,0
			CP      TON
			JR      Z,TRACE0
			CP      OFF_
			JR      Z,TRACE1
			DEC     IY
			CALL    EXPRI
			EXX
TRACE0:			DEC     HL
TRACE1:			LD      (TRACEN),HL
			JP      XEQ

; VDU expr,expr;....
;
; BEGIN MISSING FROM BINARY
; VDU:			LD	IX,BUFFER		; Storage for the VDU stream
; VDU1:			PUSH	IX
; 			CALL    EXPRI			; Fetch the VDU character
; 			POP	IX
; 			EXX
; 			LD	(IX+0),L		; Write out the character to the buffer
; 			INC	IX 
; 			LD      A,(IY)			;  A: The separator character
; 			CP      ','			; Is it a comma?
; 			JR      Z,VDU2			; Yes, so it's a byte value - skip to next expression
; 			CP      ';'			; Is it a semicolon?
; 			JR      NZ,VDU3			; No, so skip to the next expression
; 			LD	(IX+0),H		; Write out the high byte to the buffer
; 			INC	IX 
; VDU2:			INC     IY			; Skip to the next character
; VDU3:			CALL    TERMQ			; Skip past white space
; 			JR      NZ,VDU1			; Loop unti reached end of the VDU command
; 			LD	A,IXL			;  A: Number of bytes to write out 
; 			OR	A
; 			JR 	Z,VDU4			; No bytes to write, so skip the next bit
; 			LD	HL,BUFFER		; HL: Start of stream
; 			LD	BC,0
; 			LD	C,A			; BC: Number of bytes to write out
; 			RST.LIL	18h			; Output the buffer to MOS
; END MISSING FROM BINARY
; BEGIN ADDED FROM BINARY
VDU:
			call EXPRI
			exx
			ld a,l
			call OSWRCH
			ld a,(iy)
			cp $2c
			jr z,$+$0b
			cp $3b
			jr nz,$+$09
			ld a,h
			call OSWRCH
			inc iy
			call TERMQ
			jr nz,$-$20
; END ADDED FROM BINARY
VDU4:			JP      XEQ

; CLOSE channel number
;
CLOSE:			CALL    CHANEL			; Fetch the channel number
			CALL    OSSHUT			; Close the channel
			JP      XEQ

; BPUT channel,byte
;
BPUT:			CALL    CHANEL          	; Fetch the channel number
			PUSH    DE			; DE: Channel number
			CALL    COMMA			; Skip to the next expression
			CALL    EXPRI           	; Feth the data
			EXX
			LD      A,L			; A: The byte to write
			POP     DE
			CALL    OSBPUT			; Write the byte out
			JP      XEQ

; CALL address[,var[,var...]]
;
; Note that the parameter table differs from the Z80 version
; Each entry now takes up 4 bytes, not 3, so the table is now:
;  -1 byte:  Number of parameters
; Then, for each parameter:
;  -1 byte:  Parameter type (00h: byte, 04h: word, 05h: real, 80h: fixed string, 81h: dynamic string)
;  -3 bytes: Parameter address
;
; See https://www.bbcbasic.co.uk/bbcbasic/mancpm/bbckey1.html#callparms for more information
;
CALL_:			CALL    EXPRI           	; Fetch the address
			LD	A,L			;  A: MSB of address
			EXX
			LD	(R0+0),HL		; HL: LSW of address
			LD	(R0+2),A		
			LD      B,0             	;  B: The parameter counter
			LD      DE,BUFFER       	; DE: Vector
;
CALL1:			CALL    NXT			; Skip whitespace
			CP      ','			; Check for comma
			JR      NZ,CALL2		; If no more parameters, then jump here
			INC     IY			; Skip to the next character
			INC     B			; Increment the parameter count
			CALL    NXT			; Skip whitespace
			PUSH    BC
			PUSH    DE
			CALL    VAR_
			POP     DE
			POP     BC
			INC     DE
			LD      (DE),A			; Save the parameter type
			INC     DE
			EX      DE,HL
			LD	(HL),DE			; Save the parameter address (3 bytes)
			INC	HL
			INC	HL
			INC	HL
			EX      DE,HL
			JR      CALL1
;
CALL2:			LD      A,B
			LD      (BUFFER),A      	; Save the parameter count
			LD	HL,(R0)			; HL: Address of the code
			CALL    USR1			; And call it
			JP      XEQ

; USR(address)
;
USR:			CALL    ITEMI			; Evaluate the expression
			LD	A,L			;  A: MSB of address
			EXX
			LD	(R0+0),HL		; HL: LSW of address
			LD	(R0+2),A		
			LD	HL,(R0)			; Get the 24-bit address in HL
;
USR1:			PUSH    HL              	; Address on stack
			EX      (SP),IY
			INC     H               	; Check for PAGE &00FFxx
			OR	H
			LD      HL,USR2         	; Return address
			PUSH    HL
			LD      IX,STAVAR
			CALL    Z,OSCALL        	; Intercept &00FFxx
;
			LD      C, (IX+24)		; F%
			PUSH    BC
;
			LD	A, (IX+8)		; B% -> MSW 
			LD	(R0+1), A 		
			LD	A, (IX+9)
			LD	(R0+2), A 
			LD	A, (IX+12)		; C% -> LSB 
			LD	(R0+0), A 
			LD	BC, (R0)
;
			LD	A, (IX+16)		; D% -> MSW 
			LD	(R0+1), A 		
			LD	A, (IX+17)
			LD	(R0+2), A 
			LD	A, (IX+20)		; E% -> LSB 
			LD	(R0+0), A 
			LD	DE, (R0)
;
			LD	A, (IX+32)		; H% -> MSW 
			LD	(R0+1), A 		
			LD	A, (IX+33)
			LD	(R0+2), A 
			LD	A, (IX+48)		; L% -> LSB 
			LD	(R0+0), A 
			LD	HL, (R0)
;
			POP     AF			; F%
			LD      A, (IX+4)        	; A%

			LD      IX,BUFFER
			JP      (IY)            	; Off to user routine
;
USR2:			POP     IY	
			XOR     A
			LD      C,A
			RET

; PUT port,data
;
PUT:			CALL    EXPRI           ;PORT ADDRESS
			EXX
			PUSH    HL
			CALL    COMMA
			CALL    EXPRI           ;DATA
			EXX
			POP     BC
			OUT     (C),L           ;OUTPUT TO PORT BC
			JP      XEQ

; SUBROUTINES -----------------------------------------------------------------

; ASSIGN - Assign a numeric value to a variable.
; Outputs: NC,  Z - OK, numeric.
;          NC, NZ - OK, string.
;           C, NZ - illegal
;
ASSIGN:			CALL    GETVAR          	; Try to get the variable
			RET     C               	; Return with C if it is an illegal variable
			CALL    NZ,PUTVAR		; If it does not exist, then create the variable
			OR      A
			RET     M               	; Return if type is string (81h)
			PUSH    AF              	; It's a numeric type from this point on
			CALL    EQUALS			; Check if the variable is followed by an '=' symbol; this will throw a 'Mistake' error if not
			PUSH    HL
			CALL    EXPRN
			POP     IX
			POP     AF
STORE:			BIT     0,A
			JR      Z,STOREI
			CP      A               	; Set the variable to 0
STORE5:			LD      (IX+4),C
STORE4:			EXX
			LD      (IX+0),L
			LD      (IX+1),H
			EXX
			LD      (IX+2),L
			LD      (IX+3),H
			RET
STOREI:			PUSH    AF
			INC     C               ;SPEED - & PRESERVE F'
			DEC     C               ; WHEN CALLED BY FNEND0
			CALL    NZ,SFIX         ;CONVERT TO INTEGER
			POP     AF
			CP      4
			JR      Z,STORE4
			CP      A               ;SET ZERO
STORE1:			EXX
			LD      (IX+0),L
			EXX
			RET
;
; Copy a string from the string accumulator to variable storage on the stack
; Parameters:
; - AF: The variable type (should be 81h for a string, 80h for a fixed/static string)
; - IX: Address of the variable storage on the stack
;
STACCS:			LD      HL,ACCS			; HL: Pointer to the string accumulator
;
; Parameters:
; As above, but:
; - HL: Address of the string to be stored
; -  E: The string length
; NB:
; Strings are mutable
; Strings are stored in the following format in the variable:
; - Address of the next variable (3 bytes)
; - The rest of the variable name - this is zero terminated
; - Current string length (byte)
; - Maximum (original) string length (byte)
; - String start address (3 bytes for BBC BASIC for eZ80, 2 bytes for standard BBC BASIC for Z80)
; See https://www.bbcbasic.co.uk/bbcbasic/mancpm/annexd.html#string for more details
;
STORES:			RRA				; Rotate right to shift bit 0 into carry
			JR      NC,STORS3		; It's a fixed/static string, so skip the next bit
			PUSH    HL			; Stack ACCS
;
; Load the string pointer and lengths into registers - these are all zeroed for new strings
;
			EXX				; This block was a call to LOAD4
			LD      L,(IX+0)		; The length of the string currently stored in the allocated space
			LD      H,(IX+1)		; The maximum original string length
			EXX
			LD	HL,(IX+2)		; Address of the string (24-bit)
;
			LD      A,E             	; E : Length of string in ACCS (as passed to the function)
			EXX
			LD      L,A			; L': Length of string currently stored on the stack
			LD      A,H             	; H': The maximum (original) string length
			EXX
			CP      E			; Check whether there is enough room for the string in ACCS in the allocated space
			JR      NC,STORS1       	; Yes there is, so skip the next bit
;
; At this point we're either initialising a new string or assigning more memory to an existing string
; Note that there is no garbage collection here, so if a string is reassigned and the new string is longer
; then the existing and new strings may both exist in memory.
;
			EXX
			LD      H,L			; H: Set the maximum string length to the string length
			EXX
			PUSH    HL
			LD	BC, 0			
			LD      C,A			; BC: The maximum (original) string length
			ADD     HL,BC			; Work out whether this is the last string in memory
			LD      BC,(FREE)
			SBC     HL,BC			; Is string last?
			POP     HL
			SCF
			JR      Z,STORS1
			; LD	HL, BC			; HL=BC
			push bc 
			pop hl
; 
; At this point carry flag will be clear if the string can be replaced in memory, otherwise will be set
; - H': The maximum (original) string length
; - L': The actual string length (must be less than H')
; - HL: Address of the string in memory
;
STORS1:			EXX				; This block was a call to STORE4
			LD      (IX+0),L		; The actual string length (must be less then H')
			LD      (IX+1),H		; The maximum (original) string length
			EXX
			LD	(IX+2),HL		; The pointer to the original string
;			
			LD	BC, 0
			LD      C,E			; BC: The new string length
			EX      DE,HL
			POP     HL
			DEC     C			; Strings can only be 255 bytes long, so this is a quick way to
			INC     C			; check whether BC is 0 without affecting the carry flag
			RET     Z               	; It is, so it's a NULL string, don't need to do anything else here
			LDIR				; Replace the string in memory
			RET     NC
			LD      (FREE),DE		; Set the new value of FREE and fall through to CHECK
;
; Check whether the stack is full
;
CHECK:			PUSH    HL
			PUSH	BC
			LD      HL,(FREE)		; HL: Address of first free space byte
			LD	BC,100h			; BC: One page of memory
			ADD	HL,BC			; Add a page to FREE
			SBC     HL,SP			; And subtract the current SP
			POP	BC
			POP     HL
			RET     C			; The SP is not in the same page, so just return
			XOR     A			; Otherwise
			JP      ERROR_			; Throw error "No room"
;
STORS3:			LD	BC,0
			LD      C,E			; BC: String length
			PUSH    IX
			POP     DE			; DE: Destination
			XOR     A			; Check if string length is 0
			CP      C
			JR      Z,STORS5		; Yes, so don't copy
			LDIR
STORS5:			LD      A,CR			; Finally add the terminator
			LD      (DE),A
			RET

; ARGUE: TRANSFER FN OR PROC ARGUMENTS FROM THE
;  CALLING STATEMENT TO THE DUMMY VARIABLES VIA
;  THE STACK.  IT MUST BE DONE THIS WAY TO MAKE
;  PROCFRED(A,B)    DEF PROCFRED(B,A)     WORK.
;    Inputs: DE addresses parameter list 
;            IY addresses dummy variable list
;   Outputs: DE,IY updated
;  Destroys: Everything
;
ARGUE:			LD      A,-1
			PUSH    AF              	; Put marker on the stack
ARGUE1:			INC     IY              	; Bump past '(' or ',''
			INC     DE
			PUSH    DE
			CALL    NXT			; Skip any whitespace
			CALL    GETVAR			; Get the location of the variable in HL/IX
			JR      C,ARGERR		; If the parameter contains an illegal character then throw an error
			CALL    NZ,PUTVAR
			POP     DE
			PUSH    HL              	; VARPTR
			OR      A               	; Check the variable type
			PUSH    AF
			PUSH    DE
			EX      (SP),IY
			JP      M,ARGUE2        	; Jump here if it is a string
;
			CALL    EXPRN           	; At this point it is numeric, so get the numeric expression value
			EX      (SP),IY
			POP     DE
			POP     AF
			EXX
			PUSH    HL
			EXX
			PUSH    HL
			LD      B,A
			PUSH    BC
			CALL    CHECK           	; Check room
			JR      ARGUE4
;
ARGUE2:			CALL    EXPRS			; At this point it is a string variable, so get the string expression value
			EX      (SP),IY
			EXX
			POP     DE
			EXX
			POP     AF
			CALL    PUSHS
			EXX
;
ARGUE4:			CALL    NXT			; Skip whitespace
			CP      ','			; Check to see if the next value is a comma
			JR      NZ,ARGUE5		; No, so jump here
			LD      A,(DE)	
			CP      ','			; Are there any more arguments?
			JR      Z,ARGUE1        	; Yes, so loop
;
ARGERR:			LD      A,31
			JP      ERROR_           	; Throw error "Arguments"
;
ARGUE5:			CALL    BRAKET			; Check for end bracket (throws an error if missing)
			LD      A,(DE)
			CP      ')'
			JR      NZ,ARGERR
			INC     DE
			EXX
ARGUE6:			POP     BC
			LD      A,B
			INC     A
			EXX
			RET     Z               	; Marker popped
			EXX
			DEC     A
			JP      M,ARGUE7        	; If it is a string, then jump here
			POP     HL
			EXX
			POP     HL
			EXX
			POP     IX
			CALL    STORE	           	; Write to dummy variable
			JR      ARGUE6
;
ARGUE7:			CALL    POPS
			POP     IX
			CALL    STACCS
			JR      ARGUE6

; SAVLOC: SUBROUTINE TO STACK LOCAL PARAMETERS
;   OF A FUNCTION OR PROCEDURE.
; THERE IS A LOT OF STACK MANIPULATION - CARE!!
;    Inputs: IY is parameters pointer
;   Outputs: IY updated
;  Destroys: A,B,C,D,E,H,L,IX,IY,F,SP
;
SAVLOC:			POP     DE              	; DE: Return address (from the CALL)
;
SAVLO1:			INC     IY              	; Bump past '(' or ','
			CALL    NXT			; And also any whitespace
			PUSH    DE			; Push the return address back onto the stack
			EXX
			PUSH    BC
			PUSH    DE
			PUSH    HL
			EXX
			CALL    VAR_             	; Dummy variable
			EXX
			POP     HL
			POP     DE
			POP     BC
			EXX
			POP     DE
			OR      A               	; Check the variable type
			JP      M,SAVLO2        	; 80h = string, so jump to save a local string
			EXX
			PUSH    HL              	; Save H'L'
			EXX
			LD      B,A             	;  B: Variable type
			CALL    LOADN
			EXX
			EX      (SP),HL
			EXX
			PUSH    HL
			PUSH    BC
			JR      SAVLO4
;
SAVLO2:			PUSH    AF              	; Save the type (string)
			PUSH    DE
			EXX
			PUSH    HL
			EXX
			CALL    LOADS
			EXX
			POP     HL
			EXX
			LD	BC,0
			LD      C,E			; BC: String length
			POP     DE
			CALL    CHECK			; Check if there is space on the stack
			POP     AF              	; Level stack
			LD      HL,0
			SBC     HL,BC			; HL: Number of bytes required on the stack for the string
			ADD     HL,SP			; Make space for the string on the stack
			LD      SP,HL
			LD      B,A             	;  B: Variable type
			PUSH    BC
			JR      Z,SAVLO4
			PUSH    DE
			LD      DE,ACCS
			EX      DE,HL
			LD      B,L
			LDIR                    	; Save the string onto the stack
			POP     DE
;
SAVLO4:			PUSH    IX			; VARPTR
			CALL    SAVLO5
LOCCHK:			EQU     $
SAVLO5:			CALL    CHECK
			CALL    NXT
			CP      ','             	; Are there any more local variables?
			JR      Z,SAVLO1		; Yes, so loop
			EX      DE,HL			; DE -> HL: The return address
			JP      (HL)            	; And effectvely return
;
DELIM:			LD      A,(IY)          	; Assembler delimiter
			CP      ' '
			RET     Z
			CP      ','
			RET     Z
			CP      ')'
			RET     Z
TERM:			CP      ';'             	; Assembler terminator
			RET     Z
			CP      '\'
			RET     Z
			JR      TERM0
;
TERMQ:			CALL    NXT
			CP      ELSE_EX_
			RET     NC
TERM0:			CP      ':'             	; Assembler seperator
			RET     NC
			CP      CR
			RET
;
SPAN:			CALL    TERMQ
			RET     Z
			INC     IY
			JR      SPAN
;
; This snippet is used to check whether an expression is followed by an '=' symbol
;
EQUALS:			CALL    NXT			; Skip whitespace
			INC     IY			; Skip past the character in question
			CP      '='			; Is it '='
			RET     Z			; Yes, so return
			LD      A,4			; Otherwise
			JP      ERROR_           	; Throw error "Mistake"
;
FORMAT:			CP      TAB
			JR      Z,DOTAB
			CP      SPC
			JR      Z,DOSPC
			CP      '''
			RET     NZ
			CALL    CRLF
			XOR     A
			RET
;
DOTAB:			PUSH    BC
			CALL    EXPRI
			EXX
			POP     BC
			LD      A,(IY)
			CP      ','
			JR      Z,DOTAB1
			CALL    BRAKET
			LD      A,L
TABIT:			LD      HL,COUNT
			CP      (HL)
			RET     Z
			PUSH    AF
			CALL    C,CRLF
			POP     AF
			SUB     (HL)
			JR      FILL
DOTAB1:			INC     IY
			PUSH    BC
			PUSH    HL
			CALL    EXPRI
			EXX
			POP     DE
			POP     BC
			CALL    BRAKET
			CALL    PUTCSR
			XOR     A
			RET
;
DOSPC:			PUSH    BC
			CALL    ITEMI
			EXX
			LD      A,L
			POP     BC
FILL:			OR      A
			RET     Z
			PUSH    BC
			LD      B,A
FILL1:			LD      A,' '
			CALL    OUTCHR
			DJNZ    FILL1
			POP     BC
			XOR     A
			RET
;
PTEXT:			LD      HL,ACCS
			INC     E
PTEXT1:			DEC     E
			RET     Z
			LD      A,(HL)
			INC     HL
			CALL    OUTCHR
			JR      PTEXT1
;
FETCHS:			PUSH    AF
			PUSH    BC
			PUSH    HL
			EX      (SP),IY
			CALL    XTRACT
			CALL    NXT
			EX      (SP),IY
			POP     HL
			POP     BC
			POP     AF
			RET
;
LINES:			LD      DE,ACCS
LINE1S:			LD      A,(HL)
			LD      (DE),A
			CP      CR
			RET     Z
			INC     HL
			INC     E
			JR      LINE1S
;
XTRACT:			CALL    NXT
			CP      '"'
			INC     IY
			JP      Z,CONS
			DEC     IY
			LD      DE,ACCS
XTRAC1:			LD      A,(IY)
			LD      (DE),A
			CP      ','
			RET     Z
			CP      CR
			RET     Z
			INC     IY
			INC     E
			JR      XTRAC1

; Search for a token at the start of a program line
; - HL: Pointer to the start of a tokenised line in the program area
; Returns:
; - HL: Pointer to the 
; -  F: Carry set if not found
; Corrupts:
; - BC
;
SEARCH_EX:			LD      BC,0			; Clear BC
;
SRCH1_EX:			LD      C,(HL)			;  C: Fetch the line length
			INC     C			; Check for 0, i.e. end of program marker
			DEC     C
			JR      Z,SRCH2_EX         	; Not found the token, so end
			INC     HL			; Skip the line length and line number
			INC     HL
			INC     HL
			CP      (HL)			; Compare with the token
			RET     Z			; Found it, so return with carry not set
			DEC     C			; Skip to the next line
			DEC     C
			DEC     C
			ADD     HL,BC
			JR      SRCH1_EX			; Rinse, lather and repeat
; 			
SRCH2_EX:			DEC     HL              	; Token not found, so back up to the CR at the end of the last line
			SCF				; And set the carry flag
			RET

; Multiply by 4 or 5
; This function is used to allocate space for dimensioned variables
; This is a 24-bit operation
; - DE: Number to multiple
; -  A: 04h (Integer) - takes up 4 bytes
;       05h (Float)   - takes up 5 bytes
;       81h (String)  - takes up 5 bytes - this is different from BBC BASIC for Z80 where strings only take up 4 bytes
; Returns:
; - DE: Multiplied by 4 if A = 4, otherwise multiplies by 5
; -  F: Carry if overflow
; Corrupts:
; - HL
X4OR5:			CP      4			; Check A = 4 (Z flag is used later)
			; LD	HL,DE
			push de
			pop hl
			ADD     HL,HL			; Multiply by 2 (note this operation preserves the zero flag)
			RET     C			; Exit if overflow
			ADD     HL,HL			; Multiply by 2 again
			RET     C			; Exit if overflow
			EX      DE,HL			; DE: Product
			RET     Z			; Exit if A = 4
			ADD     HL,DE			; Add original value to HL (effectively multiplying by 5)
			EX      DE,HL			; DE: Product
			RET

; 16-bit unsigned multiply
; - HL: Operand 1
; - BC: Operand 2
; Returns:
; - HL: Result
; -  F: C if overflow
;
MUL16:			PUSH	BC
			LD	D, C			; Set up the registers for the multiplies
			LD	E, L		
			LD	L, C
			LD	C, E
			MLT	HL			; HL = H * C (*256)
			MLT	DE			; DE = L * C
			MLT	BC			; BC = B * L (*256)
			ADD	HL, BC			; HL = The sum of the two most significant multiplications
			POP	BC
			XOR	A
			SBC	H			; If H is not zero then it's an overflow
			RET	C
			LD	H, L			; HL = ((H * C) + (B * L) * 256) + (L * C)
			LD	L, A
			ADD	HL, DE
			RET
;
CHANEL:			CALL    NXT			; Skip whitespace
			CP      '#'			; Check for the '#' symbol
			LD      A,45	
			JP      NZ,ERROR_        	; If it is missing, then throw a "Missing #" error
CHNL:			INC     IY             		; Bump past the '#'
			CALL    ITEMI			; Get the channel number
			EXX
			EX      DE,HL			; DE: The channel number
			RET

; ASSEMBLER -------------------------------------------------------------------

; Language independant control section:
;  Outputs: A=delimiter, carry set if syntax error.
;
ASSEM:			CALL    SKIP
			INC     IY
			CP      ':'
			JR      Z,ASSEM
			CP      ']'
			RET     Z
			CP      CR
			RET     Z
			DEC     IY
			LD      IX,(PC)         	; Program counter (P% - defined in equs.inc)
			LD      HL,LISTON
			BIT     6,(HL)
			JR      Z,ASSEM0
			LD      IX,(OC)         	; Code origin (O% - defined in equs.inc)
ASSEM0:			PUSH    IX
			PUSH    IY
			CALL    ASMB
			POP     BC
			POP     DE
			RET     C
			CALL    SKIP
			SCF
			RET     NZ
			DEC     IY
ASSEM3:			INC     IY
			LD      A,(IY)
			CALL    TERM0
			JR      NZ,ASSEM3
			LD      A,(LISTON)
			PUSH    IX
			POP     HL
			OR      A
			SBC     HL,DE
			EX      DE,HL           	; DE: Number of bytes
			PUSH    HL
			LD      HL,(PC)
			PUSH    HL
			ADD     HL,DE
			LD      (PC),HL         	; Update PC
			BIT     6,A
			JR      Z,ASSEM5
			LD      HL,(OC)
			ADD     HL,DE
			LD      (OC),HL         	; Update OC
ASSEM5:			POP     HL              	; Old PC
			POP     IX              	; Code here
			BIT     4,A
			JR      Z,ASSEM
			LD	(R0),HL			; Store HL in R0 so we can access the MSB
			LD	A,(R0+2)		; Print out the address
			CALL	HEX_EX
			LD      A,H			
			CALL    HEX_EX
			LD      A,L
			CALL    HEXSP
			XOR     A
			CP      E
			JR      Z,ASSEM2
;
ASSEM1:			LD      A,(COUNT)
			CP      20
			LD      A,7
			CALL    NC,TABIT        	; Next line
			LD      A,(IX)
			CALL    HEXSP
			INC     IX
			DEC     E
			JR      NZ,ASSEM1
;
ASSEM2:			LD      A,22			; Tab to the disassembly field
			CALL    TABIT
			PUSH    IY
			POP     HL
			SBC     HL,BC
ASSEM4:			LD      A,(BC)
			CALL    OUT_
			INC     BC
			DEC     L
			JR      NZ,ASSEM4
			CALL    CRLF
			JP      ASSEM
;
HEXSP:			CALL    HEX_EX
			LD      A,' '
			JR      OUTCH1
HEX_EX:			PUSH    AF
			RRCA
			RRCA
			RRCA
			RRCA
			CALL    HEXOUT
			POP     AF
HEXOUT:			AND     0FH
			ADD     A,90H
			DAA
			ADC     A,40H
			DAA
OUTCH1:			JP      OUT_
	
; Processor Specific Translation Section:
;
; Register Usage: B: Type of most recent operand (the base value selected from the opcode table)
;                 C: Opcode beig built
;                 D: Flags
;			Bit 7: Set to 1 if the instruction uses long addressing
;			Bit 6: Set to 1 if the instruction is an index instruction with offset
;                 E: Offset from IX or IY
;                HL: Numeric operand value
;                IX: Code destination pointer
;                IY: Source text pointer
;    Inputs: A = initial character
;   Outputs: Carry set if syntax error.
;
ASMB:			CP      '.'			; Check for a dot; this indicates a label
			JR      NZ,ASMB1		; No, so just process the instruction
			INC     IY			; Skip past the dot to the label name
			PUSH    IX			; Store the code destination pointer
			CALL    VAR_			; Create a variable
			PUSH    AF
			CALL    ZERO			; Zero it
			LD	A,(PC+2)
			LD	L,A			; The MSB of the 24-bit address
			EXX
			LD      HL,(PC)			; The LSW of the 24-bit address (only 16-bits used)
			EXX
			POP     AF
			CALL    STORE			; Store the program counter
			POP     IX			; Restore the code destination pointer
;			
ASMB1:			LD	A,(LISTON)		; Get the OPT flags
			AND	80H
			LD      D,A     		;  D: Clear the flags and set the initial ADL mode (copied from bit 7 of LISTON)
			CALL    SKIP			; Skip any whitespace
			RET     Z			; And return if there is nothing further to process
			CP      TCALL			; Check if it is the token CALL (it will have been tokenised by BASIC)
			LD      C,0C4H			;  A: The base operand
			INC     IY			; Skip past the token
			JP      Z,GROUP13_1		; And jump to GROUP13, which handles CALL
			DEC     IY			; Skip back, as we're not doing the above at this point
			LD      HL,OPCODS		; HL: Pointer to the eZ80 opcodes table
			CALL    FIND			; Find the opcode
			RET     C			; If not found, then return; carry indicates an error condition
			LD      C,B     		;  C: A copy of the opcode
;
; GROUP 0: Trivial cases requiring no computation
; GROUP 1: As Group 0, but with "ED" prefix
;
			SUB     68			; The number of opcodes in GROUP0 and GROUP1
			JR      NC,GROUP02		; If not in that range, then check GROUP2
			CP      15-68			; Anything between 15 and 68 (neat compare trick here)
			CALL    NC,ED			; Needs to be prefixed with ED
			JR      BYTE0			; Then write the opcode byte
;
; GROUP 2: BIT, RES, SET
; GROUP 3: RLC, RRC, RL, RR, SLA, SRA, SRL
;
GROUP02:		SUB     10			; The number of opcodes in GROUP2 and GROUP3
			JR      NC,GROUP04		; If not in that range, then check GROUP4
			CP      3-10			; 
			CALL    C,BIT_
			RET     C
			CALL    REGLO
			RET     C
			CALL    CB
			JR      BYTE0
;
; GROUP 4 - PUSH, POP, EX (SP)
;
GROUP04:		SUB     3			; The number of opcodes in GROUP4
			JR      NC,GROUP05		; If not in that range, then check GROUP5
GROUP04_1:		CALL    PAIR_EX				
			RET     C
			JR      BYTE0				
;
; GROUP 5 - SUB, AND, XOR, OR, CP
; GROUP 6 - ADD, ADC, SBC
;
GROUP05:		SUB     8+2			; The number of opcodes in GROUP5 and GROUP6
			JR      NC,GROUP07
			CP      5-8
			LD      B,7
			CALL    NC,OPND			; Get the first operand
			LD      A,B			
			CP      7			; Is the operand 'A'?
			JR      NZ,GROUP05_HL		; No, so check for HL, IX or IY
;			
GROUP05_1:		CALL    REGLO			; Handle ADD A,?
			LD      A,C
			JR      NC,BIND1		; If it is a register, then write that out
			XOR     46H			; Handle ADD A,n
			CALL    BIND
DB_:			CALL    NUMBER
			JP      VAL8
;
GROUP05_HL:		AND     3FH
			CP      12
			SCF
			RET     NZ
			LD      A,C
			CP      80H
			LD      C,9
			JR      Z,GROUP04_1
			XOR     1CH
			RRCA
			LD      C,A
			CALL    ED
			JR      GROUP04_1
;
; GROUP 7 - INC, DEC
;
GROUP07:		SUB     2			; The number of opcodes in GROUP7
			JR      NC,GROUP08
			CALL    REGHI
			LD      A,C
BIND1:			JP      NC,BIND
			XOR     64H
			RLCA
			RLCA
			RLCA
			LD      C,A
			CALL    PAIR1_EX
			RET     C
BYTE0:			LD      A,C
			JP      BYTE_
;
; Group 8: IN0, OUT0
;
GROUP08:		SUB	2			; The number of opcodes in GROUP8
			JR	NC,GROUP09
			CP	1-2
			CALL    Z,NUMBER		; Fetch number first if OUT
			EX      AF,AF'			; Save flags
			CALL    REG			; Get the register value regardless
			RET     C			; Return if not a register
			EX      AF,AF'			; Restore the flags
			CALL    C,NUMBER		; Fetch number last if IN
			LD	A,B			; Get the register number
			CP	6			; Fail on (HL)
			SCF
			RET	Z
			CP	8			; Check it is just single pairs only
			CCF
			RET	C			; And return if it is an invalid register
			RLCA				; Bind with the operand
			RLCA
			RLCA
			ADD	A,C
; BEGIN NOT IN BINARY
			; LD	C,A
			; CALL	ED			; Prefix with ED
			; LD	A,C
; END NOT IN BINARY
			CALL	BYTE_			; Write out the operand
			JP	VAL8			; Write out the value
;
; GROUP 9 - IN
; GROUP 10 - OUT
;
GROUP09:		SUB     2			; The number of opcodes in GROUP09 amd GROUP10
			JR      NC,GROUP11
			CP      1-2			; Check if Group 9 or Group 1
			CALL    Z,CORN			; Call CORN if Group 10 (OUT)
			EX      AF,AF'			; Save flags
			CALL    REGHI			; Get the register value regardless
			RET     C			; Return if not a register
			EX      AF,AF'			; Restore the flags
			CALL    C,CORN			; Call CORN if Group 9 (IN)
			INC     H			; If it is IN r,(C) or OUT (C),r then
			JR      Z,BYTE0			; Just write the operand out
;			
			LD      A,B			; Check the register
			CP      7	
			SCF
			RET     NZ			; If it is not A, then return
;
			LD      A,C			; Bind the register with the operand
			XOR     3
			RLCA
			RLCA
			RLCA
			CALL    BYTE_			; Write out the operand
			JR      VAL8			; And the value
;
; GROUP 11 - JR, DJNZ
;
GROUP11:		SUB     2			; The number of opcodes in GROUP11
			JR      NC,GROUP12
			CP      1-2
			CALL    NZ,COND_
			LD      A,C
			JR      NC,@F
			LD      A,18H
@@:			CALL    BYTE_
			CALL    NUMBER
			LD      DE,(PC)
			INC     DE
			SCF
			SBC     HL,DE
			LD      A,L
			RLA
			SBC     A,A
			CP      H
TOOFAR:			LD      A,1
			JP      NZ,ERROR_		; Throw an "Out of range" error
VAL8:			LD      A,L
			JP      BYTE_
;
; GROUP 12 - JP
;
GROUP12:		SUB	1			; The number of opcodes in GROUP12
			JR	NC,GROUP13
			CALL	EZ80SF_PART		; Evaluate the suffix (just LIL and SIS)
			RET	C			; Exit if an invalid suffix is provided
			CALL    COND_			; Evaluate the conditions
			LD      A,C
			JR      NC,GROUP12_1
			LD      A,B
			AND     3FH
			CP      6
			LD      A,0E9H
			JP      Z,BYTE_
			LD      A,0C3H
GROUP12_1:		CALL    BYTE_			; Output the opcode (with conditions)
			JP	ADDR_			; Output the address
;
; GROUP 13 - CALL
;
GROUP13:		SUB	1			; The number of opcodes in GROUP13
			JR	NC,GROUP14
GROUP13_1:		CALL	EZ80SF_FULL		; Evaluate the suffix
			CALL    GROUP15_1		; Output the opcode (with conditions)
			JP	ADDR_			; Output the address
;
; GROUP 14 - RST
;
GROUP14:		SUB	1			; The number of opcodes in GROUP14
			JR	NC,GROUP15
			CALL	EZ80SF_FULL		; Evaluate the suffix
			RET	C			; Exit if an invalid suffix provided		
			CALL    NUMBER
			AND     C
			OR      H
			JR      NZ,TOOFAR
			LD      A,L
			OR      C
	  		JP      BYTE_
;
; GROUP 15 - RET
;
GROUP15:		SUB	1			; The number of opcodes in GROUP15
			JR	NC,GROUP16
GROUP15_1:		CALL    COND_
			LD      A,C
			JP      NC,BYTE_
			OR      9
			JP      BYTE_
;
; GROUP 16 - LD
;
GROUP16:		SUB	1			; The number of opcodes in GROUP16
			JR	NC,GROUP17
			CALL	EZ80SF_FULL		; Evaluate the suffix
			CALL    LDOP			; Check for accumulator loads	
			JP      NC,LDA			; Yes, so jump here
			CALL    REGHI
			EX      AF,AF'
			CALL    SKIP
			CP      '('			; Check for bracket
			JR      Z,LDIN			; Yes, so we're doing an indirect load from memory
			EX      AF,AF'
			JP      NC,GROUP05_1		; Load single register direct; go here
			LD      C,1
			CALL    PAIR1_EX
			RET     C
			LD      A,14
			CP      B
			LD      B,A
			CALL    Z,PAIR_EX
			LD      A,B
			AND     3FH
			CP      12
			LD      A,C
			JP      NZ,GROUP12_1		; Load register pair direct; go here
			LD      A,0F9H
			JP      BYTE_
;
LDIN:			EX      AF,AF'
			PUSH    BC
			CALL    NC,REGLO
			LD      A,C
			POP     BC
			JP      NC,BIND
			LD      C,0AH
			CALL    PAIR1_EX
			CALL    LD16
			JP      NC,GROUP12_1
			CALL    NUMBER
			LD      C,2
			CALL    PAIR_EX
			CALL    LD16
			RET     C
			CALL    BYTE_
			BIT	7,D			; Check the ADL flag
			JP	NZ,VAL24 		; If it is set, then use 24-bit addresses			
			JP      VAL16			; Otherwise use 16-bit addresses
;
; Group 17 - TST
;
GROUP17:		SUB	1			; The number of opcodes in GROUP17
			JR	NC,OPTS
			CALL	ED			; Needs to be prefixed with ED
			CALL	REG			; Fetch the register
			JR	NC,GROUP17_1		; It's just a register
;
			LD	A,64H			; Opcode for TST n
			CALL	BYTE_			; Write out the opcode
			CALL	NUMBER			; Get the number
			JP	VAL8			; And write that out
;
GROUP17_1:		LD	A,B			; Check the register rangs
			CP	8
			CCF
			RET	C			; Ret with carry flag set for error if out of range
			RLCA				; Get the opcode value
			RLCA
			RLCA
			ADD	A,C			; Add the opcode base in
			JP	BYTE_

;
; Assembler directives - OPT, ADL
;
OPTS:			SUB	2
			JR	NC, DEFS
			CP	1-2			; Check for ADL opcode
			JR	Z, ADL_
;
OPT:			CALL    NUMBER			; Fetch the OPT value
			LD      HL,LISTON		; Address of the LISTON/OPT flag
			AND	7			; Only interested in the first three bits
			LD      C,A			; Store the new OPT value in C
			RLD				; Shift the top nibble of LISTON (OPT) into A
			AND	8			; Clear the bottom three bits, preserving the ADL bit
			OR	C			; OR in the new value
			RRD				; And shift the nibble back in
			RET
;
ADL_:			CALL	NUMBER			; Fetch the ADL value
			AND	1			; Only interested if it is 0 or 1
			RRCA				; Rotate to bit 7
			LD	C,A			; Store in C
			LD	A,(LISTON)		; Get the LISTON system variable
			AND	7Fh			; Clear bit 7
			OR	C			; OR in the ADL value
			LD	(LISTON),A		; Store
			RET			
;
; DEFB, DEFW, DEFL, DEFM
;
DEFS:			OR	A			; Handle DEFB
			JP	Z, DB_	
			DEC	A			; Handle DEFW
			JP	Z, ADDR16
			DEC	A			; Handle DEFL
			JP	Z, ADDR24
;
			PUSH    IX			; Handle DEFM
			CALL    EXPRS
			POP     IX
			LD      HL,ACCS
@@:			XOR     A
			CP      E
			RET     Z
			LD      A,(HL)
			INC     HL
			CALL    BYTE_
			DEC     E
			JR      @B
			
;
;SUBROUTINES:
;
EZ80SF_PART:		LD	A,(IY)			; Check for a dot
			CP	'.'
			JR	Z, @F			; If present, then carry on processing the eZ80 suffix
			OR	A			; Reset the carry flag (no error)
			RET				; And return
@@:			INC	IY			; Skip the dot
			PUSH	BC			; Push the operand
			LD	HL,EZ80SFS_2		; Check the shorter fully qualified table (just LIL and SIS)
			CALL	FIND			; Look up the operand
			JR	NC,EZ80SF_OK
			POP	BC			; Not found at this point, so will return with a C (error)
			RET
;			
EZ80SF_FULL:		LD	A,(IY)			; Check for a dot
			CP	'.'
			JR	Z,@F			; If present, then carry on processing the eZ80 suffix
			OR	A			; Reset the carry flag (no error)
			RET				; And return
@@:			INC	IY 			; Skip the dot
			PUSH	BC			; Push the operand
			LD	HL,EZ80SFS_1		; First check the fully qualified table
			CALL	FIND 			; Look up the operand
			JR	NC,EZ80SF_OK		; Yes, we've found it, so go write it out
			CALL	EZ80SF_TABLE		; Get the correct shortcut table in HL based upon the ADL mode
			CALL	FIND
			JR	NC,EZ80SF_OK
			POP	BC			; Not found at this point, so will return with a C (error)
			RET
;
EZ80SF_OK:		LD	A,B			; The operand value
			CALL	NC,BYTE_ 		; Write it out if found
			RES	7,D			; Clear the default ADL mode from the flags
			AND	2			; Check the second half of the suffix (.xxL)
			RRCA				; Shift into bit 7
			RRCA
			OR	D			; Or into bit 7 of D
			LD	D,A
			POP	BC 			; Restore the operand
			RET
;
EZ80SF_TABLE:		LD	HL,EZ80SFS_ADL0		; Return with the ADL0 lookup table
			BIT 	7,D			; if bit 7 of D is 0
			RET	Z
			LD	HL,EZ80SFS_ADL1		; Otherwise return with the ADL1 lookup table
			RET 
;
ADDR_:			BIT	7,D			; Check the ADL flag
			JR	NZ,ADDR24 		; If it is set, then use 24-bit addresses
;
ADDR16:			CALL	NUMBER			; Fetch an address (16-bit) and fall through to VAL16
VAL16:			CALL    VAL8			; Write out a 16-bit value (HL)
			LD      A,H
			JP      BYTE_
;
ADDR24:			CALL    NUMBER			; Fetch an address (24-bit) and fall through to VAL24
VAL24:			CALL	VAL16			; Lower 16-bits are in HL
			EXX
			LD	A,L			; Upper 16-bits are in HL', just need L' to make up 24-bit value
			EXX
			JP	BYTE_
;
LDA:			CP      4
			CALL    C,ED
			LD      A,B
			JP      BYTE_
;
LD16:			LD      A,B
			JR      C,LD8
			LD      A,B
			AND     3FH
			CP      12
			LD      A,C
			RET     Z
			CALL    ED
			LD      A,C
			OR      43H
			RET
;
LD8:			CP      7
			SCF
			RET     NZ
			LD      A,C
			OR      30H
			RET
;
; Used in IN and OUT to handle whether the operand is C or a number
;
CORN:			PUSH    BC
			CALL    OPND			; Get the operand
			BIT     5,B			
			POP     BC
			JR      Z,NUMBER		; If bit 5 is clear, then it's IN A,(N) or OUT (N),A, so fetch the port number
			LD      H,-1			; At this point it's IN r,(C) or OUT (C),r, so flag by setting H to &FF
;
ED:			LD      A,0EDH			; Write an ED prefix out
			JR      BYTE_
;
CB:			LD      A,0CBH
BIND:			CP      76H
			SCF
			RET     Z               	; Reject LD (HL),(HL)
			CALL    BYTE_
			BIT	6,D			; Check the index bit in flags
			RET     Z	
			LD      A,E			; If there is an index, output the offset
			JR      BYTE_
;
; Search through the operand table
; Returns:
; - B: The operand type
; - D: Bit 7: 0 = no prefix, 1 = prefix
; - E: The IX/IY offset
; - F: Carry if not found
;
OPND:			PUSH    HL			; Preserve HL
			LD      HL,OPRNDS		; The operands table
			CALL    FIND			; Find the operand
			POP     HL
			RET     C			; Return if not found
			BIT     7,B			; Check if it is an index register (IX, IY)
			RET     Z			; Return if it isn't
			SET	6,D			; Set flag to indicate we've got an index
			BIT     3,B			; Check if an offset is required
			PUSH    HL
			CALL    Z,OFFSET_EX		; If bit 3 of B is zero, then get the offset
			LD      E,L			; E: The offset
			POP     HL
			LD	A,DDH			; IX prefix
			BIT     6,B			; If bit 6 is reset then
			JR      Z,BYTE_			; It's an IX instruction, otherwise set
			LD	A,FDH			; IY prefix
;
BYTE_:			LD      (IX),A			; Write a byte out
			INC     IX
			OR      A
			RET
;
OFFSET_EX:			LD      A,(IY)
			CP      ')'
			LD      HL,0
			RET     Z
NUMBER:			CALL    SKIP
			PUSH    BC
			PUSH    DE
			PUSH    IX
			CALL    EXPRI
			POP     IX
			EXX
			POP     DE
			POP     BC
			LD      A,L
			OR      A
			RET
;
REG:			CALL    OPND
			RET     C
			LD      A,B
			AND     3FH
			CP      8
			CCF
			RET
;
REGLO:			CALL    REG
			RET     C
			JR      ORC
;
REGHI:			CALL    REG
			RET     C
			JR      SHL3
;
COND_:			CALL    OPND
			RET     C
			LD      A,B
			AND     1FH
			SUB     16
			JR      NC,SHL3
			CP      -15
			SCF
			RET     NZ
			LD      A,3
			JR      SHL3
;
PAIR_EX:			CALL    OPND
			RET     C
PAIR1_EX:			LD      A,B
			AND     0FH
			SUB     8
			RET     C
			JR      SHL3
;
BIT_:			CALL    NUMBER
			CP      8
			CCF
			RET     C
SHL3:			RLCA
			RLCA
			RLCA
ORC:			OR      C
			LD      C,A
			RET
;
LDOP:			LD      HL,LDOPS

;
; Look up a value in a table
; Parameters:
; - IY: Address of the assembly language line in the BASIC program area
; - HL: Address of the table
; Returns:
; - B: The operand code
; - F: Carry set if not found
;
FIND:			CALL    SKIP			; Skip delimiters
;
EXIT_:			LD      B,0			; Set B to 0
			SCF				; Set the carry flag
			RET     Z			; Returns if Z
;
			CP      DEF_			; Special case for token DEF (used in DEFB, DEFW, DEFL, DEFM)
			JR      Z,FIND0
			CP      TOR+1			; Special case for tokens AND and OR
			CCF
			RET     C
FIND0:			LD      A,(HL)			; Check for the end of the table (0 byte marker)
			OR      A		
			JR      Z,EXIT_			; Exit
			XOR     (IY)
			AND     01011111B
			JR      Z,FIND2
FIND1:			BIT     7,(HL)
			INC     HL
			JR      Z,FIND1
			INC     HL
			INC     B
			JR      FIND0
;
FIND2:			PUSH    IY
FIND3:			BIT     7,(HL)			; Is this the end of token marker?
			INC     IY
			INC     HL
			JR      NZ,FIND5		; Yes
			CP      (HL)			
			CALL    Z,SKIP0
			LD      A,(HL)
			XOR     (IY)
			AND     01011111B
			JR      Z,FIND3
FIND4:			POP     IY
			JR      FIND1
;
FIND5:			CALL    DELIM			; Is it a delimiter?
			CALL	NZ,DOT 			; No, so also check whether it is a dot character (for suffixes)
			CALL    NZ,SIGN			; No, so also check whether it is a SIGN character ('+' or '-')
			JR      NZ,FIND4		; If it is not a sign or a delimiter, then loop
;
FIND6:			LD      A,B			; At this point we have a token
			LD      B,(HL)			; Fetch the token type code
			POP     HL			; Restore the stack
			RET
;
SKIP0:			INC     HL
SKIP:			CALL    DELIM			; Is it a delimiter?
			RET     NZ			; No, so return
			CALL    TERM			; Is it a terminator?
			RET     Z			; Yes, so return
			INC     IY			; Increment the basic program counter
			JR      SKIP			; And loop
;
SIGN:			CP      '+'			; Check whether the character is a sign symbol
			RET     Z
			CP      '-'
			RET
;
DOT:			CP	'.'			; Check if it is a dot character
			RET 
; Z80 opcode list
;
; Group 0: (15 opcodes)
; Trivial cases requiring no computation
;
; BEGIN REFACTOR FROM BINARY
; OPCODS:			
; 			DB	"NO","P"+80H,00h	; # 00h
; 			DB	"RLC","A"+80H,07h
; 			DB	"EX",0,"AF",0,"AF","'"+80H,08h
; 			DB	"RRC","A"+80H,0FH
; 			DB	"RL","A"+80H,17H
; 			DB	"RR","A"+80H,1FH
; 			DB	"DA","A"+80H,27H
; 			DB	"CP","L"+80H,2FH
; 			DB	"SC","F"+80H,37H
; 			DB	"CC","F"+80H,3FH
; 			DB	"HAL","T"+80H,76H
; 			DB	"EX","X"+80H,D9H
; 			DB	"EX",0,"DE",0,"H","L"+80H,EBH
; 			DB	"D","I"+80H,F3H
; 			DB	"E","I"+80H,FBH
; ;
; ; Group 1: (53 opcodes)
; ; As Group 0, but with an ED prefix
; ;
; 			DB	"NE","G"+80H,44H	; 0Fh
; 			DB	"IM",0,"0"+80H,46H
; 			DB	"RET","N"+80H,45H
; 			DB	"MLT",0,"B","C"+80H,4CH
; 			DB	"RET","I"+80H,4DH
; 			DB	"IM",0,"1"+80H,56H
; 			DB	"MLT",0,"D","E"+80H,5CH						
; 			DB	"IM",0,"2"+80H,5EH
; 			DB	"RR","D"+80H,67H
; 			DB	"MLT",0,"H","L"+80H,6CH
; 			DB	"LD",0,"MB",0,"A"+80H,6DH
; 			DB	"LD",0,"A",0,"M","B"+80H,6EH
; 			DB	"RL","D"+80H,6FH
; 			DB	"SL","P"+80H,76H
; 			DB	"MLT",0,"S","P"+80H,7CH
; 			DB	"STMI","X"+80H,7DH
; 			DB	"RSMI","X"+80H,7EH
; 			DB	"INI","M"+80H,82H
; 			DB	"OTI","M"+80H,83H
; 			DB	"INI","2"+80H,84H
; 			DB	"IND","M"+80H,8AH
; 			DB	"OTD","M"+80H,8BH
; 			DB	"IND","2"+80H,8CH
; 			DB	"INIM","R"+80H,92H
; 			DB	"OTIM","R"+80H,93H
; 			DB	"INI2","R"+80H,94H
; 			DB	"INDM","R"+80H,9AH
; 			DB	"OTDM","R"+80H,9BH
; 			DB	"IND2","R"+80H,9CH
; 			DB	"LD","I"+80H,A0H
; 			DB	"CP","I"+80H,A1H
; 			DB	"IN","I"+80H,A2H
; 			DB	"OUTI","2"+80H,A4H	; These are swapped round so that FIND will find
; 			DB	"OUT","I"+80H,A3H	; OUTI2 before OUTI
; 			DB	"LD","D"+80H,A8H
; 			DB	"CP","D"+80H,A9H
; 			DB	"IN","D"+80H,AAH
; 			DB	"OUTD","2"+80H,ACH	; Similarly these are swapped round so that FIND
; 			DB	"OUT","D"+80H,ABH	; will find OUTD2 before OUTD
; 			DB	"LDI","R"+80H,B0H
; 			DB	"CPI","R"+80H,B1H
; 			DB	"INI","R"+80H,B2H
; 			DB	"OTI","R"+80H,B3H
; 			DB	"OTI2","R"+80H,B4H
; 			DB	"LDD","R"+80H,B8H
; 			DB	"CPD","R"+80H,B9H
; 			DB	"IND","R"+80H,BAH
; 			DB	"OTD","R"+80H,BBH
; 			DB	"OTD2","R"+80H,BCH
; 			DB	"INIR","X"+80H,C2H
; 			DB	"OTIR","X"+80H,C3H
; 			DB	"INDR","X"+80H,CAH
; 			DB	"OTDR","X"+80H,CBH
; ;
; ; Group 2: (3 opcodes)
; ;
; 			DB	"BI","T"+80H,40H	; 44h
; 			DB	"RE","S"+80H,80H
; 			DB	"SE","T"+80H,C0H
; ;
; ; Group 3: (7 opcodes)
; ;
; 			DB	"RL","C"+80H,00H	; 47h
; 			DB	"RR","C"+80H,08H
; 			DB	"R","L"+80H,10H
; 			DB	"R","R"+80H,18H
; 			DB	"SL","A"+80H,20H
; 			DB	"SR","A"+80H,28H
; 			DB	"SR","L"+80H,38H
; ;
; ; Group 4: (3 opcodes)
; ;
; 			DB	"PO","P"+80H,C1H	; 4Eh
; 			DB	"PUS","H"+80H,C5H
; 			DB	"EX",0,"(S","P"+80H,E3H
; ;
; ; Group 5: (7 opcodes)
; ;
; 			DB	"SU","B"+80H,90H	; 51h
; 			DB	"AN","D"+80H,A0H
; 			DB	"XO","R"+80H,A8H
; 			DB	"O","R"+80H,B0H
; 			DB	"C","P"+80H,B8H
; 			DB	TAND,A0H		; 56h TAND: Tokenised AND
; 			DB	TOR,B0H			; 57h TOR: Tokenised OR
; ;
; ; Group 6 (3 opcodes)
; ;
; 			DB	"AD","D"+80H,80H	; 58h
; 			DB	"AD","C"+80H,88H
; 			DB	"SB","C"+80H,98H
; ;
; ; Group 7: (2 opcodes)
; ;
; 			DB	"IN","C"+80H,04H	; 5Bh
; 			DB	"DE","C"+80H,05H
; ;
; ; Group 8: (2 opcodes)
; ;
; 			DB	"IN","0"+80H,00H	; 5Dh
; 			DB	"OUT","0"+80H,01H
; ;
; ; Group 9: (1 opcode)
; ;
; 			DB	"I","N"+80H,40H		; 5Fh
; ;
; ; Group 10: (1 opcode)
; ;
; 			DB	"OU","T"+80H,41H	; 60h
; ;
; ; Group 11: (2 opcodes)
; ;
; 			DB	"J","R"+80H,20H		; 61h
; 			DB	"DJN","Z"+80H,10H
; ;
; ; Group 12: (1 opcode)
; ;
; 			DB	"J","P"+80H,C2H		; 63h
; ;
; ; Group 13: (1 opcode)
; ;
; 			DB	"CAL","L"+80H,C4H	; 64h
; ;
; ; Group 14: (1 opcode)
; ;
; 			DB	"RS","T"+80H,C7H	; 65h
; ;
; ; Group 15: (1 opcode)
; ;
; 			DB	"RE","T"+80H,C0H	; 66h
; ;
; ; Group 16: (1 opcode)
; ;
; 			DB	"L","D"+80H,40H		; 67h
; ;
; ; Group 17: (1 opcode)
; ;
; 			DB	"TS","T"+80H,04H	; 68h

; ;
; ; Assembler Directives
; ;
; 			DB	"OP","T"+80H,00H	; 69h OPT
; 			DB	"AD","L"+80H,00H	; 6Ah ADL
; ;
; 			DB	DEF_ & 7FH,"B"+80H,00H	; 6Bh Tokenised DEF + B
; 			DB	DEF_ & 7FH,"W"+80H,00H	; 6Ch Tokenised DEF + W
; 			DB	DEF_ & 7FH,"L"+80H,00H	; 6Dh Tokenised DEF + L
; 			DB 	DEF_ & 7FH,"M"+80H,00H	; 6Eh Tokenised DEF + M
; ;
; 			DB	0
; ;			
; ; Operands
; ;
; OPRNDS:			DB	"B"+80H, 00H
; 			DB	"C"+80H, 01H
; 			DB	"D"+80H, 02H
; 			DB	"E"+80H, 03H
; 			DB	"H"+80H, 04H
; 			DB	"L"+80H, 05H
; 			DB	"(H","L"+80H,06H
; 			DB	"A"+80H, 07H
; 			DB	"(I","X"+80H,86H
; 			DB	"(I","Y"+80H,C6H
; ;
; 			DB	"B","C"+80H,08H
; 			DB	"D","E"+80H,0AH
; 			DB	"H","L"+80H,0CH
; 			DB	"I","X"+80H,8CH
; 			DB	"I","Y"+80H,CCH
; 			DB	"A","F"+80H,0EH
; 			DB	"S","P"+80H,0EH
; ;
; 			DB	"N","Z"+80H,10H
; 			DB	"Z"+80H,11H
; 			DB	"N","C"+80H,12H
; 			DB	"P","O"+80H,14H
; 			DB	"P","E"+80H,15H
; 			DB	"P"+80H,16H
; 			DB	"M"+80H,17H
; ;
; 			DB	"(","C"+80H,20H
; ;
; 			DB	0
; ;
; ; Load operations
; ;
; LDOPS:			DB	"I",0,"A"+80H,47H
; 			DB	"R",0,"A"+80H,4FH
; 			DB	"A",0,"I"+80H,57H
; 			DB	"A",0,"R"+80H,5FH
; 			DB	"(BC",0,"A"+80H,02h
; 			DB	"(DE",0,"A"+80H,12H
; 			DB	"A",0,"(B","C"+80H,0AH
; 			DB	"A",0,"(D","E"+80H,1AH
; ;
; 			DB	0
; ;
; ; eZ80 addressing mode suffixes
; ;
; ; Fully qualified suffixes
; ;
; EZ80SFS_1:		DB	"LI","S"+80H,49H
; 			DB	"SI","L"+80H,52H
; EZ80SFS_2:		DB	"SI","S"+80H,40H
; 			DB	"LI","L"+80H,5BH
; ;
; 			DB	0
; ;
; ; Shortcuts when ADL mode is 0
; ;
; EZ80SFS_ADL0:		DB	"S"+80H,40H		; Equivalent to .SIS
; 			DB	"L"+80H,49H		; Equivalent to .LIS
; 			DB	"I","S"+80H,40H		; Equivalent to .SIS
; 			DB	"I","L"+80H,52H		; Equivalent to .SIL
; ;
; 			DB	0
; ;
; ; Shortcuts when ADL mode is 1
; ;
; EZ80SFS_ADL1:		DB	"S"+80H,52H		; Equivalent to .SIL
; 			DB	"L"+80H,5BH		; Equivalent to .LIL
; 			DB	"I","S"+80H,49H		; Equivalent to .LIS
; 			DB	"I","L"+80H,5BH		; Equivalent to .LIL
; ;
; 			DB	0
; END REFACTOR FROM BINARY 
; BEGIN INSERT FROM BINARY
;
; Trivial cases requiring no computation
OPCODS:			
	db 0x4e ; 041DCC 4E      5258 DB	"NO","P"+80H,00h	; # 00h
	db 0x4f ; 041DCD
	db 0xd0 ; 041DCE
	db 0x00 ; 041DCF
	db 0x52 ; 041DD0 52      5259 DB	"RLC","A"+80H,07h
	db 0x4c ; 041DD1
	db 0x43 ; 041DD2
	db 0xc1 ; 041DD3
	db 0x07 ; 041DD4
	db 0x45 ; 041DD5 45      5260 DB	"EX",0,"AF",0,"AF","'"+80H,08h
	db 0x58 ; 041DD6
	db 0x00 ; 041DD7
	db 0x41 ; 041DD8
	db 0x46 ; 041DD9
	db 0x00 ; 041DDA
	db 0x41 ; 041DDB
	db 0x46 ; 041DDC
	db 0xa7 ; 041DDD
	db 0x08 ; 041DDE
	db 0x52 ; 041DDF 52      5261 DB	"RRC","A"+80H,0FH
	db 0x52 ; 041DE0
	db 0x43 ; 041DE1
	db 0xc1 ; 041DE2
	db 0x0f ; 041DE3
	db 0x52 ; 041DE4 52      5262 DB	"RL","A"+80H,17H
	db 0x4c ; 041DE5
	db 0xc1 ; 041DE6
	db 0x17 ; 041DE7
	db 0x52 ; 041DE8 52      5263 DB	"RR","A"+80H,1FH
	db 0x52 ; 041DE9
	db 0xc1 ; 041DEA
	db 0x1f ; 041DEB
	db 0x44 ; 041DEC 44      5264 DB	"DA","A"+80H,27H
	db 0x41 ; 041DED
	db 0xc1 ; 041DEE
	db 0x27 ; 041DEF
	db 0x43 ; 041DF0 43      5265 DB	"CP","L"+80H,2FH
	db 0x50 ; 041DF1
	db 0xcc ; 041DF2
	db 0x2f ; 041DF3
	db 0x53 ; 041DF4 53      5266 DB	"SC","F"+80H,37H
	db 0x43 ; 041DF5
	db 0xc6 ; 041DF6
	db 0x37 ; 041DF7
	db 0x43 ; 041DF8 43      5267 DB	"CC","F"+80H,3FH
	db 0x43 ; 041DF9
	db 0xc6 ; 041DFA
	db 0x3f ; 041DFB
	db 0x48 ; 041DFC 48      5268 DB	"HAL","T"+80H,76H
	db 0x41 ; 041DFD
	db 0x4c ; 041DFE
	db 0xd4 ; 041DFF
	db 0x76 ; 041E00
	db 0x45 ; 041E01 45      5269 DB	"EX","X"+80H,D9H
	db 0x58 ; 041E02
	db 0xd8 ; 041E03
	db 0xd9 ; 041E04
	db 0x45 ; 041E05 45      5270 DB	"EX",0,"DE",0,"H","L"+80H,EBH
	db 0x58 ; 041E06
	db 0x00 ; 041E07
	db 0x44 ; 041E08
	db 0x45 ; 041E09
	db 0x00 ; 041E0A
	db 0x48 ; 041E0B
	db 0xcc ; 041E0C
	db 0xeb ; 041E0D
	db 0x44 ; 041E0E 44      5271 DB	"D","I"+80H,F3H
	db 0xc9 ; 041E0F
	db 0xf3 ; 041E10
	db 0x45 ; 041E11 45      5272 DB	"E","I"+80H,FBH
	db 0xc9 ; 041E12
	db 0xfb ; 041E13
;
; Group 1: (53 opcodes)
; As Group 0, but with an ED prefix
;
	db 0x4e ; 041E14 4E      5277 DB	"NE","G"+80H,44H	; 0Fh
	db 0x45 ; 041E15
	db 0xc7 ; 041E16
	db 0x44 ; 041E17
	db 0x49 ; 041E18 49      5278 DB	"IM",0,"0"+80H,46H
	db 0x4d ; 041E19
	db 0x00 ; 041E1A
	db 0xb0 ; 041E1B
	db 0x46 ; 041E1C
	db 0x52 ; 041E1D 52      5279 DB	"RET","N"+80H,45H
	db 0x45 ; 041E1E
	db 0x54 ; 041E1F
	db 0xce ; 041E20
	db 0x45 ; 041E21
	db 0x4d ; 041E22 4D      5280 DB	"MLT",0,"B","C"+80H,4CH
	db 0x4c ; 041E23
	db 0x54 ; 041E24
	db 0x00 ; 041E25
	db 0x42 ; 041E26
	db 0xc3 ; 041E27
	db 0x4c ; 041E28
	db 0x52 ; 041E29 52      5281 DB	"RET","I"+80H,4DH
	db 0x45 ; 041E2A
	db 0x54 ; 041E2B
	db 0xc9 ; 041E2C
	db 0x4d ; 041E2D
	db 0x49 ; 041E2E 49      5282 DB	"IM",0,"1"+80H,56H
	db 0x4d ; 041E2F
	db 0x00 ; 041E30
	db 0xb1 ; 041E31
	db 0x56 ; 041E32
	db 0x4d ; 041E33 4D      5283 DB	"MLT",0,"D","E"+80H,5CH
	db 0x4c ; 041E34
	db 0x54 ; 041E35
	db 0x00 ; 041E36
	db 0x44 ; 041E37
	db 0xc5 ; 041E38
	db 0x5c ; 041E39
	db 0x49 ; 041E3A 49      5284 DB	"IM",0,"2"+80H,5EH
	db 0x4d ; 041E3B
	db 0x00 ; 041E3C
	db 0xb2 ; 041E3D
	db 0x5e ; 041E3E
	db 0x52 ; 041E3F 52      5285 DB	"RR","D"+80H,67H
	db 0x52 ; 041E40
	db 0xc4 ; 041E41
	db 0x67 ; 041E42
	db 0x4d ; 041E43 4D      5286 DB	"MLT",0,"H","L"+80H,6CH
	db 0x4c ; 041E44
	db 0x54 ; 041E45
	db 0x00 ; 041E46
	db 0x48 ; 041E47
	db 0xcc ; 041E48
	db 0x6c ; 041E49
	db 0x4c ; 041E4A 4C      5287 DB	"LD",0,"MB",0,"A"+80H,6DH
	db 0x44 ; 041E4B
	db 0x00 ; 041E4C
	db 0x4d ; 041E4D
	db 0x42 ; 041E4E
	db 0x00 ; 041E4F
	db 0xc1 ; 041E50
	db 0x6d ; 041E51
	db 0x4c ; 041E52 4C      5288 DB	"LD",0,"A",0,"M","B"+80H,6EH
	db 0x44 ; 041E53
	db 0x00 ; 041E54
	db 0x41 ; 041E55
	db 0x00 ; 041E56
	db 0x4d ; 041E57
	db 0xc2 ; 041E58
	db 0x6e ; 041E59
	db 0x52 ; 041E5A 52      5289 DB	"RL","D"+80H,6FH
	db 0x4c ; 041E5B
	db 0xc4 ; 041E5C
	db 0x6f ; 041E5D
	db 0x53 ; 041E5E 53      5290 DB	"SL","P"+80H,76H
	db 0x4c ; 041E5F
	db 0xd0 ; 041E60
	db 0x76 ; 041E61
	db 0x4d ; 041E62 4D      5291 DB	"MLT",0,"S","P"+80H,7CH
	db 0x4c ; 041E63
	db 0x54 ; 041E64
	db 0x00 ; 041E65
	db 0x53 ; 041E66
	db 0xd0 ; 041E67
	db 0x7c ; 041E68
	db 0x53 ; 041E69 53      5292 DB	"STMI","X"+80H,7DH
	db 0x54 ; 041E6A
	db 0x4d ; 041E6B
	db 0x49 ; 041E6C
	db 0xd8 ; 041E6D
	db 0x7d ; 041E6E
	db 0x52 ; 041E6F 52      5293 DB	"RSMI","X"+80H,7EH
	db 0x53 ; 041E70
	db 0x4d ; 041E71
	db 0x49 ; 041E72
	db 0xd8 ; 041E73
	db 0x7e ; 041E74
	db 0x49 ; 041E75 49      5294 DB	"INI","M"+80H,82H
	db 0x4e ; 041E76
	db 0x49 ; 041E77
	db 0xcd ; 041E78
	db 0x82 ; 041E79
	db 0x4f ; 041E7A 4F      5295 DB	"OTI","M"+80H,83H
	db 0x54 ; 041E7B
	db 0x49 ; 041E7C
	db 0xcd ; 041E7D
	db 0x83 ; 041E7E
	db 0x49 ; 041E7F 49      5296 DB	"INI","2"+80H,84H
	db 0x4e ; 041E80
	db 0x49 ; 041E81
	db 0xb2 ; 041E82
	db 0x84 ; 041E83
	db 0x49 ; 041E84 49      5297 DB	"IND","M"+80H,8AH
	db 0x4e ; 041E85
	db 0x44 ; 041E86
	db 0xcd ; 041E87
	db 0x8a ; 041E88
	db 0x4f ; 041E89 4F      5298 DB	"OTD","M"+80H,8BH
	db 0x54 ; 041E8A
	db 0x44 ; 041E8B
	db 0xcd ; 041E8C
	db 0x8b ; 041E8D
	db 0x49 ; 041E8E 49      5299 DB	"IND","2"+80H,8CH
	db 0x4e ; 041E8F
	db 0x44 ; 041E90
	db 0xb2 ; 041E91
	db 0x8c ; 041E92
	db 0x49 ; 041E93 49      5300 DB	"INIM","R"+80H,92H
	db 0x4e ; 041E94
	db 0x49 ; 041E95
	db 0x4d ; 041E96
	db 0xd2 ; 041E97
	db 0x92 ; 041E98
	db 0x4f ; 041E99 4F      5301 DB	"OTIM","R"+80H,93H
	db 0x54 ; 041E9A
	db 0x49 ; 041E9B
	db 0x4d ; 041E9C
	db 0xd2 ; 041E9D
	db 0x93 ; 041E9E
	db 0x49 ; 041E9F 49      5302 DB	"INI2","R"+80H,94H
	db 0x4e ; 041EA0
	db 0x49 ; 041EA1
	db 0x32 ; 041EA2
	db 0xd2 ; 041EA3
	db 0x94 ; 041EA4
	db 0x49 ; 041EA5 49      5303 DB	"INDM","R"+80H,9AH
	db 0x4e ; 041EA6
	db 0x44 ; 041EA7
	db 0x4d ; 041EA8
	db 0xd2 ; 041EA9
	db 0x9a ; 041EAA
	db 0x4f ; 041EAB 4F      5304 DB	"OTDM","R"+80H,9BH
	db 0x54 ; 041EAC
	db 0x44 ; 041EAD
	db 0x4d ; 041EAE
	db 0xd2 ; 041EAF
	db 0x9b ; 041EB0
	db 0x49 ; 041EB1 49      5305 DB	"IND2","R"+80H,9CH
	db 0x4e ; 041EB2
	db 0x44 ; 041EB3
	db 0x32 ; 041EB4
	db 0xd2 ; 041EB5
	db 0x9c ; 041EB6
	db 0x4c ; 041EB7 4C      5306 DB	"LD","I"+80H,A0H
	db 0x44 ; 041EB8
	db 0xc9 ; 041EB9
	db 0xa0 ; 041EBA
	db 0x43 ; 041EBB 43      5307 DB	"CP","I"+80H,A1H
	db 0x50 ; 041EBC
	db 0xc9 ; 041EBD
	db 0xa1 ; 041EBE
	db 0x49 ; 041EBF 49      5308 DB	"IN","I"+80H,A2H
	db 0x4e ; 041EC0
	db 0xc9 ; 041EC1
	db 0xa2 ; 041EC2
	db 0x4f ; 041EC3 4F      5309 DB	"OUTI","2"+80H,A4H	; These are swapped round so that FIND will find
	db 0x55 ; 041EC4
	db 0x54 ; 041EC5
	db 0x49 ; 041EC6
	db 0xb2 ; 041EC7
	db 0xa4 ; 041EC8
	db 0x4f ; 041EC9 4F      5310 DB	"OUT","I"+80H,A3H	; OUTI2 before OUTI
	db 0x55 ; 041ECA
	db 0x54 ; 041ECB
	db 0xc9 ; 041ECC
	db 0xa3 ; 041ECD
	db 0x4c ; 041ECE 4C      5311 DB	"LD","D"+80H,A8H
	db 0x44 ; 041ECF
	db 0xc4 ; 041ED0
	db 0xa8 ; 041ED1
	db 0x43 ; 041ED2 43      5312 DB	"CP","D"+80H,A9H
	db 0x50 ; 041ED3
	db 0xc4 ; 041ED4
	db 0xa9 ; 041ED5
	db 0x49 ; 041ED6 49      5313 DB	"IN","D"+80H,AAH
	db 0x4e ; 041ED7
	db 0xc4 ; 041ED8
	db 0xaa ; 041ED9
	db 0x4f ; 041EDA 4F      5314 DB	"OUTD","2"+80H,ACH	; Similarly these are swapped round so that FIND
	db 0x55 ; 041EDB
	db 0x54 ; 041EDC
	db 0x44 ; 041EDD
	db 0xb2 ; 041EDE
	db 0xac ; 041EDF
	db 0x4f ; 041EE0 4F      5315 DB	"OUT","D"+80H,ABH	; will find OUTD2 before OUTD
	db 0x55 ; 041EE1
	db 0x54 ; 041EE2
	db 0xc4 ; 041EE3
	db 0xab ; 041EE4
	db 0x4c ; 041EE5 4C      5316 DB	"LDI","R"+80H,B0H
	db 0x44 ; 041EE6
	db 0x49 ; 041EE7
	db 0xd2 ; 041EE8
	db 0xb0 ; 041EE9
	db 0x43 ; 041EEA 43      5317 DB	"CPI","R"+80H,B1H
	db 0x50 ; 041EEB
	db 0x49 ; 041EEC
	db 0xd2 ; 041EED
	db 0xb1 ; 041EEE
	db 0x49 ; 041EEF 49      5318 DB	"INI","R"+80H,B2H
	db 0x4e ; 041EF0
	db 0x49 ; 041EF1
	db 0xd2 ; 041EF2
	db 0xb2 ; 041EF3
	db 0x4f ; 041EF4 4F      5319 DB	"OTI","R"+80H,B3H
	db 0x54 ; 041EF5
	db 0x49 ; 041EF6
	db 0xd2 ; 041EF7
	db 0xb3 ; 041EF8
	db 0x4f ; 041EF9 4F      5320 DB	"OTI2","R"+80H,B4H
	db 0x54 ; 041EFA
	db 0x49 ; 041EFB
	db 0x32 ; 041EFC
	db 0xd2 ; 041EFD
	db 0xb4 ; 041EFE
	db 0x4c ; 041EFF 4C      5321 DB	"LDD","R"+80H,B8H
	db 0x44 ; 041F00
	db 0x44 ; 041F01
	db 0xd2 ; 041F02
	db 0xb8 ; 041F03
	db 0x43 ; 041F04 43      5322 DB	"CPD","R"+80H,B9H
	db 0x50 ; 041F05
	db 0x44 ; 041F06
	db 0xd2 ; 041F07
	db 0xb9 ; 041F08
	db 0x49 ; 041F09 49      5323 DB	"IND","R"+80H,BAH
	db 0x4e ; 041F0A
	db 0x44 ; 041F0B
	db 0xd2 ; 041F0C
	db 0xba ; 041F0D
	db 0x4f ; 041F0E 4F      5324 DB	"OTD","R"+80H,BBH
	db 0x54 ; 041F0F
	db 0x44 ; 041F10
	db 0xd2 ; 041F11
	db 0xbb ; 041F12
	db 0x4f ; 041F13 4F      5325 DB	"OTD2","R"+80H,BCH
	db 0x54 ; 041F14
	db 0x44 ; 041F15
	db 0x32 ; 041F16
	db 0xd2 ; 041F17
	db 0xbc ; 041F18
	db 0x49 ; 041F19 49      5326 DB	"INIR","X"+80H,C2H
	db 0x4e ; 041F1A
	db 0x49 ; 041F1B
	db 0x52 ; 041F1C
	db 0xd8 ; 041F1D
	db 0xc2 ; 041F1E
	db 0x4f ; 041F1F 4F      5327 DB	"OTIR","X"+80H,C3H
	db 0x54 ; 041F20
	db 0x49 ; 041F21
	db 0x52 ; 041F22
	db 0xd8 ; 041F23
	db 0xc3 ; 041F24
	db 0x49 ; 041F25 49      5328 DB	"INDR","X"+80H,CAH
	db 0x4e ; 041F26
	db 0x44 ; 041F27
	db 0x52 ; 041F28
	db 0xd8 ; 041F29
	db 0xca ; 041F2A
	db 0x4f ; 041F2B 4F      5329 DB	"OTDR","X"+80H,CBH
	db 0x54 ; 041F2C
	db 0x44 ; 041F2D
	db 0x52 ; 041F2E
	db 0xd8 ; 041F2F
	db 0xcb ; 041F30
;
; Group 2: (3 opcodes)
;
	db 0x42 ; 041F31 42      5333 DB	"BI","T"+80H,40H	; 44h
	db 0x49 ; 041F32
	db 0xd4 ; 041F33
	db 0x40 ; 041F34
	db 0x52 ; 041F35 52      5333 DB	"RE","S"+80H,80H
	db 0x45 ; 041F36
	db 0xd3 ; 041F37
	db 0x80 ; 041F38
	db 0x53 ; 041F39 53      5334 DB	"SE","T"+80H,C0H
	db 0x45 ; 041F3A
	db 0xd4 ; 041F3B
	db 0xc0 ; 041F3C
;
; Group 3: (7 opcodes)
;
	db 0x52 ; 041F3D 52      5338 DB	"RL","C"+80H,00H	; 47h
	db 0x4c ; 041F3E
	db 0xc3 ; 041F3F
	db 0x00 ; 041F40
	db 0x52 ; 041F41 52      5339 DB	"RR","C"+80H,08H
	db 0x52 ; 041F42
	db 0xc3 ; 041F43
	db 0x08 ; 041F44
	db 0x52 ; 041F45 52      5340 DB	"R","L"+80H,10H
	db 0xcc ; 041F46
	db 0x10 ; 041F47
	db 0x52 ; 041F48 52      5341 DB	"R","R"+80H,18H
	db 0xd2 ; 041F49
	db 0x18 ; 041F4A
	db 0x53 ; 041F4B 53      5342 DB	"SL","A"+80H,20H
	db 0x4c ; 041F4C
	db 0xc1 ; 041F4D
	db 0x20 ; 041F4E
	db 0x53 ; 041F4F 53      5343 DB	"SR","A"+80H,28H
	db 0x52 ; 041F50
	db 0xc1 ; 041F51
	db 0x28 ; 041F52
	db 0x53 ; 041F53 53      5344 DB	"SR","L"+80H,38H
	db 0x52 ; 041F54
	db 0xcc ; 041F55
	db 0x38 ; 041F56
;
; Group 4: (3 opcodes)
;
	db 0x50 ; 041F57 50      5348 DB	"PO","P"+80H,C1H	; 4Eh
	db 0x4f ; 041F58
	db 0xd0 ; 041F59
	db 0xc1 ; 041F5A
	db 0x50 ; 041F5B 50      5349 DB	"PUS","H"+80H,C5H
	db 0x55 ; 041F5C
	db 0x53 ; 041F5D
	db 0xc8 ; 041F5E
	db 0xc5 ; 041F5F
	db 0x45 ; 041F60 45      5350 DB	"EX",0,"(S","P"+80H,E3H
	db 0x58 ; 041F61
	db 0x00 ; 041F62
	db 0x28 ; 041F63
	db 0x53 ; 041F64
	db 0xd0 ; 041F65
	db 0xe3 ; 041F66
;
; Group 5: (7 opcodes)
;
	db 0x53 ; 041F67 53      5354 DB	"SU","B"+80H,90H	; 51h
	db 0x55 ; 041F68
	db 0xc2 ; 041F69
	db 0x90 ; 041F6A
	db 0x41 ; 041F6B 41      5355 DB	"AN","D"+80H,A0H
	db 0x4e ; 041F6C
	db 0xc4 ; 041F6D
	db 0xa0 ; 041F6E
	db 0x58 ; 041F6F 58      5356 DB	"XO","R"+80H,A8H
	db 0x4f ; 041F70
	db 0xd2 ; 041F71
	db 0xa8 ; 041F72
	db 0x4f ; 041F73 4F      5357 DB	"O","R"+80H,B0H
	db 0xd2 ; 041F74
	db 0xb0 ; 041F75
	db 0x43 ; 041F76 43      5358 DB	"C","P"+80H,B8H
	db 0xd0 ; 041F77
	db 0xb8 ; 041F78
	db 0x80 ; 041F79 80      5359 DB	TAND,A0H		; 56h TAND: Tokenised AND
	db 0xa0 ; 041F7A
	db 0x84 ; 041F7B 84      5360 DB	TOR,B0H			; 57h TOR: Tokenised OR
	db 0xb0 ; 041F7C
;
; Group 6 (3 opcodes)
;
	db 0x41 ; 041F7D 41      5364 DB	"AD","D"+80H,80H	; 58h
	db 0x44 ; 041F7E
	db 0xc4 ; 041F7F
	db 0x80 ; 041F80
	db 0x41 ; 041F81 41      5365 DB	"AD","C"+80H,88H
	db 0x44 ; 041F82
	db 0xc3 ; 041F83
	db 0x88 ; 041F84
	db 0x53 ; 041F85 53      5366 DB	"SB","C"+80H,98H
	db 0x42 ; 041F86
	db 0xc3 ; 041F87
	db 0x98 ; 041F88
;
; Group 7: (2 opcodes)
;
	db 0x49 ; 041F89 49      5370 DB	"IN","C"+80H,04H	; 5Bh
	db 0x4e ; 041F8A
	db 0xc3 ; 041F8B
	db 0x04 ; 041F8C
	db 0x44 ; 041F8D 44      5371 DB	"DE","C"+80H,05H
	db 0x45 ; 041F8E
	db 0xc3 ; 041F8F
	db 0x05 ; 041F90
;
; Group 8: (2 opcodes)
;
	db 0x49 ; 041F91 49      5375 DB	"IN","0"+80H,00H	; 5Dh
	db 0x4e ; 041F92
	db 0xb0 ; 041F93
	db 0x00 ; 041F94
	db 0x4f ; 041F95 4F      5376 DB	"OUT","0"+80H,01H
	db 0x55 ; 041F96
	db 0x54 ; 041F97
	db 0xb0 ; 041F98
	db 0x01 ; 041F99
;
; Group 9: (1 opcode)
;
	db 0x49 ; 041F9A 49      5380 DB	"I","N"+80H,40H		; 5Fh
	db 0xce ; 041F9B
	db 0x40 ; 041F9C
;
; Group 10: (1 opcode)
;
	db 0x4f ; 041F9D 4F      5384 DB	"OU","T"+80H,41H	; 60h
	db 0x55 ; 041F9E
	db 0xd4 ; 041F9F
	db 0x41 ; 041FA0
;
; Group 11: (2 opcodes)
;
	db 0x4a ; 041FA1 4A      5388 DB	"J","R"+80H,20H		; 61h
	db 0xd2 ; 041FA2
	db 0x20 ; 041FA3
	db 0x44 ; 041FA4 44      5389 DB	"DJN","Z"+80H,10H
	db 0x4a ; 041FA5
	db 0x4e ; 041FA6
	db 0xda ; 041FA7
	db 0x10 ; 041FA8
;
; Group 12: (1 opcode)
;
	db 0x4a ; 041FA9 4A      5393 DB	"J","P"+80H,C2H		; 63h
	db 0xd0 ; 041FAA
	db 0xc2 ; 041FAB
;
; Group 13: (1 opcode)
;
	db 0x43 ; 041FAC 43      5397 DB	"CAL","L"+80H,C4H	; 64h
	db 0x41 ; 041FAD
	db 0x4c ; 041FAE
	db 0xcc ; 041FAF
	db 0xc4 ; 041FB0
;
; Group 14: (1 opcode)
;
	db 0x52 ; 041FB1 52      5401 DB	"RS","T"+80H,C7H	; 65h
	db 0x53 ; 041FB2
	db 0xd4 ; 041FB3
	db 0xc7 ; 041FB4
;
; Group 15: (1 opcode)
;
	db 0x52 ; 041FB5 52      5405 DB	"RE","T"+80H,C0H	; 66h
	db 0x45 ; 041FB6
	db 0xd4 ; 041FB7
	db 0xc0 ; 041FB8
;
; Group 16: (1 opcode)
;
	db 0x4c ; 041FB9 4C      5409 DB	"L","D"+80H,40H		; 67h
	db 0xc4 ; 041FBA
	db 0x40 ; 041FBB
;
; Group 17: (1 opcode)
;
	db 0x54 ; 041FBC 54      5413 DB	"TS","T"+80H,04H	; 68h
	db 0x53 ; 041FBD
	db 0xd4 ; 041FBE
	db 0x04 ; 041FBF
;
; Assembler Directives
;
	db 0x4f ; 041FC0 4F      5418 DB	"OP","T"+80H,00H	; 69h OPT
	db 0x50 ; 041FC1
	db 0xd4 ; 041FC2
	db 0x00 ; 041FC3
	db 0x41 ; 041FC4 41      5419 DB	"AD","L"+80H,00H	; 6Ah ADL
	db 0x44 ; 041FC5
	db 0xcc ; 041FC6
	db 0x00 ; 041FC7
	db 0x5d ; 041FC8 5D      5421 DB	DEF_ & 7FH,"B"+80H,00H	; 6Bh Tokenised DEF + B
	db 0xc2 ; 041FC9
	db 0x00 ; 041FCA
	db 0x5d ; 041FCB 5D      5422 DB	DEF_ & 7FH,"W"+80H,00H	; 6Ch Tokenised DEF + W
	db 0xd7 ; 041FCC
	db 0x00 ; 041FCD
	db 0x5d ; 041FCE 5D      5423 DB	DEF_ & 7FH,"L"+80H,00H	; 6Dh Tokenised DEF + L
	db 0xcc ; 041FCF
	db 0x00 ; 041FD0
	db 0x5d ; 041FD1 5D      5424 DB 	DEF_ & 7FH,"M"+80H,00H	; 6Eh Tokenised DEF + M
	db 0xcd ; 041FD2
	db 0x00 ; 041FD3
	db 0x00 ; 041FD4 00      5426 DB	0
;
; Operands
;
OPRNDS:
	db 0xc2 ; 041FD5 42      5430 OPRNDS:			DB	"B"+80H, 00H
	db 0x00 ; 041FD6
	db 0xc3 ; 041FD7 43      5431 DB	"C"+80H, 01H
	db 0x01 ; 041FD8
	db 0xc4 ; 041FD9 44      5432 DB	"D"+80H, 02H
	db 0x02 ; 041FDA
	db 0xc5 ; 041FDB 45      5433 DB	"E"+80H, 03H
	db 0x03 ; 041FDC
	db 0xc8 ; 041FDD 48      5434 DB	"H"+80H, 04H
	db 0x04 ; 041FDE
	db 0xcc ; 041FDF 4C      5435 DB	"L"+80H, 05H
	db 0x05 ; 041FE0
	db 0x28 ; 041FE1 28      5436 DB	"(H","L"+80H,06H
	db 0x48 ; 041FE2
	db 0xcc ; 041FE3
	db 0x06 ; 041FE4
	db 0xc1 ; 041FE5 41      5437 DB	"A"+80H, 07H
	db 0x07 ; 041FE6
	db 0x28 ; 041FE7 28      5438 DB	"(I","X"+80H,86H
	db 0x49 ; 041FE8
	db 0xd8 ; 041FE9
	db 0x86 ; 041FEA
	db 0x28 ; 041FEB 28      5439 DB	"(I","Y"+80H,C6H
	db 0x49 ; 041FEC
	db 0xd9 ; 041FED
	db 0xc6 ; 041FEE
	db 0x42 ; 041FEF 42      5441 DB	"B","C"+80H,08H
	db 0xc3 ; 041FF0
	db 0x08 ; 041FF1
	db 0x44 ; 041FF2 44      5442 DB	"D","E"+80H,0AH
	db 0xc5 ; 041FF3
	db 0x0a ; 041FF4
	db 0x48 ; 041FF5 48      5443 DB	"H","L"+80H,0CH
	db 0xcc ; 041FF6
	db 0x0c ; 041FF7
	db 0x49 ; 041FF8 49      5444 DB	"I","X"+80H,8CH
	db 0xd8 ; 041FF9
	db 0x8c ; 041FFA
	db 0x49 ; 041FFB 49      5445 DB	"I","Y"+80H,CCH
	db 0xd9 ; 041FFC
	db 0xcc ; 041FFD
	db 0x41 ; 041FFE 41      5446 DB	"A","F"+80H,0EH
	db 0xc6 ; 041FFF
	db 0x0e ; 042000
	db 0x53 ; 042001 53      5447 DB	"S","P"+80H,0EH
	db 0xd0 ; 042002
	db 0x0e ; 042003
	db 0x4e ; 042004 4E      5449 DB	"N","Z"+80H,10H
	db 0xda ; 042005
	db 0x10 ; 042006
	db 0xda ; 042007 5A      5450 DB	"Z"+80H,11H
	db 0x11 ; 042008
	db 0x4e ; 042009 4E      5451 DB	"N","C"+80H,12H
	db 0xc3 ; 04200A
	db 0x12 ; 04200B
	db 0x50 ; 04200C 50      5452 DB	"P","O"+80H,14H
	db 0xcf ; 04200D
	db 0x14 ; 04200E
	db 0x50 ; 04200F 50      5453 DB	"P","E"+80H,15H
	db 0xc5 ; 042010
	db 0x15 ; 042011
	db 0xd0 ; 042012 50      5454 DB	"P"+80H,16H
	db 0x16 ; 042013
	db 0xcd ; 042014 4D      5455 DB	"M"+80H,17H
	db 0x17 ; 042015
	db 0x28 ; 042016 28      5457 DB	"(","C"+80H,20H
	db 0xc3 ; 042017
	db 0x20 ; 042018
	db 0x00 ; 042019 00      5459 DB	0
;
; Load operations
;
LDOPS:
	db 0x49 ; 04201A 49      5463 LDOPS:			DB	"I",0,"A"+80H,47H
	db 0x00 ; 04201B
	db 0xc1 ; 04201C
	db 0x47 ; 04201D
	db 0x52 ; 04201E 52      5464 DB	"R",0,"A"+80H,4FH
	db 0x00 ; 04201F
	db 0xc1 ; 042020
	db 0x4f ; 042021
	db 0x41 ; 042022 41      5465 DB	"A",0,"I"+80H,57H
	db 0x00 ; 042023
	db 0xc9 ; 042024
	db 0x57 ; 042025
	db 0x41 ; 042026 41      5466 DB	"A",0,"R"+80H,5FH
	db 0x00 ; 042027
	db 0xd2 ; 042028
	db 0x5f ; 042029
	db 0x28 ; 04202A 28      5467 DB	"(BC",0,"A"+80H,02h
	db 0x42 ; 04202B
	db 0x43 ; 04202C
	db 0x00 ; 04202D
	db 0xc1 ; 04202E
	db 0x02 ; 04202F
	db 0x28 ; 042030 28      5468 DB	"(DE",0,"A"+80H,12H
	db 0x44 ; 042031
	db 0x45 ; 042032
	db 0x00 ; 042033
	db 0xc1 ; 042034
	db 0x12 ; 042035
	db 0x41 ; 042036 41      5469 DB	"A",0,"(B","C"+80H,0AH
	db 0x00 ; 042037
	db 0x28 ; 042038
	db 0x42 ; 042039
	db 0xc3 ; 04203A
	db 0x0a ; 04203B
	db 0x41 ; 04203C 41      5470 DB	"A",0,"(D","E"+80H,1AH
	db 0x00 ; 04203D
	db 0x28 ; 04203E
	db 0x44 ; 04203F
	db 0xc5 ; 042040
	db 0x1a ; 042041
	db 0x00 ; 042042 00      5472 DB	0
;
; eZ80 addressing mode suffixes
;
; Fully qualified suffixes
;
EZ80SFS_1:
	db 0x4c ; 042043 4C      5478 EZ80SFS_1:		DB	"LI","S"+80H,49H
	db 0x49 ; 042044
	db 0xd3 ; 042045
	db 0x49 ; 042046
	db 0x53 ; 042047 53      5479 DB	"SI","L"+80H,52H
	db 0x49 ; 042048
	db 0xcc ; 042049
	db 0x52 ; 04204A

EZ80SFS_2:
	db 0x53 ; 04204B 53      5480 EZ80SFS_2:		DB	"SI","S"+80H,40H
	db 0x49 ; 04204C
	db 0xd3 ; 04204D
	db 0x40 ; 04204E
	db 0x4c ; 04204F 4C      5481 DB	"LI","L"+80H,5BH
	db 0x49 ; 042050
	db 0xcc ; 042051
	db 0x5b ; 042052
	db 0x00 ; 042053 00      5483 DB	0
;
; Shortcuts when ADL mode is 0
;
EZ80SFS_ADL0:
	db 0xd3 ; 042054 53      5487 EZ80SFS_ADL0:		DB	"S"+80H,40H		; Equivalent to .SIS
	db 0x40 ; 042055
	db 0xcc ; 042056 4C      5488 DB	"L"+80H,49H		; Equivalent to .LIS
	db 0x49 ; 042057
	db 0x49 ; 042058 49      5489 DB	"I","S"+80H,40H		; Equivalent to .SIS
	db 0xd3 ; 042059
	db 0x40 ; 04205A
	db 0x49 ; 04205B 49      5490 DB	"I","L"+80H,52H		; Equivalent to .SIL
	db 0xcc ; 04205C
	db 0x52 ; 04205D
	db 0x00 ; 04205E 00      5492 DB	0
;
; Shortcuts when ADL mode is 1
;
EZ80SFS_ADL1:
	db 0xd3 ; 04205F 53      5496 EZ80SFS_ADL1:		DB	"S"+80H,52H		; Equivalent to .SIL
	db 0x52 ; 042060
	db 0xcc ; 042061 4C      5497 DB	"L"+80H,5BH		; Equivalent to .LIL
	db 0x5b ; 042062
	db 0x49 ; 042063 49      5498 DB	"I","S"+80H,49H		; Equivalent to .LIS
	db 0xd3 ; 042064
	db 0x49 ; 042065
	db 0x49 ; 042066 49      5499 DB	"I","L"+80H,5BH		; Equivalent to .LIL
	db 0xcc ; 042067
	db 0x5b ; 042068
	db 0x00 ; 042069 00      5501 DB	0
; END INSERT FROM BINARY
;
; .LIST
;
; already defined in equs.inc
; LF:			EQU     0AH
; CR:			EQU     0DH; --- End exec.asm ---

; --- Begin fpp.asm ---
;
; Title:	BBC Basic Interpreter - Z80 version
;		Z80 Floating Point Package
; Author:	(C) Copyright  R.T.Russell  1986
; Modified By:	Dean Belfield
; Created:	03/05/2022
; Last Updated:	07/06/2023
;
; Modinfo:
; 26/10/1986:	Version 0.0
; 14/12/1988:	Vesion 0.1 (Bug Fix)
; 12/05/2023:	Modified by Dean Belfield
; 07/06/2023:	Modified to run in ADL mode

			; .ASSUME	ADL = 1

			; SEGMENT CODE
				
			; XDEF	FPP
			; XDEF	DLOAD5
			; XDEF	DLOAD5_SPL			
;
;BINARY FLOATING POINT REPRESENTATION:
;   32 BIT SIGN-MAGNITUDE NORMALIZED MANTISSA
;    8 BIT EXCESS-128 SIGNED EXPONENT
;   SIGN BIT REPLACES MANTISSA MSB (IMPLIED "1")
;   MANTISSA=0 & EXPONENT=0 IMPLIES VALUE IS ZERO.
;
;BINARY INTEGER REPRESENTATION:
;   32 BIT 2'S-COMPLEMENT SIGNED INTEGER
;    "EXPONENT" BYTE = 0 (WHEN PRESENT)
;
;NORMAL REGISTER ALLOCATION: MANTISSA - HLH'L'
;                            EXPONENT - C
;ALTERNATE REGISTER ALLOCATION: MANTISSA - DED'E'
;                               EXPONENT - B

;
;Error codes:
;

BADOP:			EQU     1               ;Bad operation code
DIVBY0:			EQU     18              ;Division by zero
TOOBIG_FP:			EQU     20              ;Too big
NGROOT:			EQU     21              ;Negative root
LOGRNG:			EQU     22              ;Log range
ACLOST:			EQU     23              ;Accuracy lost
EXPRNG:			EQU     24              ;Exp range
;
;Call entry and despatch code:
;
FPP:			PUSH    IY              ;Save IY
        		LD      IY,0
        		ADD     IY,SP           ;Save SP in IY
        		CALL    OP              ;Perform operation
        		CP      A               ;Good return (Z, NC)
EXIT_FP_:			POP     IY              ;Restore IY
        		RET                     ;Return to caller
;
;Error exit:
;
BAD_FP:			LD      A,BADOP         ;"Bad operation code"
ERROR_FP_:			LD      SP,IY           ;Restore SP from IY
        		OR      A               ;Set NZ
        		SCF                     ;Set C
        		JR      EXIT_FP_
;
;Perform operation or function:
;
; OP:			CP      (RTABLE-DTABLE)/3
OP:				CP      RTABLE-DTABLE/3 ; ez80asm doesn't do nested expressions

        		JR      NC,BAD_FP
        		; CP      (FTABLE-DTABLE)/3
				CP      FTABLE-DTABLE/3 ; ditto
        		JR      NC,DISPAT_FP
        		EX      AF,AF'
        		LD      A,B
        		OR      C               ;Both integer?
        		CALL    NZ,FLOATA       ;No, so float both
        		EX      AF,AF'
DISPAT_FP:			PUSH    HL
        		LD      HL,DTABLE
        		PUSH    BC
			LD	BC, 3		; C = 3
			LD	B, A 		; B = op-code
			MLT 	BC 		;BC = op-code * 3
			ADD	HL, BC 		;Add to table base 
			LD	HL, (HL)	;Get the routine address (24-bit)

;        		ADD     A, A            ;A = op-code * 2
;        		LD      C,A
;        		LD      B,0             ;BC = op-code * 2
;        		ADD     HL,BC
;        		LD      A,(HL)          ;Get low byte
;        		INC     HL
;        		LD      H,(HL)          ;Get high byte
;        		LD      L,A

        		POP     BC
        		EX      (SP),HL
        		RET                     ;Off to routine
;
;Despatch table:
;
DTABLE:			DW24  IAND            ;AND (INTEGER)
        		DW24  IBDIV           ;DIV
        		DW24  IEOR            ;EOR
        		DW24  IMOD            ;MOD
        		DW24  IOR             ;OR
        		DW24  ILE             ;<=
        		DW24  INE             ;<>
        		DW24  IGE             ;>=
        		DW24  ILT             ;<
        		DW24  IEQ             ;=
        		DW24  IMUL            ;*
        		DW24  IADD            ;+
        		DW24  IGT             ;>
        		DW24  ISUB            ;-
        		DW24  IPOW            ;^
        		DW24  IDIV            ;/
;
FTABLE:			
				DW24  ABSV_FP            ;ABS
        		DW24  ACS_FP             ;ACS
        		DW24  ASN_FP             ;ASN
        		DW24  ATN_FP             ;ATN
        		DW24  COS_FP             ;COS
        		DW24  DEG_FP             ;DEG
        		DW24  EXP_FP             ;EXP
        		DW24  INT_FP_            ;INT
        		DW24  LN_FP              ;LN
        		DW24  LOG_FP             ;LOG
        		DW24  NOTK_FP            ;NOT
        		DW24  RAD_FP             ;RAD
        		DW24  SGN_FP             ;SGN
        		DW24  SIN_FP             ;SIN
        		DW24  SQR_FP             ;SQR
        		DW24  TAN_FP             ;TAN
;
		        DW24  ZERO_FP            ;ZERO
        		DW24  FONE_FP            ;FONE
        		DW24  TRUE_FP            ;TRUE
        		DW24  PI_FP              ;PI
;
		        DW24  VAL_FP             ;VAL
        		DW24  STR_FP             ;STR$
;
        		DW24  SFIX_FP            ;FIX
        		DW24  SFLOAT_FP          ;FLOAT
;
		        DW24  FTEST_FP           ;TEST
        		DW24  FCOMP_FP           ;COMPARE
;
RTABLE:			DW24  FAND            ;AND (FLOATING-POINT)
        		DW24  FBDIV           ;DIV
        		DW24  FEOR            ;EOR
        		DW24  FMOD            ;MOD
        		DW24  FFOR             ;OR
        		DW24  FLE             ;<= 
        		DW24  FNE             ;<>
        		DW24  FGE             ;>=
        		DW24  FLT             ;<
        		DW24  FEQ             ;=
        		DW24  FMUL            ;*
        		DW24  FADD            ;+
        		DW24  FGT             ;>
        		DW24  FSUB            ;-
        		DW24  FPOW            ;^
        		DW24  FDIV            ;/
;
;       PAGE
;
;ARITHMETIC AND LOGICAL OPERATORS:
;All take two arguments, in HLH'L'C & DED'E'B.
;Output in HLH'L'C
;All registers except IX, IY destroyed.
; (N.B. FPOW destroys IX).
;
;FAND - Floating-point AND.
;IAND - Integer AND.
;
FAND:			CALL    FIX2
IAND:			LD      A,H
        		AND     D
        		LD      H,A
        		LD      A,L
        		AND     E
        		LD      L,A
        		EXX
        		LD      A,H
        		AND     D
        		LD      H,A
        		LD      A,L
        		AND     E
        		LD      L,A
        		EXX
        		RET
;
;FEOR - Floating-point exclusive-OR.
;IEOR - Integer exclusive-OR.
;
FEOR:			CALL    FIX2
IEOR:			LD      A,H
        		XOR     D
        		LD      H,A
        		LD      A,L
        		XOR     E
        		LD      L,A
        		EXX
        		LD      A,H
        		XOR     D
        		LD      H,A
        		LD      A,L
        		XOR     E
        		LD      L,A
        		EXX
        		RET
;
;FOR - Floating-point OR.
;IOR - Integer OR.
;
FFOR:			CALL    FIX2
IOR:			LD      A,H
        		OR      D
        		LD      H,A
        		LD      A,L
        		OR      E
        		LD      L,A
        		EXX
        		LD      A,H
        		OR      D
        		LD      H,A
        		LD      A,L
        		OR      E
        		LD      L,A
        		EXX
        		RET
;
;FMOD - Floating-point remainder.
;IMOD - Integer remainder.
;
FMOD:			CALL    FIX2
IMOD:			LD      A,H
        		XOR     D               ;DIV RESULT SIGN
        		BIT     7,H
        		EX      AF,AF'
        		BIT     7,H
        		CALL    NZ,NEGATE       ;MAKE ARGUMENTS +VE
        		CALL    SWAP_FP
        		BIT     7,H
        		CALL    NZ,NEGATE
        		LD      B,H
        		LD      C,L
        		LD      HL,0
        		EXX
        		LD      B,H
        		LD      C,L
        		LD      HL,0
        		LD      A,-33
        		CALL    DIVA            ;DIVIDE
        		EXX
        		LD      C,0             ;INTEGER MARKER
        		EX      AF,AF'
        		RET     Z
        		JP      NEGATE
;
;BDIV - Integer division.
;
FBDIV:			CALL    FIX2
IBDIV:			CALL    IMOD
        		OR      A
        		CALL    SWAP_FP
        		LD      C,0
        		RET     P
        		JP      NEGATE
;
;ISUB - Integer subtraction.
;FSUB - Floating point subtraction with rounding.
;
ISUB:			CALL    SUB_
        		RET     PO
        		CALL    ADD_
        		CALL    FLOAT2
FSUB:			LD      A,D
        		XOR     80H             ;CHANGE SIGN THEN ADD
        		LD      D,A
        		JR      FADD
;
;Reverse subtract.
;
RSUB:			LD      A,H
        		XOR     80H
        		LD      H,A
        		JR      FADD
;
;IADD - Integer addition.
;FADD - Floating point addition with rounding.
;
IADD:			CALL    ADD_
        		RET     PO
        		CALL    SUB_
        		CALL    FLOAT2
FADD:			DEC     B
        		INC     B
        		RET     Z               ;ARG 2 ZERO
        		DEC     C
        		INC     C
        		JP      Z,SWAP_FP          ;ARG 1 ZERO
        		EXX
        		LD      BC,0            ;INITIALISE
        		EXX
        		LD      A,H
        		XOR     D               ;XOR SIGNS
        		PUSH    AF
        		LD      A,B
        		CP      C               ;COMPARE EXPONENTS
        		CALL    C,SWAP_FP          ;MAKE DED'E'B LARGEST
        		LD      A,B
        		SET     7,H             ;IMPLIED 1
        		CALL    NZ,FIX          ;ALIGN
        		POP     AF
        		LD      A,D             ;SIGN OF LARGER
        		SET     7,D             ;IMPLIED 1
        		JP      M,FADD3         ;SIGNS DIFFERENT
        		CALL    ADD_             ;HLH'L'=HLH'L'+DED'E'
        		CALL    C,DIV2          ;NORMALISE
        		SET     7,H
        		JR      FADD4
;
FADD3:			CALL    SUB_             ;HLH'L'=HLH'L'-DED'E'
        		CALL    C,NEG_           ;NEGATE HLH'L'B'C'
        		CALL    FLO48
        		CPL                     ;CHANGE RESULT SIGN
FADD4:			EXX
        		EX      DE,HL
        		LD      HL,8000H
        		OR      A               ;CLEAR CARRY
        		SBC.S   HL,BC
        		EX      DE,HL
        		EXX
        		CALL    Z,ODD           ;ROUND UNBIASSED
        		CALL    C,ADD1_FP          ;ROUND UP
        		CALL    C,INCC
        		RES     7,H
        		DEC     C
        		INC     C
        		JP      Z,ZERO_FP
        		OR      A               ;RESULT SIGNQ
        		RET     P               ;POSITIVE
        		SET     7,H             ;NEGATIVE
        		RET
;
;IDIV - Integer division.
;FDIV - Floating point division with rounding.
;
IDIV:			CALL    FLOAT2
FDIV:			DEC     B               ;TEST FOR ZERO
        		INC     B
        		LD      A,DIVBY0
        		JP      Z,ERROR_FP_         ;"Division by zero"
        		DEC     C               ;TEST FOR ZERO
        		INC     C
        		RET     Z
        		LD      A,H
        		XOR     D               ;CALC. RESULT SIGN
        		EX      AF,AF'          ;SAVE SIGN
        		SET     7,D             ;REPLACE IMPLIED 1's
        		SET     7,H
        		PUSH    BC              ;SAVE EXPONENTS
        		LD      B,D             ;LOAD REGISTERS
        		LD      C,E
        		LD      DE,0
        		EXX
        		LD      B,D
        		LD      C,E
        		LD      DE,0
        		LD      A,-32           ;LOOP COUNTER
        		CALL    DIVA            ;DIVIDE
        		EXX
        		BIT     7,D
        		EXX
        		CALL    Z,DIVB          ;NORMALISE & INC A
        		EX      DE,HL
        		EXX
        		SRL     B               ;DIVISOR/2
        		RR      C
        		OR      A               ;CLEAR CARRY
        		SBC.S   HL,BC           ;REMAINDER-DIVISOR/2
        		CCF
        		EX      DE,HL           ;RESULT IN HLH'L'
        		CALL    Z,ODD           ;ROUND UNBIASSED
        		CALL    C,ADD1_FP          ;ROUND UP
        		POP     BC              ;RESTORE EXPONENTS
        		CALL    C,INCC
        		RRA                     ;LSB OF A TO CARRY
        		LD      A,C             ;COMPUTE NEW EXPONENT
        		SBC     A,B
        		CCF
        		JP      CHKOVF
;
;IMUL - Integer multiplication.
;
IMUL:			LD      A,H
        		XOR     D
        		EX      AF,AF'          ;SAVE RESULT SIGN
        		BIT     7,H
        		CALL    NZ,NEGATE
        		CALL    SWAP_FP
        		BIT     7,H
        		CALL    NZ,NEGATE
        		LD      B,H
        		LD      C,L
        		LD      HL,0
        		EXX
        		LD      B,H
        		LD      C,L
        		LD      HL,0
        		LD      A,-33
        		CALL    MULA            ;MULTIPLY
        		EXX
        		LD      C,191           ;PRESET EXPONENT
        		CALL    TEST_FP            ;TEST RANGE
        		JR      NZ,IMUL1        ;TOO BIG
        		BIT     7,D
        		JR      NZ,IMUL1
        		CALL    SWAP_FP
        		LD      C,D             ;INTEGER MARKER
        		EX      AF,AF'
        		RET     P
        		JP      NEGATE
;
IMUL1:			DEC     C
        		EXX
        		SLA     E
        		RL      D
        		EXX
        		RL      E
        		RL      D
        		EXX
        		ADC.S   HL,HL
        		EXX
        		ADC.S   HL,HL
        		JP      P,IMUL1         ;NORMALISE
        		EX      AF,AF'
        		RET     M
        		RES     7,H             ;POSITIVE
        		RET
;
;FMUL - Floating point multiplication with rounding.
;
FMUL:			DEC     B               ;TEST FOR ZERO
        		INC     B
        		JP      Z,ZERO_FP
        		DEC     C               ;TEST FOR ZERO
        		INC     C
        		RET     Z
        		LD      A,H
        		XOR     D               ;CALC. RESULT SIGN
        		EX      AF,AF'
        		SET     7,D             ;REPLACE IMPLIED 1's
        		SET     7,H
        		PUSH    BC              ;SAVE EXPONENTS
        		LD      B,H             ;LOAD REGISTERS
        		LD      C,L
        		LD      HL,0
        		EXX
        		LD      B,H
        		LD      C,L
        		LD      HL,0
        		LD      A,-32           ;LOOP COUNTER
        		CALL    MULA            ;MULTIPLY
        		CALL    C,MULB          ;NORMALISE & INC A
        		EXX
        		PUSH    HL
        		LD      HL,8000H
        		OR      A               ;CLEAR CARRY
        		SBC.S   HL,DE
        		POP     HL
        		CALL    Z,ODD           ;ROUND UNBIASSED
        		CALL    C,ADD1_FP          ;ROUND UP
        		POP     BC              ;RESTORE EXPONENTS
        		CALL    C,INCC
        		RRA                     ;LSB OF A TO CARRY
        		LD      A,C             ;COMPUTE NEW EXPONENT
        		ADC     A,B
CHKOVF:			JR      C,CHKO1
        		JP      P,ZERO_FP          ;UNDERFLOW
        		JR      CHKO2
CHKO1:			JP      M,OFLOW         ;OVERFLOW
CHKO2:			ADD     A,80H
        		LD      C,A
        		JP      Z,ZERO_FP
        		EX      AF,AF'          ;RESTORE SIGN BIT
        		RES     7,H
        		RET     P
        		SET     7,H
        		RET
;
;IPOW - Integer involution.
;
IPOW:			CALL    SWAP_FP
        		BIT     7,H
        		PUSH    AF              ;SAVE SIGN
        		CALL    NZ,NEGATE
IPOW0:			LD      C,B
        		LD      B,32            ;LOOP COUNTER
IPOW1:			CALL    X2
        		JR      C,IPOW2
        		DJNZ    IPOW1
        		POP     AF
        		EXX
        		INC     L               ;RESULT=1
        		EXX
        		LD      C,H
        		RET
;
IPOW2:			POP     AF
        		PUSH    BC
        		EX      DE,HL
        		PUSH    HL
        		EXX
        		EX      DE,HL
        		PUSH    HL
        		EXX
        		LD      IX,0
        		ADD     IX,SP
        		JR      Z,IPOW4
        		PUSH    BC
        		EXX
        		PUSH    DE
        		EXX
        		PUSH    DE
        		CALL    SFLOAT_FP
        		CALL    RECIP
        		LD      (IX+4),C
        		EXX
        		LD      (IX+0),L
        		LD      (IX+1),H
        		EXX
        		LD      (IX+2),L
        		LD      (IX+3),H
        		JR      IPOW5
;
IPOW3:			PUSH    BC
        		EXX
        		SLA     E
        		RL      D
        		PUSH    DE
        		EXX
        		RL      E
        		RL      D
        		PUSH    DE
        		LD      A,'*' & 0FH
        		PUSH    AF
        		CALL    COPY_
        		CALL    OP              ;SQUARE
        		POP     AF
        		CALL    DLOAD5
        		CALL    C,OP            ;MULTIPLY BY X
IPOW5:			POP     DE
        		EXX
        		POP     DE
        		EXX
        		LD      A,C
        		POP     BC
        		LD      C,A
IPOW4:			DJNZ    IPOW3
        		POP     AF
        		POP     AF
        		POP     AF
        		RET
;
FPOW0:			POP     AF
        		POP     AF
        		POP     AF
        		JR      IPOW0
;
;FPOW - Floating-point involution.
;
FPOW:			BIT     7,D
        		PUSH    AF
        		CALL    SWAP_FP
        		CALL    PUSH5
        		DEC     C
        		INC     C
        		JR      Z,FPOW0
        		LD      A,158
        		CP      C
        		JR      C,FPOW1
        		INC     A
        		CALL    FIX
        		EX      AF,AF'
        		JP      P,FPOW0
FPOW1:			CALL    SWAP_FP
        		CALL    LN0
        		CALL    POP5
        		POP     AF
        		CALL    FMUL
        		JP      EXP0
;
;Integer and floating-point compare.
;Result is TRUE (-1) or FALSE (0).
;
FLT:			CALL    FCP
        		JR      ILT1
ILT:			CALL    ICP
ILT1:			RET     NC
        		JR      TRUE_FP
;
FGT:			CALL    FCP
        		JR      IGT1
IGT:			CALL    ICP
IGT1:			RET     Z
        		RET     C
        		JR      TRUE_FP
;
FGE:			CALL    FCP
        		JR      IGE1
IGE:			CALL    ICP
IGE1:			RET     C
        		JR      TRUE_FP
;
FLE:			CALL    FCP
        		JR      ILE1
ILE:			CALL    ICP
ILE1:			JR      Z,TRUE_FP
        		RET     NC
        		JR      TRUE_FP
;
FNE:			CALL    FCP
        		JR      INE1
INE:			CALL    ICP
INE1:			RET     Z
        		JR      TRUE_FP
;
FEQ:			CALL    FCP
        		JR      IEQ1
IEQ:			CALL    ICP
IEQ1:			RET     NZ
TRUE_FP:			LD      HL,-1
        		EXX
        		LD      HL,-1
        		EXX
        		XOR     A
        		LD      C,A
        		RET
;
;FUNCTIONS:
;
;Result returned in HLH'L'C (floating point)
;Result returned in HLH'L' (C=0) (integer)
;All registers except IY destroyed.
;
;ABS - Absolute value
;Result is numeric, variable type.
;
ABSV_FP:			BIT     7,H
        		RET     Z               ;POSITIVE/ZERO
        		DEC     C
        		INC     C
        		JP      Z,NEGATE        ;INTEGER
        		RES     7,H
        		RET
;
;NOT - Complement integer.
;Result is integer numeric.
;
NOTK_FP:			CALL    SFIX_FP
        		LD      A,H
        		CPL
        		LD      H,A
        		LD      A,L
        		CPL
        		LD      L,A
        		EXX
        		LD      A,H
        		CPL
        		LD      H,A
        		LD      A,L
        		CPL
        		LD      L,A
        		EXX
        		XOR     A               ;NUMERIC MARKER
        		RET
;
;PI - Return PI (3.141592654)
;Result is floating-point numeric.
;
PI_FP:			LD      HL,490FH
        		EXX
        		LD      HL,0DAA2H
        		EXX
        		LD      C,81H
        		XOR     A               ;NUMERIC MARKER
        		RET
;
;DEG - Convert radians to degrees
;Result is floating-point numeric.
;
DEG_FP:			CALL    FPI180
        		CALL    FMUL
        		XOR     A
        		RET
;
;RAD - Convert degrees to radians
;Result is floating-point numeric.
;
RAD_FP:			CALL    FPI180
        		CALL    FDIV
        		XOR     A
        		RET
;
;180/PI
;
FPI180:			CALL    SFLOAT_FP
        		LD      DE,652EH
        		EXX
        		LD      DE,0E0D3H
        		EXX
        		LD      B,85H
        		RET
;
;SGN - Return -1, 0 or +1
;Result is integer numeric.
;
SGN_FP:			CALL    TEST_FP
        		OR      C
        		RET     Z               ;ZERO
        		BIT     7,H
        		JP      NZ,TRUE_FP         ;-1
        		CALL    ZERO_FP
        		JP      ADD1_FP            ;1
;
;VAL - Return numeric value of string.
;Input: ASCII string at IX
;Result is variable type numeric.
;
VAL_FP:			CALL    SIGNQ
        		PUSH    AF
        		CALL    CON_FP
        		POP     AF
        		CP      '-'
        		LD      A,0             ;NUMERIC MARKER
        		RET     NZ
        		DEC     C
        		INC     C
        		JP      Z,NEGATE        ;ZERO/INTEGER
        		LD      A,H
        		XOR     80H             ;CHANGE SIGN (FP)
        		LD      H,A
        		XOR     A
        		RET
;
;INT - Floor function
;Result is integer numeric.
;
INT_FP_:			DEC     C
        		INC     C
        		RET     Z               ;ZERO/INTEGER
        		LD      A,159
        		LD      B,H             ;B7=SIGN BIT
        		CALL    FIX
        		EX      AF,AF'
        		AND     B
        		CALL    M,ADD1_FP          ;NEGATIVE NON-INTEGER
        		LD      A,B
        		OR      A
        		CALL    M,NEGATE
        		XOR     A
        		LD      C,A
        		RET
;
;SQR - square root
;Result is floating-point numeric.
;
SQR_FP:			CALL    SFLOAT_FP
SQR0:			BIT     7,H
        		LD      A,NGROOT
        		JP      NZ,ERROR_FP_        ;"-ve root"
        		DEC     C
        		INC     C
        		RET     Z               ;ZERO
        		SET     7,H             ;IMPLIED 1
        		BIT     0,C
        		CALL    Z,DIV2          ;MAKE EXPONENT ODD
        		LD      A,C
        		SUB     80H
        		SRA     A               ;HALVE EXPONENT
        		ADD     A,80H
        		LD      C,A
        		PUSH    BC              ;SAVE EXPONENT
        		EX      DE,HL
        		LD      HL,0
        		LD      B,H
        		LD      C,L
        		EXX
        		EX      DE,HL
        		LD      HL,0
        		LD      B,H
        		LD      C,L
        		LD      A,-31
        		CALL    SQRA            ;ROOT
        		EXX
        		BIT     7,B
        		EXX
        		CALL    Z,SQRA          ;NORMALISE & INC A
        		CALL    SQRB
        		OR      A               ;CLEAR CARRY
        		CALL    DIVB
        		RR      E               ;LSB TO CARRY
        		LD      H,B
        		LD      L,C
        		EXX
        		LD      H,B
        		LD      L,C
        		CALL    C,ADD1_FP          ;ROUND UP
        		POP     BC              ;RESTORE EXPONENT
        		CALL    C,INCC
        		RRA
        		SBC     A,A
        		ADD     A,C
        		LD      C,A
        		RES     7,H             ;POSITIVE
        		XOR     A
        		RET
;
;TAN - Tangent function
;Result is floating-point numeric.
;
TAN_FP:			CALL    SFLOAT_FP
        		CALL    PUSH5
        		CALL    COS0
        		CALL    POP5
        		CALL    PUSH5
        		CALL    SWAP_FP
        		CALL    SIN0
        		CALL    POP5
        		CALL    FDIV
        		XOR     A               ;NUMERIC MARKER
        		RET
;
;COS - Cosine function
;Result is floating-point numeric.
;
COS_FP:			CALL    SFLOAT_FP
COS0:			CALL    SCALE
        		INC     E
        		INC     E
        		LD      A,E
        		JR      SIN1
;
;SIN - Sine function
;Result is floating-point numeric.
;
SIN_FP:			CALL    SFLOAT_FP
SIN0:			PUSH    HL              ;H7=SIGN
        		CALL    SCALE
        		POP     AF
        		RLCA
        		RLCA
        		RLCA
        		AND     4
        		XOR     E
SIN1:			PUSH    AF              ;OCTANT
        		RES     7,H
        		RRA
        		CALL    PIBY4
        		CALL    C,RSUB          ;X=(PI/4)-X
        		POP     AF
        		PUSH    AF
        		AND     3
        		JP      PO,SIN2         ;USE COSINE APPROX.
        		CALL    PUSH5           ;SAVE X
        		CALL    SQUARE          ;PUSH X*X
        		CALL    POLY
        		DW	0A8B7H          ;a(8)
        		DW	3611H
        		DB	6DH
        		DW	0DE26H          ;a(6)
        		DW	0D005H
        		DB	73H
        		DW	80C0H           ;a(4)
        		DW	888H
        		DB	79H
        		DW	0AA9DH          ;a(2)
        		DW	0AAAAH
        		DB	7DH
        		DW	0               ;a(0)
        		DW	0
        		DB	80H
        		CALL    POP5
        		CALL    POP5
        		CALL    FMUL
        		JP      SIN3
;
SIN2:			CALL    SQUARE          ;PUSH X*X
        		CALL    POLY
        		DW	0D571H          ;b(8)
        		DW	4C78H
        		DB	70H
        		DW	94AFH           ;b(6)
        		DW	0B603H
        		DB	76H
        		DW	9CC8H           ;b(4)
        		DW	2AAAH
        		DB	7BH
        		DW	0FFDDH          ;b(2)
        		DW	0FFFFH
        		DB	7EH
        		DW	0               ;b(0)
        		DW	0
        		DB	80H
        		CALL    POP5
SIN3:			POP     AF
        		AND     4
        		RET     Z
        		DEC     C
        		INC     C
        		RET     Z               ;ZERO
        		SET     7,H             ;MAKE NEGATIVE
        		RET
;
;Floating-point one:
;
FONE_FP:			LD      HL,0
        		EXX
        		LD      HL,0
        		EXX
        		LD      C,80H
        		RET
;
DONE:			LD      DE,0
        		EXX
        		LD      DE,0
        		EXX
        		LD      B,80H
        		RET
;
PIBY4:			LD      DE,490FH
        		EXX
        		LD      DE,0DAA2H
        		EXX
        		LD      B,7FH
        		RET
;
;EXP - Exponential function
;Result is floating-point numeric.
;
EXP_FP:			CALL    SFLOAT_FP
EXP0:			CALL    LN2             ;LN(2)
        		EXX
	        	DEC     E
		        LD      BC,0D1CFH       ;0.6931471805599453
        		EXX
        		PUSH    HL              ;H7=SIGN
        		CALL    MOD48           ;"MODULUS"
        		POP     AF
        		BIT     7,E
        		JR      Z,EXP1
        		RLA
        		JP      C,ZERO_FP
        		LD      A,EXPRNG
        		JP      ERROR_FP_           ;"Exp range"
;
EXP1:			AND     80H
        		OR      E
        		PUSH    AF              ;INTEGER PART
        		RES     7,H
        		CALL    PUSH5           ;PUSH X*LN(2)
        		CALL    POLY
        		DW	4072H           ;a(7)
        		DW	942EH
        		DB	73H
        		DW	6F65H           ;a(6)
        		DW	2E4FH
        		DB	76H
        		DW	6D37H           ;a(5)
        		DW	8802H
        		DB	79H
        		DW	0E512H          ;a(4)
        		DW	2AA0H
        		DB	7BH
        		DW	4F14H           ;a(3)
        		DW	0AAAAH
        		DB	7DH
        		DW	0FD56H          ;a(2)
        		DW	7FFFH
        		DB	7EH
        		DW	0FFFEH          ;a(1)
        		DW	0FFFFH
        		DB	7FH
        		DW	0               ;a(0)
        		DW	0
        		DB	80H
        		CALL    POP5
        		POP     AF
        		PUSH    AF
        		CALL    P,RECIP         ;X=1/X
        		POP     AF
        		JP      P,EXP4
        		AND     7FH
        		NEG
EXP4:			ADD     A,80H
        		ADD     A,C
        		JR      C,EXP2
        		JP      P,ZERO_FP          ;UNDERFLOW
        		JR      EXP3
EXP2:			JP      M,OFLOW         ;OVERFLOW
EXP3:			ADD     A,80H
        		JP      Z,ZERO_FP
        		LD      C,A
        		XOR     A               ;NUMERIC MARKER
        		RET
;
RECIP:			CALL    DONE
RDIV:			CALL    SWAP_FP
        		JP      FDIV            ;RECIPROCAL
;
LN2:			LD      DE,3172H        ;LN(2)
        		EXX
        		LD      DE,17F8H
        		EXX
        		LD      B,7FH
        		RET
;
;LN - Natural log.
;Result is floating-point numeric.
;
LN_FP:			CALL    SFLOAT_FP
LN0:			LD      A,LOGRNG
        		BIT     7,H
        		JP      NZ,ERROR_FP_        ;"Log range"
        		INC     C
        		DEC     C
        		JP      Z,ERROR_FP_
        		LD      DE,3504H        ;SQR(2)
        		EXX
        		LD      DE,0F333H       ;1.41421356237
        		EXX
        		CALL    ICP0            ;MANTISSA>SQR(2)?
        		LD      A,C             ;EXPONENT
        		LD      C,80H           ;1 <= X < 2
        		JR      C,LN4
        		DEC     C
        		INC     A
LN4:			PUSH    AF              ;SAVE EXPONENT
        		CALL    RATIO           ;X=(X-1)/(X+1)
        		CALL    PUSH5
		        CALL    SQUARE          ;PUSH X*X
        		CALL    POLY
        		DW	0CC48H          ;a(9)
        		DW	74FBH
        		DB	7DH
        		DW	0AEAFH          ;a(7)
        		DW	11FFH
        		DB	7EH
        		DW	0D98CH          ;a(5)
        		DW	4CCDH
        		DB	7EH
        		DW	0A9E3H          ;a(3)
        		DW	2AAAH
        		DB	7FH
        		DW	0               ;a(1)
        		DW	0
        		DB	81H
        		CALL    POP5
        		CALL    POP5
        		CALL    FMUL
        		POP     AF              ;EXPONENT
        		CALL    PUSH5
        		EX      AF,AF'
        		CALL    ZERO_FP
        		EX      AF,AF'
        		SUB     80H
        		JR      Z,LN3
        		JR      NC,LN1
        		CPL
        		INC     A
LN1:			LD      H,A
        		LD      C,87H
        		PUSH    AF
        		CALL    FLOAT_
        		RES     7,H
        		CALL    LN2
        		CALL    FMUL
        		POP     AF
        		JR      NC,LN3
        		JP      M,LN3
        		SET     7,H
LN3:			CALL    POP5
        		CALL    FADD
        		XOR     A
        		RET
;
;LOG - base-10 logarithm.
;Result is floating-point numeric.
;
LOG_FP:			CALL    LN_FP
        		LD      DE,5E5BH        ;LOG(e)
        		EXX
        		LD      DE,0D8A9H
        		EXX
        		LD      B,7EH
        		CALL    FMUL
        		XOR     A
        		RET
;
;ASN - Arc-sine
;Result is floating-point numeric.
;
ASN_FP:			CALL    SFLOAT_FP
        		CALL    PUSH5
        		CALL    COPY_
        		CALL    FMUL
        		CALL    DONE
        		CALL    RSUB
        		CALL    SQR0
        		CALL    POP5
        		INC     C
        		DEC     C
        		LD      A,2
        		PUSH    DE
        		JP      Z,ACS1
        		POP     DE
        		CALL    RDIV
        		JR      ATN0
;
;ATN - arc-tangent
;Result is floating-point numeric.
;
ATN_FP:			CALL    SFLOAT_FP
ATN0:			PUSH    HL              ;SAVE SIGN
        		RES     7,H
        		LD      DE,5413H        ;TAN(PI/8)=SQR(2)-1
        		EXX
        		LD      DE,0CCD0H
        		EXX
        		LD      B,7EH
        		CALL    FCP0            ;COMPARE
        		LD      B,0
        		JR      C,ATN2
        		LD      DE,1A82H        ;TAN(3*PI/8)=SQR(2)+1
        		EXX
        		LD      DE,799AH
        		EXX
        		LD      B,81H
        		CALL    FCP0            ;COMPARE
        		JR      C,ATN1
        		CALL    RECIP           ;X=1/X
        		LD      B,2
        		JP      ATN2
ATN1:			CALL    RATIO           ;X=(X-1)/(X+1)
        		LD      B,1
ATN2:			PUSH    BC              ;SAVE FLAG
        		CALL    PUSH5
        		CALL    SQUARE          ;PUSH X*X
        		CALL    POLY
        		DW	0F335H          ;a(13)
        		DW	37D8H
        		DB	7BH
        		DW	6B91H           ;a(11)
        		DW	0AAB9H
        		DB	7CH
        		DW	41DEH           ;a(9)
        		DW	6197H
        		DB	7CH
        		DW	9D7BH           ;a(7)
        		DW	9237H
        		DB	7DH
        		DW	2A5AH           ;a(5)
        		DW	4CCCH
        		DB	7DH
        		DW	0A95CH          ;a(3)
        		DW	0AAAAH
        		DB	7EH
        		DW	0               ;a(1)
        		DW	0
        		DB	80H
        		CALL    POP5
        		CALL    POP5
        		CALL    FMUL
        		POP     AF
ACS1:			CALL    PIBY4           ;PI/4
        		RRA
        		PUSH    AF
        		CALL    C,FADD
        		POP     AF
        		INC     B
        		RRA
        		CALL    C,RSUB
        		POP     AF
        		OR      A
        		RET     P
        		SET     7,H             ;MAKE NEGATIVE
        		XOR     A
        		RET
;
;ACS - Arc cosine=PI/2-ASN.
;Result is floating point numeric.
;
ACS_FP:			CALL    ASN_FP
        		LD      A,2
        		PUSH    AF
        		JR      ACS1
;
;Function STR - convert numeric value to ASCII string.
;   Inputs: HLH'L'C = integer or floating-point number
;           DE = address at which to store string
;           IX = address of @% format control
;  Outputs: String stored, with NUL terminator
;
;First normalise for decimal output:
;
STR_FP:			CALL    SFLOAT_FP
        		LD      B,0             ;DEFAULT PT. POSITION
        		BIT     7,H             ;NEGATIVE?
        		JR      Z,STR10
        		RES     7,H
        		LD      A,'-'
        		LD      (DE),A          ;STORE SIGN
        		INC     DE
STR10:			XOR     A               ;CLEAR A
        		CP      C
        		JR      Z,STR02          ;ZERO
        		PUSH    DE              ;SAVE TEXT POINTER
        		LD      A,B
STR11:			PUSH    AF              ;SAVE DECIMAL COUNTER
        		LD      A,C             ;BINARY EXPONENT
        		CP      161
        		JR      NC,STR14
        		CP      155
        		JR      NC,STR15
        		CPL
        		CP      225
        		JR      C,STR13
        		LD      A,-8
STR13:			ADD     A,28
        		CALL    POWR10
        		PUSH    AF
        		CALL    FMUL
        		POP     AF
        		LD      B,A
        		POP     AF
        		SUB     B
        		JR      STR11
STR14:			SUB     32
        		CALL    POWR10
        		PUSH    AF
        		CALL    FDIV
        		POP     AF
        		LD      B,A
        		POP     AF
        		ADD     A,B
        		JR      STR11
STR15:			LD      A,9
        		CALL    POWR10          ;10^9
        		CALL    FCP0
        		LD      A,C
        		POP     BC
        		LD      C,A
        		SET     7,H             ;IMPLIED 1
        		CALL    C,X10B          ;X10, DEC B
        		POP     DE              ;RESTORE TEXT POINTER
        		RES     7,C
        		LD      A,0
        		RLA                     ;PUT CARRY IN LSB
;
;At this point decimal normalisation has been done,
;now convert to decimal digits:
;      AHLH'L' = number in normalised integer form
;            B = decimal place adjustment
;            C = binary place adjustment (29-33)
;
STR02:			INC     C
        		EX      AF,AF'          ;SAVE A
        		LD      A,B
        		BIT     1,(IX+2)
        		JR      NZ,STR20
        		XOR     A
        		CP      (IX+1)
        		JR      Z,STR21
        		LD      A,-10
STR20:			ADD     A,(IX+1)        ;SIG. FIG. COUNT
        		OR      A               ;CLEAR CARRY
        		JP      M,STR21
        		XOR     A
STR21:			PUSH    AF
        		EX      AF,AF'          ;RESTORE A
STR22:			CALL    X2              ;RL AHLH'L'
        		ADC     A,A
        		CP      10
        		JR      C,STR23
        		SUB     10
        		EXX
        		INC     L               ;SET RESULT BIT
        		EXX
STR23:			DEC     C
        		JR      NZ,STR22        ;32 TIMES
        		LD      C,A             ;REMAINDER
        		LD      A,H
        		AND     3FH             ;CLEAR OUT JUNK
        		LD      H,A
        		POP     AF
        		JP      P,STR24
        		INC     A
        		JR      NZ,STR26
        		LD      A,4
        		CP      C               ;ROUND UP?
        		LD      A,0
        		JR      STR26
STR24:			PUSH    AF
        		LD      A,C
        		ADC     A,'0'           ;ADD CARRY
        		CP      '0'
        		JR      Z,STR25         ;SUPPRESS ZERO
        		CP      '9'+1
        		CCF
        		JR      NC,STR26
STR25:			EX      (SP),HL
        		BIT     6,L             ;ZERO FLAG
		        EX      (SP),HL
        		JR      NZ,STR27
        		LD      A,'0'
STR26:			INC     A               ;SET +VE
        		DEC     A
        		PUSH    AF              ;PUT ON STACK + CARRY
STR27:			INC     B
        		CALL    TEST_FP            ;IS HLH'L' ZERO?
        		LD      C,32
        		LD      A,0
        		JR      NZ,STR22
        		POP     AF
        		PUSH    AF
        		LD      A,0
        		JR      C,STR22
;
;At this point, the decimal character string is stored
; on the stack. Trailing zeroes are suppressed and may
; need to be replaced.
;B register holds decimal point position.
;Now format number and store as ASCII string:
;
STR3:			EX      DE,HL           ;STRING POINTER
        		LD      C,-1            ;FLAG "E"
        		LD      D,1
        		LD      E,(IX+1)        ;f2
        		BIT     0,(IX+2)
        		JR      NZ,STR34        ;E MODE
        		BIT     1,(IX+2)
        		JR      Z,STR31
        		LD      A,B             ;F MODE
        		OR      A
        		JR      Z,STR30
        		JP      M,STR30
        		LD      D,B
STR30:			LD      A,D
        		ADD     A,(IX+1)
        		LD      E,A
        		CP      11
        		JR      C,STR32
STR31:			LD      A,B             ;G MODE
        		LD      DE,101H
        		OR      A
        		JP      M,STR34
        		JR      Z,STR32
        		LD      A,(IX+1)
        		OR      A
        		JR      NZ,STR3A
        		LD      A,10
STR3A:			CP      B
        		JR      C,STR34
        		LD      D,B
        		LD      E,B
STR32:			LD      A,B
        		ADD     A,129
        		LD      C,A
STR34:			SET     7,D
        		DEC     E
STR35:			LD      A,D
        		CP      C
        		JR      NC,STR33
STR36:			POP     AF
        		JR      Z,STR37
        		JP      P,STR38
STR37:			PUSH    AF
        		INC     E
        		DEC     E
        		JP      M,STR4
STR33:			LD      A,'0'
STR38:			DEC     D
        		JP      PO,STR39
        		LD      (HL),'.'
        		INC     HL
STR39:			LD      (HL),A
        		INC     HL
        		DEC     E
        		JP      P,STR35
        		JR      STR36
;
STR4:			POP     AF
STR40:			INC     C
        		LD      C,L
        		JR      NZ,STR44
        		LD      (HL),'E'        ;EXPONENT
        		INC     HL
        		LD      A,B
        		DEC     A
        		JP      P,STR41
        		LD      (HL),'-'
        		INC     HL
        		NEG
STR41:			LD      (HL),'0'
        		JR      Z,STR47
        		CP      10
        		LD      B,A
        		LD      A,':'
        		JR      C,STR42
        		INC     HL
        		LD      (HL),'0'
STR42:			INC     (HL)
        		CP      (HL)
        		JR      NZ,STR43
        		LD      (HL),'0'
        		DEC     HL
        		INC     (HL)
        		INC     HL
STR43:			DJNZ    STR42
STR47:			INC     HL
STR44:			EX      DE,HL
      			RET
;
;Support subroutines:
;
DLOAD5:			LD      B,(IX+4)
        		EXX
        		LD      E,(IX+0)
        		LD      D,(IX+1)
        		EXX
        		LD      E,(IX+2)
        		LD      D,(IX+3)
        		RET
;
DLOAD5_SPL:		LD      B,(IX+6)
			EXX
			LD	DE, (IX+0)
			EXX
			LD	DE, (IX+3)
			RET
;
;CON_FP - Get unsigned numeric constant from ASCII string.
;   Inputs: ASCII string at (IX).
;  Outputs: Variable-type result in HLH'L'C
;           IX updated (points to delimiter)
;           A7 = 0 (numeric marker)
;
CON_FP:			CALL    ZERO_FP            ;INITIALISE TO ZERO
        		LD      C,0             ;TRUNCATION COUNTER
        		CALL    UINT          ;GET INTEGER PART
        		CP      '.'
        		LD      B,0             ;DECL. PLACE COUNTER
        		CALL    Z,NUMBIX        ;GET FRACTION PART
        		CP      'E'
        		LD      A,0             ;INITIALISE EXPONENT
        		CALL    Z,GETEXP        ;GET EXPONENT
        		BIT     7,H
        		JR      NZ,CON0         ;INTEGER OVERFLOW
        		OR      A
        		JR      NZ,CON0         ;EXPONENT NON-ZERO
        		CP      B
        		JR      NZ,CON0         ;DECIMAL POINT
        		CP      C
        		RET     Z               ;INTEGER
CON0:			SUB     B
        		ADD     A,C
        		LD      C,159
        		CALL    FLOAT_
        		RES     7,H             ;DITCH IMPLIED 1
        		OR      A
        		RET     Z               ;DONE
        		JP      M,CON2          ;NEGATIVE EXPONENT
        		CALL    POWR10
        		CALL    FMUL            ;SCALE
        		XOR     A
        		RET
CON2:			CP      -38
        		JR      C,CON3          ;CAN'T SCALE IN ONE GO
        		NEG
        		CALL    POWR10
        		CALL    FDIV            ;SCALE
        		XOR     A
        		RET
CON3:			PUSH    AF
        		LD      A,38
        		CALL    POWR10
        		CALL    FDIV
        		POP     AF
        		ADD     A,38
        		JR      CON2
;
;GETEXP - Get decimal exponent from string
;     Inputs: ASCII string at (IX)
;             (IX points at 'E')
;             A = initial value
;    Outputs: A = new exponent
;             IX updated.
;   Destroys: A,A',IX,F,F'
;
GETEXP:			PUSH    BC              ;SAVE REGISTERS
        		LD      B,A             ;INITIAL VALUE
        		LD      C,2             ;2 DIGITS MAX
        		INC     IX              ;BUMP PAST 'E'
        		CALL    SIGNQ
        		EX      AF,AF'          ;SAVE EXPONENT SIGN
GETEX1:			CALL    DIGITQ
        		JR      C,GETEX2
        		LD      A,B             ;B=B*10
        		ADD     A,A
        		ADD     A,A
        		ADD     A,B
        		ADD     A,A
        		LD      B,A
        		LD      A,(IX)          ;GET BACK DIGIT
        		INC     IX
        		AND     0FH             ;MASK UNWANTED BITS
        		ADD     A,B             ;ADD IN DIGIT
        		LD      B,A
        		DEC     C
        		JP      P,GETEX1
        		LD      B,100           ;FORCE OVERFLOW
        		JR      GETEX1
GETEX2:			EX      AF,AF'          ;RESTORE SIGN
        		CP      '-'
        		LD      A,B
        		POP     BC              ;RESTORE
        		RET     NZ
        		NEG                     ;NEGATE EXPONENT
        		RET
;
;UINT: Get unsigned integer from string.
;    Inputs: string at (IX)
;            C = truncated digit count
;                (initially zero)
;            B = total digit count
;            HLH'L' = initial value
;   Outputs: HLH'L' = number (binary integer)
;            A = delimiter.
;            B, C & IX updated
;  Destroys: A,B,C,D,E,H,L,B',C',D',E',H',L',IX,F
;
NUMBIX:			INC     IX
UINT:			CALL    DIGITQ
        		RET     C
        		INC     B               ;INCREMENT DIGIT COUNT
        		INC     IX
        		CALL    X10             ;*10 & COPY OLD VALUE
        		JR      C,NUMB1         ;OVERFLOW
        		DEC     C               ;SEE IF TRUNCATED
        		INC     C
        		JR      NZ,NUMB1        ;IMPORTANT!
        		AND     0FH
        		EXX
        		LD      B,0
        		LD      C,A
        		ADD.S   HL,BC           ;ADD IN DIGIT
        		EXX
        		JR      NC,UINT
        		INC.S   HL              ;CARRY
        		LD      A,H
        		OR      L
        		JR      NZ,UINT
NUMB1:			INC     C               ;TRUNCATION COUNTER
        		CALL    SWAP1           ;RESTORE PREVIOUS VALUE
        		JR      UINT
;
;FIX - Fix number to specified exponent value.
;    Inputs: HLH'L'C = +ve non-zero number (floated)
;            A = desired exponent (A>C)
;   Outputs: HLH'L'C = fixed number (unsigned)
;            fraction shifted into B'C'
;            A'F' positive if integer input
;  Destroys: C,H,L,A',B',C',H',L',F,F'
;
FIX:			EX      AF,AF'
        		XOR     A
        		EX      AF,AF'
        		SET     7,H             ;IMPLIED 1
FIX1:			CALL    DIV2
        		CP      C
        		RET     Z
        		JP      NC,FIX1
        		JP      OFLOW
;
;SFIX - Convert to integer if necessary.
;    Input: Variable-type number in HLH'L'C
;   Output: Integer in HLH'L', C=0
; Destroys: A,C,H,L,A',B',C',H',L',F,F'
;
;NEGATE - Negate HLH'L'
;    Destroys: H,L,H',L',F
;
FIX2:			CALL    SWAP_FP
        		CALL    SFIX_FP
        		CALL    SWAP_FP
SFIX_FP:			DEC     C
        		INC     C
        		RET     Z               ;INTEGER/ZERO
        		BIT     7,H             ;SIGN
        		PUSH    AF
        		LD      A,159
        		CALL    FIX
        		POP     AF
        		LD      C,0
        		RET     Z
NEGATE:			OR      A               ;CLEAR CARRY
        		EXX
NEG0:			PUSH    DE
        		EX      DE,HL
        		LD      HL,0
        		SBC.S   HL,DE
        		POP     DE
        		EXX
        		PUSH    DE
        		EX      DE,HL
        		LD      HL,0
        		SBC.S   HL,DE
        		POP     DE
        		RET
;
;NEG - Negate HLH'L'B'C'
;    Also complements A (used in FADD)
;    Destroys: A,H,L,B',C',H',L',F
;
NEG_:			EXX
        		CPL
        		PUSH    HL
        		OR      A               ;CLEAR CARRY
        		LD      HL,0
        		SBC.S   HL,BC
        		LD      B,H
        		LD      C,L
        		POP     HL
        		JR      NEG0
;
;SCALE - Trig scaling.
;MOD48 - 48-bit floating-point "modulus" (remainder).
;   Inputs: HLH'L'C unsigned floating-point dividend
;           DED'E'B'C'B unsigned 48-bit FP divisor
;  Outputs: HLH'L'C floating point remainder (H7=1)
;           E = quotient (bit 7 is sticky)
; Destroys: A,B,C,D,E,H,L,B',C',D',E',H',L',IX,F
;FLO48 - Float unsigned number (48 bits)
;    Input/output in HLH'L'B'C'C
;   Destroys: C,H,L,B',C',H',L',F
;
SCALE:			LD      A,150
        		CP      C
        		LD      A,ACLOST
        		JP      C,ERROR_FP_         ;"Accuracy lost"
        		CALL    PIBY4
        		EXX
        		LD      BC,2169H        ;3.141592653589793238
        		EXX
MOD48:			SET     7,D             ;IMPLIED 1
        		SET     7,H
        		LD      A,C
        		LD      C,0             ;INIT QUOTIENT
        		LD      IX,0
        		PUSH    IX              ;PUT ZERO ON STACK
        		CP      B
        		JR      C,MOD485        ;DIVIDEND<DIVISOR
MOD481:			EXX                     ;CARRY=0 HERE
        		EX      (SP),HL
        		SBC.S   HL,BC
        		EX      (SP),HL
        		SBC.S   HL,DE
        		EXX
        		SBC.S   HL,DE
        		JR      NC,MOD482       ;DIVIDEND>=DIVISOR
        		EXX
        		EX      (SP),HL
        		ADD.S   HL,BC
        		EX      (SP),HL
        		ADC.S   HL,DE
        		EXX
        		ADC.S   HL,DE
MOD482:			CCF
        		RL      C               ;QUOTIENT
        		JR      NC,MOD483
        		SET     7,C             ;STICKY BIT
MOD483:			DEC     A
        		CP      B
        		JR      C,MOD484        ;DIVIDEND<DIVISOR
        		EX      (SP),HL
        		ADD.S   HL,HL           ;DIVIDEND * 2
        		EX      (SP),HL
        		EXX
        		ADC.S   HL,HL
        		EXX
        		ADC.S   HL,HL
        		JR      NC,MOD481       ;AGAIN
        		OR      A
        		EXX
        		EX      (SP),HL
        		SBC.S   HL,BC           ;OVERFLOW, SO SUBTRACT
        		EX      (SP),HL
        		SBC.S   HL,DE
        		EXX
        		SBC.S   HL,DE
        		OR      A
        		JR      MOD482
;
MOD484:			INC     A
MOD485:			LD      E,C             ;QUOTIENT
        		LD      C,A             ;REMAINDER EXPONENT
        		EXX
        		POP     BC
        		EXX
FLO48:			BIT     7,H
        		RET     NZ
        		EXX
        		SLA     C
        		RL      B
        		ADC.S   HL,HL
        		EXX
        		ADC.S   HL,HL
        		DEC     C
        		JP      NZ,FLO48
        		RET
;
;Float unsigned number
;    Input/output in HLH'L'C
;   Destroys: C,H,L,H',L',F
;
FLOAT_:			BIT     7,H
        		RET     NZ
        		EXX                     ;SAME AS "X2"
        		ADD.S   HL,HL           ;TIME-CRITICAL
        		EXX                     ;REGION
        		ADC.S   HL,HL           ;(BENCHMARKS)
        		DEC     C
        		JP      NZ,FLOAT_
        		RET
;
;SFLOAT - Convert to floating-point if necessary.
;    Input: Variable-type number in HLH'L'C
;    Output: Floating-point in HLH'L'C
;    Destroys: A,C,H,L,H',L',F
;
FLOATA:			EX      AF,AF'
        		; ADD     A,(RTABLE-DTABLE)/3
        		ADD     A,RTABLE-DTABLE/3 ; ez80asm doesn't do nested expressions        		
        		EX      AF,AF'
FLOAT2:			CALL    SWAP_FP
        		CALL    SFLOAT_FP
        		CALL    SWAP_FP
SFLOAT_FP:			DEC     C
        		INC     C
        		RET     NZ              ;ALREADY FLOATING-POINT
        		CALL    TEST_FP
        		RET     Z               ;ZERO
        		LD      A,H
        		OR      A
        		CALL    M,NEGATE
        		LD      C,159
        		CALL    FLOAT_
        		OR      A
        		RET     M               ;NEGATIVE
        		RES     7,H
        		RET
;
;ROUND UP
;Return with carry set if 32-bit overflow
;   Destroys: H,L,B',C',H',L',F
;
ADD1_FP:			EXX
        		LD      BC,1
        		ADD.S   HL,BC
        		EXX
        		RET     NC
        		PUSH    BC
        		LD      BC,1
        		ADD.S   HL,BC
        		POP     BC
        		RET
;
;ODD - Add one if even, leave alone if odd.
; (Used to perform unbiassed rounding, i.e.
;  number is rounded up half the time)
;    Destroys: L',F (carry cleared)
;
ODD:			OR      A               ;CLEAR CARRY
        		EXX
        		SET     0,L             ;MAKE ODD
        		EXX
        		RET
;
;SWAP_FP - Swap arguments.
;    Exchanges DE,HL D'E',H'L' and B,C
;    Destroys: A,B,C,D,E,H,L,D',E',H',L'
;SWAP1 - Swap DEHL with D'E'H'L'
;    Destroys: D,E,H,L,D',E',H',L'
;
SWAP_FP:			LD      A,C
        		LD      C,B
        		LD      B,A
SWAP1:			EX      DE,HL
        		EXX
        		EX      DE,HL
        		EXX
        		RET
;
; DIV2 - destroys C,H,L,A',B',C',H',L',F,F'
; INCC - destroys C,F
; OFLOW
;
DIV2:			CALL    D2
        		EXX
        		RR      B
        		RR      C
        		EX      AF,AF'
        		OR      B
        		EX      AF,AF'
        		EXX
INCC:			INC     C
        		RET     NZ
OFLOW:			LD      A,TOOBIG_FP
        		JP      ERROR_FP_           ;"Too big"
;
; FTEST - Test for zero & sign
;     Output: A=0 if zero, A=&40 if +ve, A=&C0 if -ve
;
FTEST_FP:			CALL    TEST_FP
        		RET     Z
        		LD      A,H
        		AND     10000000B
        		OR      01000000B
        		RET
;
; TEST_FP - Test HLH'L' for zero.
;     Output: Z-flag set & A=0 if HLH'L'=0
;     Destroys: A,F
;
TEST_FP:			LD      A,H
        		OR      L
        		EXX
        		OR      H
        		OR      L
        		EXX
        		RET
;
; FCOMP - Compare two numbers
;     Output: A=0 if equal, A=&40 if L>R, A=&C0 if L<R
;
FCOMP_FP:			LD      A,B
        		OR      C               ;Both integer?
        		JR      NZ,FCOMP1
        		CALL    ICP
FCOMP0:			LD      A,0
        		RET     Z               ;Equal
        		LD      A,80H
        		RRA
        		RET
;
FCOMP1:			CALL    FLOAT2          ;Float both
        		CALL    FCP
        		JR      FCOMP0
;
; Integer and floating point compare.
; Sets carry & zero flags according to HLH'L'C-DED'E'B
; Result pre-set to FALSE
; ICP1, FCP1 destroy A,F
;
; ZERO - Return zero.
;  Destroys: A,C,H,L,H',L'
;
ICP:			CALL    ICP1
ZERO_FP:			LD      A,0
        		EXX
        		LD      H,A
	       		LD      L,A
        		EXX
      			LD      H,A
     			LD      L,A
	    		LD      C,A
        		RET
;
FCP:			CALL    FCP1
        		JR      ZERO_FP            ;PRESET FALSE
;
FCP0:			LD      A,C
        		CP      B               ;COMPARE EXPONENTS
        		RET     NZ
ICP0:			
			SBC.S   HL,DE           ;COMP MANTISSA MSB
        		ADD.S   HL,DE
        		RET     NZ
        		EXX
        		SBC.S   HL,DE           ;COMP MANTISSA LSB
        		ADD.S   HL,DE
        		EXX
        		RET
;
FCP1:			LD      A,H
        		XOR     D
        		LD      A,H
        		RLA
        		RET     M
        		JR      NC,FCP0
        		CALL    FCP0
        		RET     Z               ;** V0.1 BUG FIX
        		CCF
        		RET
;
ICP1:			LD      A,H
        		XOR     D
        		JP      P,ICP0
        		LD      A,H
        		RLA
        		RET
;
; ADD - Integer add.
; Carry, sign & zero flags valid on exit
;     Destroys: H,L,H',L',F
;
X10B:			DEC     B
        		INC     C
X5:			CALL    COPY0
        		CALL    D2C
        		CALL    D2C
        		EX      AF,AF'          ;SAVE CARRY
ADD_:			EXX
        		ADD.S   HL,DE
        		EXX
        		ADC.S   HL,DE
        		RET
;
; SUB - Integer subtract.
; Carry, sign & zero flags valid on exit
;     Destroys: H,L,H',L',F
;
SUB_:			EXX
        		OR      A
        		SBC.S   HL,DE
        		EXX
        		SBC.S   HL,DE
        		RET
;
; X10 - unsigned integer * 10
;    Inputs: HLH'L' initial value
;   Outputs: DED'E' = initial HLH'L'
;            Carry bit set if overflow
;            If carry not set HLH'L'=result
;  Destroys: D,E,H,L,D',E',H',L',F
; X2 - Multiply HLH'L' by 2 as 32-bit integer.
;     Carry set if MSB=1 before shift.
;     Sign set if MSB=1 after shift.
;     Destroys: H,L,H',L',F
;
X10:			CALL    COPY0           ;DED'E'=HLH'L'
        		CALL    X2
        		RET     C               ;TOO BIG
        		CALL    X2
        		RET     C
        		CALL    ADD_
        		RET     C
X2:			EXX
        		ADD.S   HL,HL
        		EXX
        		ADC.S   HL,HL
        		RET
;
; D2 - Divide HLH'L' by 2 as 32-bit integer.
;     Carry set if LSB=1 before shift.
;     Destroys: H,L,H',L',F
;
D2C:			INC     C
D2:			SRL     H
        		RR      L
        		EXX
        		RR      H
        		RR      L
        		EXX
        		RET
;
; COPY - COPY HLH'L'C INTO DED'E'B
;   Destroys: B,C,D,E,H,L,D',E',H',L'
;
COPY_:			LD      B,C
COPY0:			LD      D,H
        		LD      E,L
        		EXX
        		LD      D,H
        		LD      E,L
        		EXX
        		RET
;
; SQUARE - PUSH X*X
; PUSH5 - PUSH HLH'L'C ONTO STACK.
;   Destroys: SP,IX
;
SQUARE:			CALL    COPY_
        		CALL    FMUL
PUSH5:			POP     IX              ;RETURN ADDRESS
        		PUSH    BC
        		PUSH    HL
        		EXX
        		PUSH    HL
        		EXX
        		JP      (IX)            ;"RETURN"
;
; POP5 - POP DED'E'B OFF STACK.
;   Destroys: A,B,D,E,D',E',SP,IX
;
POP5:			POP     IX              ;RETURN ADDRESS
        		EXX
        		POP     DE
        		EXX
        		POP     DE
        		LD      A,C
        		POP     BC
        		LD      B,C
        		LD      C,A
        		JP      (IX)            ;"RETURN"
;
; RATIO - Calculate (X-1)/(X+1)
;     Inputs: X in HLH'L'C
;    Outputs: (X-1)/(X+1) in HLH'L'C
;   Destroys: Everything except IY,SP,I
;
RATIO:			CALL    PUSH5           ;SAVE X
        		CALL    DONE
        		CALL    FADD
        		CALL    POP5            ;RESTORE X
        		CALL    PUSH5           ;SAVE X+1
        		CALL    SWAP_FP
        		CALL    DONE
        		CALL    FSUB
        		CALL    POP5            ;RESTORE X+1
        		JP      FDIV
;
; POLY - Evaluate a polynomial.
;     Inputs: X in HLH'L'C and also stored at (SP+2)
;             Polynomial coefficients follow call.
;    Outputs: Result in HLH'L'C
;   Destroys: Everything except IY,SP,I
; Routine terminates on finding a coefficient >=1.
; Note: The last coefficient is EXECUTED on return
;       so must contain only innocuous bytes!
;
POLY:			LD      IX, 3				; Advance the SP to the return address
        		ADD     IX, SP				
        		EX      (SP), IX			; IX: Points to the inline list of coefficients
;		
        		CALL    DLOAD5          		; Load the first coefficient from (IX)
POLY1:			CALL    FMUL
        		LD      DE, 5				; Skip to the next coefficient
        		ADD     IX, DE		
        		CALL    DLOAD5          		; Load the second coefficient from (IX)
        		EX      (SP), IX			; Restore the SP just in case we need to return
        		INC     B		
        		DEC     B               		; Test B for end byte (80h)
        		JP      M,FADD				; Yes, so add and return
        		CALL    FADD				; No, so add
        		CALL    DLOAD5_SPL			; Load X from SP
        		EX      (SP), IX			; IX: Points to the inline list of coefficients
        		JR      POLY1				; And loop
;
; POWR10 - Calculate power of ten.
;     Inputs: A=power of 10 required (A<128)
;             A=binary exponent to be exceeded (A>=128)
;    Outputs: DED'E'B = result
;             A = actual power of ten returned
;   Destroys: A,B,D,E,A',D',E',F,F'
;
POWR10:			INC     A
        		EX      AF,AF'
        		PUSH    HL
        		EXX
        		PUSH    HL
        		EXX
        		CALL    DONE
        		CALL    SWAP_FP
        		XOR     A
POWR11:			EX      AF,AF'
        		DEC     A
        		JR      Z,POWR14        ;EXIT TYPE 1
        		JP      P,POWR13
        		CP      C
        		JR      C,POWR14        ;EXIT TYPE 2
        		INC     A
POWR13:			EX      AF,AF'
        		INC     A
        		SET     7,H
        		CALL    X5
        		JR      NC,POWR12
        		EX      AF,AF'
        		CALL    D2C
        		EX      AF,AF'
POWR12:			EX      AF,AF'
        		CALL    C,ADD1_FP          ;ROUND UP
        		INC     C
        		JP      M,POWR11
        		JP      OFLOW
POWR14:			CALL    SWAP_FP
        		RES     7,D
        		EXX
        		POP     HL
        		EXX
        		POP     HL
        		EX      AF,AF'
        		RET
;
; DIVA, DIVB - DIVISION PRIMITIVE.
;     Function: D'E'DE = H'L'HLD'E'DE / B'C'BC
;               Remainder in H'L'HL
;     Inputs: A = loop counter (normally -32)
;     Destroys: A,D,E,H,L,D',E',H',L',F
;
DIVA:			OR      A               ;CLEAR CARRY
DIV0:			
			SBC.S   HL,BC           ;DIVIDEND-DIVISOR
        		EXX
        		SBC.S   HL,BC
        		EXX
        		JR      NC,DIV1
        		ADD.S   HL,BC           ;DIVIDEND+DIVISOR
        		EXX
        		ADC.S   HL,BC
        		EXX
DIV1:			CCF
DIVC:			RL      E               ;SHIFT RESULT INTO DE
        		RL      D
        		EXX
        		RL      E
        		RL      D
        		EXX
        		INC     A
        		RET     P
DIVB:			
			ADC.S   HL,HL           ;DIVIDEND*2
        		EXX
        		ADC.S   HL,HL
        		EXX
        		JR      NC,DIV0
        		OR      A
        		SBC.S   HL,BC           ;DIVIDEND-DIVISOR
        		EXX
        		SBC.S   HL,BC
        		EXX
        		SCF
        		JP      DIVC
;
;MULA, MULB - MULTIPLICATION PRIMITIVE.
;    Function: H'L'HLD'E'DE = B'C'BC * D'E'DE
;    Inputs: A = loop counter (usually -32)
;            H'L'HL = 0
;    Destroys: D,E,H,L,D',E',H',L',A,F
;
MULA:			OR      A               ;CLEAR CARRY
MUL0:			EXX
        		RR      D               ;MULTIPLIER/2
        		RR      E
        		EXX
        		RR      D
        		RR      E
        		JR      NC,MUL1
        		ADD.S   HL,BC           ;ADD IN MULTIPLICAND
        		EXX
        		ADC.S   HL,BC
        		EXX
MUL1:			INC     A
        		RET     P
MULB:			EXX
        		RR      H               ;PRODUCT/2
        		RR      L
        		EXX
        		RR      H
        		RR      L
        		JP      MUL0
;
; SQRA, SQRB - SQUARE ROOT PRIMITIVES
;     Function: B'C'BC = SQR (D'E'DE)
;     Inputs: A = loop counter (normally -31)
;             B'C'BCH'L'HL initialised to 0
;   Destroys: A,B,C,D,E,H,L,B',C',D',E',H',L',F
;
SQR1:			
			SBC.S   HL,BC
        		EXX
        		SBC.S   HL,BC
        		EXX
        		INC     C
        		JR      NC,SQR2
        		DEC     C
        		ADD.S   HL,BC
        		EXX
        		ADC.S   HL,BC
        		EXX
        		DEC     C
SQR2:			INC     A
        		RET     P
SQRA:			SLA     C
        		RL      B
        		EXX
        		RL      C
        		RL      B
        		EXX
        		INC     C
        		SLA     E
        		RL      D
        		EXX
        		RL      E
        		RL      D
        		EXX
        		ADC.S   HL,HL
        		EXX
        		ADC.S   HL,HL
        		EXX
        		SLA     E
        		RL      D
        		EXX
        		RL      E
        		RL      D
        		EXX
        		ADC.S   HL,HL
        		EXX
        		ADC.S   HL,HL
        		EXX
        		JP      NC,SQR1
SQR3:			OR      A
        		SBC.S   HL,BC
        		EXX
        		SBC.S   HL,BC
        		EXX
        		INC     C
        		JP      SQR2
;
SQRB:			
			ADD.S   HL,HL
        		EXX
        		ADC.S   HL,HL
        		EXX
        		JR      C,SQR3
        		INC     A
        		INC     C
        		SBC.S   HL,BC
        		EXX
        		SBC.S   HL,BC
        		EXX
        		RET     NC
        		ADD.S   HL,BC
        		EXX
        		ADC.S   HL,BC
        		EXX
        		DEC     C
        		RET
;
DIGITQ:			LD      A,(IX)
        		CP      '9'+1
        		CCF
        		RET     C
        		CP      '0'
        		RET
;
SIGNQ:			LD      A,(IX)
        		INC     IX
        		CP      ' '
        		JR      Z,SIGNQ
        		CP      '+'
        		RET     Z
        		CP      '-'
        		RET     Z
        		DEC     IX
        		RET; --- End fpp.asm ---

; --- Begin gpio.asm ---
;
; Title:	BBC Basic for AGON - GPIO functions
; Author:	Dean Belfield
; Created:	12/05/2023
; Last Updated:	12/05/2023
;
; Modinfo:

			; INCLUDE	"macros.inc"
			; INCLUDE	"equs.inc"

			; .ASSUME	ADL = 1

			; SEGMENT CODE
				
			; XDEF	GPIOB_SETMODE
				
			; XREF	SWITCH_A

;  A: Mode
;  B: Pins
;  				
GPIOB_SETMODE:		CALL	SWITCH_A
			DW	GPIOB_M0	; Output
			DW	GPIOB_M1	; Input
			DW	GPIOB_M2	; Open Drain IO
			DW	GPIOB_M3	; Open Source IO
			DW	GPIOB_M4	; Interrupt, Dual Edge
			DW	GPIOB_M5	; Alt Function
			DW	GPIOB_M6	; Interrupt, Active Low
			DW	GPIOB_M7	; Interrupt, Active High
			DW	GPIOB_M8	; Interrupt, Falling Edge
			DW	GPIOB_M9	; Interrupt, Rising Edge

; Output
;
GPIOB_M0:		RES_GPIO PB_DDR,  B
			RES_GPIO PB_ALT1, B
			RES_GPIO PB_ALT2, B
			RET

; Input
;
GPIOB_M1:		SET_GPIO PB_DDR,  B
			RES_GPIO PB_ALT1, B
			RES_GPIO PB_ALT2, B
			RET

; Open Drain IO
;
GPIOB_M2:		RES_GPIO PB_DDR,  B
			SET_GPIO PB_ALT1, B
			RES_GPIO PB_ALT2, B
			RET

; Open Source IO
;
GPIOB_M3:		SET_GPIO PB_DDR,  B
			SET_GPIO PB_ALT1, B
			RES_GPIO PB_ALT2, B
			RET

; Interrupt, Dual Edge
;
GPIOB_M4:		SET_GPIO PB_DR,   B
			RES_GPIO PB_DDR,  B
			RES_GPIO PB_ALT1, B
			RES_GPIO PB_ALT2, B
			RET

; Alt Function
;
GPIOB_M5:		SET_GPIO PB_DDR,  B
			RES_GPIO PB_ALT1, B
			SET_GPIO PB_ALT2, B
			RET

; Interrupt, Active Low
;
GPIOB_M6:		RES_GPIO PB_DR,   B
			RES_GPIO PB_DDR,  B
			SET_GPIO PB_ALT1, B
			SET_GPIO PB_ALT2, B
			RET


; Interrupt, Active High
;
GPIOB_M7:		SET_GPIO PB_DR,   B
			RES_GPIO PB_DDR,  B
			SET_GPIO PB_ALT1, B
			SET_GPIO PB_ALT2, B
			RET


; Interrupt, Falling Edge
;
GPIOB_M8:		RES_GPIO PB_DR,   B
			SET_GPIO PB_DDR,  B
			SET_GPIO PB_ALT1, B
			SET_GPIO PB_ALT2, B
			RET
	
; Interrupt, Rising Edge
;
GPIOB_M9:		SET_GPIO PB_DR,   B
			SET_GPIO PB_DDR,  B
			SET_GPIO PB_ALT1, B
			SET_GPIO PB_ALT2, B
			RET	; --- End gpio.asm ---

; --- Begin main.asm ---
;
; Title:	BBC Basic Interpreter - Z80 version
;		Command, Error and Lexical Analysis Module - "MAIN"
; Author:	(C) Copyright  R.T.Russell  1984
; Modified By:	Dean Belfield
; Created:	12/05/2023
; Last Updated:	26/11/2023
;
; Modinfo:
; 07/05/1984:	Version 2.3
; 01/03/1987:	Version 3.0
; 03/05/2022:	Modified by Dean Belfield
; 06/06/2023:	Modified to run in ADL mode
; 26/06/2023:	Fixed binary and unary indirection
; 17/08/2023:	Added binary constants
; 15/11/2023:	Fixed bug in ONEDIT1 for OSLOAD_TXT, Startup message now includes Agon version
; 26/11/2023:	Fixed bug in AUTOLOAD

			; .ASSUME	ADL = 1

			; INCLUDE	"equs.inc"

			; SEGMENT CODE
			
			; XDEF	_main
			
			; XDEF	COLD
			; XDEF	WARM
			; XDEF	CLOOP
			; XDEF	DELETE
			; XDEF	LIST_
			; XDEF	RENUM
			; XDEF	AUTO
			; XDEF	NEW
			; XDEF	OLD
			; XDEF	LOAD
			; XDEF	SAVE
			; XDEF	ERROR_
			; XDEF	EXTERR
			; XDEF	LOAD0
			; XDEF	CLEAR
			; XDEF	CRLF
			; XDEF	OUTCHR
			; XDEF	OUT_
			; XDEF	FINDL
			; XDEF	SETLIN
			; XDEF	PBCDL
			; XDEF	SAYLN
			; XDEF	PUTVAR
			; XDEF	GETVAR
			; XDEF	GETDEF
			; XDEF	CREATE
			; XDEF	RANGE
			; XDEF	LEXAN2
			; XDEF	REPORT
			; XDEF	TELL
			; XDEF	SPACE_
			; XDEF	KEYWDS
			; XDEF	KEYWDL
			; XDEF	ONEDIT
			; XDEF	ONEDIT1
			; XDEF	LISTIT
			; XDEF	CLEAN
				
			; XREF	LISTON
			; XREF	ERRTXT
			; XREF	OSINIT
			; XREF	HIMEM
			; XREF	PAGE_
			; XREF	CHAIN0
			; XREF	PROMPT
			; XREF	ERRTRP
			; XREF	ERRLIN
			; XREF	AUTONO
			; XREF	LINENO
			; XREF	INCREM
			; XREF	OSLINE
			; XREF	COUNT
			; XREF	NXT
			; XREF	BUFFER
			; XREF	XEQ
			; XREF	TOP
			; XREF	EXPRI
			; XREF	SEARCH
			; XREF	LTRAP
			; XREF	LOMEM
			; XREF	DECODE
			; XREF	EXPRS
			; XREF	OSSAVE
			; XREF	ERR
			; XREF	ERL
			; XREF	TRACEN
			; XREF	RESET
			; XREF	OSSHUT
			; XREF	OSLOAD
			; XREF	FREE
			; XREF	DYNVAR
			; XREF	FILL
			; XREF	OSWRCH
			; XREF	WIDTH
			; XREF	COMMA
			; XREF	MUL16
			; XREF	BRAKET
			; XREF	X4OR5
			; XREF	LOADN
			; XREF	SFIX
			; XREF	ITEMI
			; XREF	FNPTR
			; XREF	PROPTR
			; XREF	CHECK
			; XREF	TERMQ
			; XREF	OSWRCHCH
			; XREF	NEWIT
			; XREF	BAD
			; XREF	RAM_START
			; XREF	RAM_END
			; XREF	R0
			; XREF	STAR_VERSION

			; XREF	_end			; In init.asm			
;
; A handful of common token IDs
;
TERROR_MN:			EQU     85H
LINE_MN_:			EQU     86H
ELSE_MN_:			EQU     8BH
THEN_MN_:			EQU     8CH
LINO_MN:			EQU     8DH
FN:			EQU     A4H
TO_MN:			EQU     B8H
REN:			EQU     CCH
DATA_MN_:			EQU     DCH
DIM:			EQU     DEH
FOR:			EQU     E3H
GOSUB:			EQU     E4H
GOTO:			EQU     E5H
TIF:			EQU     E7H
LOCAL_:			EQU     EAH
NEXT:			EQU     EDH
ON_:			EQU     EEH
PROC:			EQU     F2H
REM:			EQU     F4H
REPEAT:			EQU     F5H
RESTOR:			EQU     F7H
TRACE:			EQU     FCH
UNTIL:			EQU     FDH
;
; This defines the block of tokens that are pseudo-variables.
; There are two versions of each token, a GET and a SET

; Name  : GET : SET
; ------:-----:----
; PTR   : 8Fh : CFh 
; PAGE  : 90h : D0h
; TIME  : 91h : D1h
; LOMEM : 92h : D2h
; HIMEM : 93h : D3h
;
; Examples:
;   LET A% = PAGE : REM This is the GET version
;   PAGE = 40000  : REM This is the SET version
;
TOKLO:			EQU     8FH			; This defines the block of tokens that are pseudo-variables
TOKHI:			EQU     93H			; PTR, PAGE, TIME, LOMEM, HIMEM
OFFSET:			EQU     CFH-TOKLO		; Offset to the parameterised SET versions

; The main routine
; IXU: argv - pointer to array of parameters
;   C: argc - number of parameters
; Returns:
;  HL: Error code, or 0 if OK
;
_main:			LD	HL, ACCS		; Clear the ACCS
			LD	(HL), 0
			LD	A, C			
			CP	2
			JR	Z, AUTOLOAD		; 2 parameters = autoload
			JR	C, COLD			; 1 parameter = normal start
			CALL	STAR_VERSION
			CALL	TELL
			DB	"Usage:\n\r"
			DB	"RUN . <filename>\n\r", 0
			LD	HL, 0			; The error code
			JP	_end
;							
AUTOLOAD:		LD	HL, (IX+3)		; HLU: Address of filename
			LD	DE, ACCS		;  DE: Destination address
AUTOLOAD_1:		LD	A, (HL)			; Fetch the filename byte
			LD	(DE), A			; 
			INC	HL			; Increase the source pointer
			INC	E			; We only need to increase E as ACCS is on a page boundary
			JR	Z, AUTOLOAD_2		; End if we hit the page boundary
			OR	A
			JR	NZ, AUTOLOAD_1		; Loop until we hit a 0 byte
AUTOLOAD_2:		DEC	E
			LD	A, CR
			LD	(DE), A			; Replace the 0 byte with a CR for BBC BASIC
;
COLD:			POP	HL			; Pop the return address to init off SPS
			PUSH	HL 			; Stack it on SPL (*BYE will use this as the return address)
			LD	HL, STAVAR		; Cold start
			LD	SP, HL
			LD	(HL), 10
			INC	HL
			LD	(HL),9
			CALL    OSINIT			; Call the machine specific OS initialisation routines
			LD      (HIMEM),DE		; This returns HIMEM (ramtop) in DE - store in the HIMEM sysvar
			LD      (PAGE_),HL		; And PAGE in HL (where BASIC program storage starts) - store in PAGE sysvar
			LD      A,B7H           	; Set LISTO sysvar; the bottom nibble is LISTO (7), top nibble is OPT (B)
			LD      (LISTON),A		
			LD      HL,NOTICE
			LD      (ERRTXT),HL
			CALL    NEWIT			; From what I can determine, NEWIT always returns with Z flag set
			LD	A,(ACCS)		; Check if there is a filename in ACCS
			OR	A
			JP	NZ,CHAIN0		; Yes, so load and run
			CALL	STAR_VERSION		; 
			CALL    TELL			; Output the welcome message
			DB    	"BBC BASIC (Z80) Version 3.00\n\r"
NOTICE:			DB    	"(C) Copyright R.T.Russell 1987\n\r"
			DB	"\n\r", 0
;			
WARM:			DB 	F6H			; Opcode for OR? Maybe to CCF (the following SCF will be the operand)
;
; This is the main entry point for BASIC
;
CLOOP:			SCF				; See above - not sure why this is here!
			LD      SP,(HIMEM)
			CALL    PROMPT          	; Prompt user
			LD      HL,LISTON		; Pointer to the LISTO/OPT sysvar 
			LD      A,(HL)			; Fetch the value
			AND     0FH             	; Bottom nibble: LISTO
			OR      B0H             	; Top nibble: Default to OPT (3) with ADL mode bit set to 1 for assembler
			LD      (HL),A			; Store back in
			SBC     HL,HL           	; HL: 0
			LD      (ERRTRP),HL		; Clear ERRTRP sysvar 
			LD      (ERRLIN),HL		; Clear ERRLIN sysvar (ON ERROR)
;			
			LD      HL,(AUTONO)		; Get the auto line number
			LD      (LINENO),HL		; Store in line number
			LD      A,H			; If the auto line number is zero then
			OR      L
			JR      Z,NOAUTO		; We're not auto line numbering, so skip the next bit
;
; This section handles auto line numbering
;
			PUSH    HL			; Stack the line number
			CALL    PBCD           	 	; Output the line number
			POP     HL			; Pop the line number back off the stack
;			LD      BC,(INCREM)		; Load BC with Increment - but INCREM is just a byte; C is the value
;			LD      B,0			; So clear B
			LD	BC, 0			; Load BC with Increment
			LD	A,(INCREM)
			LD	C, A
			ADD     HL,BC			; Add the increment to the line number
			JP      C,TOOBIG		; And error if we wrap
			LD      (AUTONO),HL		; Store the new auto line number
			LD      A,' '			; Print a space
			CALL    OUTCHR
;
; This section invokes the line editor
;
NOAUTO:			LD      HL,ACCS			; Storage for the line editor (256 bytes)
			CALL    OSLINE          	; Call the line editor in MOS
ONEDIT:			CALL	ONEDIT1			; Enter the line into memory
			CALL    C,CLEAN			; Set TOP, write out &FFFF end of program marker
			JP      CLOOP			; Jump back to immediate mode
;
; This bit enters the line into memory
; Also called from OSLOAD_TXT
; Returns:
; F: C if a new line has been entered (CLEAN will need to be called)
;
ONEDIT1:		XOR     A			; Entry point after *EDIT
			LD      (COUNT),A
			LD      IY,ACCS
			CALL    LINNUM			; HL: The line number from the input buffer
			CALL    NXT			; Skip spaces
			LD      A,H			; HL: The line number will be 0 for immediate mode or when auto line numbering is used
			OR      L
			JR      Z,LNZERO        	; Skip if there is no line number in the input buffer
			LD      (LINENO),HL		; Otherwise store it
;
; This bit does the lexical analysis and tokenisation
;
LNZERO:			LD	C,1			; Left mode
			LD      DE,BUFFER		; Buffer for tokenised BASIC
			CALL    LEXAN2          	; Lexical analysis on the user input
			LD      (DE),A          	; Terminator
			XOR     A
;			LD      B,A
;			LD      C,E             	; BC: Line length
			LD	BC,0
			LD	C,E			; BC: Line length
			INC     DE
			LD      (DE),A          	; Zero next
			LD      HL,(LINENO)		; Get the line number
			LD      A,H			; Is it zero, i.e. a command with no line number?
			OR      L
			LD      IY,BUFFER       	; Yes, so we're in immediate mode
			JP      Z,XEQ           	; Execute it
;
; This section stores the BASIC line in memory
;
			PUSH    BC
			PUSH    HL
			CALL    SETTOP          	; Set TOP sysvar
			POP     HL
			CALL    FINDL			; Find the address of the line
			CALL    Z,DEL			; Delete the existing line if found
			POP     BC
			LD      A,C			; Check for the line length being zero, i.e.
			OR      A			; the user has just entered a line number in the command line
			RET	Z 	         	; If so, then don't do anything else
			ADD     A,4
			LD      C,A             	; Length inclusive
			PUSH    DE              	; DE: Line number (fetched from the call to FINDL)
			PUSH    BC              	; BC: Line length
			EX      DE,HL			; DE: Address of the line in memory
			LD      HL,(TOP)		; HL: TOP (the first free location after the end of the BASIC program)
			PUSH    HL			; Stack TOP (current TOP value)
			ADD     HL,BC			; Add the line length to HL, the new TOP value
			PUSH    HL			; Stack HL (new TOP value)
			INC     H			; Add 256 to HL
			XOR     A
			SBC     HL,SP			; Check whether HL is in the same page as the current stack pointer
			POP     HL			; Pop HL (new TOP value)
			JP      NC,ERROR_        	; If HL is in the stack page, then error: "No room"
			LD      (TOP),HL		; Store new value of TOP
			EX      (SP),HL			; HL: TOP (current TOP value), top of stack now contains new TOP value
			PUSH    HL			; PUSH current TOP value
			INC     HL			
			OR      A
			SBC     HL,DE			; DE: Address of the line in memory 
			LD      B,H             	; BC: Amount to move
			LD      C,L
			POP     HL			; HL: Destination (current TOP value)
			POP     DE			; DE: Source (new TOP value)
			JR      Z,ATEND			; If current TOP and new TOP are the same, i.e. adding a line at the end, then skip...		
			LDDR                    	; Otherwise, make space for the new line in the program
ATEND:			POP     BC              	; BC: Line length
			POP     DE              	; DE: Line number
			INC     HL			; HL: Destination address
			LD      (HL),C          	; Store length
			INC     HL
			LD      (HL),E          	; Store line number
			INC     HL
			LD      (HL),D
			INC     HL
			LD      DE,BUFFER		; DE: Location of the new, tokenised line
			EX      DE,HL			; HL: Location of the new, tokensied line, DE: Destination address in BASIC program
			DEC     C			; Subtract 3 from the number of bytes to copy to
			DEC     C			; compensate for the 3 bytes stored above (length and line number)
			DEC     C	
			LDIR                    	; Add the line to the BASIC program
			SCF				; To flag we need to call CLEAN
			RET
;
; List of tokens and keywords. If a keyword is followed by 0 then
; it will only match with the keyword followed immediately by
; a delimiter
;
KEYWDS:			DB    80H, "AND"
			DB    94H, "ABS"
			DB    95H, "ACS"
			DB    96H, "ADVAL"
			DB    97H, "ASC"
			DB    98H, "ASN"
			DB    99H, "ATN"
			DB    C6H, "AUTO"
			DB    9AH, "BGET", 0
			DB    D5H, "BPUT", 0
			DB    FBH, "COLOUR"
			DB    FBH, "COLOR"
			DB    D6H, "CALL"
			DB    D7H, "CHAIN"
			DB    BDH, "CHR$"
			DB    D8H, "CLEAR", 0
			DB    D9H, "CLOSE", 0
			DB    DAH, "CLG", 0
			DB    DBH, "CLS", 0
			DB    9BH, "COS"
			DB    9CH, "COUNT", 0
			DB    DCH, "DATA"
			DB    9DH, "DEG"
			DB    DDH, "DEF"
			DB    C7H, "DELETE"
			DB    81H, "DIV"
			DB    DEH, "DIM"
			DB    DFH, "DRAW"
			DB    E1H, "ENDPROC", 0
			DB    E0H, "END", 0
			DB    E2H, "ENVELOPE"
			DB    8BH, "ELSE"
			DB    A0H, "EVAL"
			DB    9EH, "ERL", 0
			DB    85H, "ERROR"
			DB    C5H, "EOF", 0
			DB    82H, "EOR"
			DB    9FH, "ERR", 0
			DB    A1H, "EXP"
			DB    A2H, "EXT", 0
			DB    E3H, "FOR"
			DB    A3H, "FALSE", 0
			DB    A4H, "FN"
			DB    E5H, "GOTO"
			DB    BEH, "GET$"
			DB    A5H, "GET"
			DB    E4H, "GOSUB"
			DB    E6H, "GCOL"
			DB    93H, "HIMEM", 0
			DB    E8H, "INPUT"
			DB    E7H, "IF"
			DB    BFH, "INKEY$"
			DB    A6H, "INKEY"
			DB    A8H, "INT"
			DB    A7H, "INSTR("
			DB    C9H, "LIST"
			DB    86H, "LINE"
			DB    C8H, "LOAD"
			DB    92H, "LOMEM", 0
			DB    EAH, "LOCAL"
			DB    C0H, "LEFT$("
			DB    A9H, "LEN"
			DB    E9H, "LET"
			DB    ABH, "LOG"
			DB    AAH, "LN"
			DB    C1H, "MID$("
			DB    EBH, "MODE"
			DB    83H, "MOD"
			DB    ECH, "MOVE"
			DB    EDH, "NEXT"
			DB    CAH, "NEW", 0
			DB    ACH, "NOT"
			DB    CBH, "OLD", 0
			DB    EEH, "ON"
			DB    87H, "OFF"
			DB    84H, "OR"
			DB    8EH, "OPENIN"
			DB    AEH, "OPENOUT"
			DB    ADH, "OPENUP"
			DB    FFH, "OSCLI"
			DB    F1H, "PRINT"
			DB    90H, "PAGE", 0
			DB    8FH, "PTR", 0
			DB    AFH, "PI", 0
			DB    F0H, "PLOT"
			DB    B0H, "POINT("
			DB    F2H, "PROC"
			DB    B1H, "POS", 0
			DB    CEH, "PUT"
			DB    F8H, "RETURN", 0
			DB    F5H, "REPEAT"
			DB    F6H, "REPORT", 0
			DB    F3H, "READ"
			DB    F4H, "REM"
			DB    F9H, "RUN", 0
			DB    B2H, "RAD"
			DB    F7H, "RESTORE"
			DB    C2H, "RIGHT$("
			DB    B3H, "RND", 0
			DB    CCH, "RENUMBER"
			DB    88H, "STEP"
			DB    CDH, "SAVE"
			DB    B4H, "SGN"
			DB    B5H, "SIN"
			DB    B6H, "SQR"
			DB    89H, "SPC"
			DB    C3H, "STR$"
			DB    C4H, "STRING$("
			DB    D4H, "SOUND"
			DB    FAH, "STOP", 0
			DB    B7H, "TAN"
			DB    8CH, "THEN"
			DB    B8H, "TO"
			DB    8AH, "TAB("
			DB    FCH, "TRACE"
			DB    91H, "TIME", 0
			DB    B9H, "TRUE", 0
			DB    FDH, "UNTIL"
			DB    BAH, "USR"
			DB    EFH, "VDU"
			DB    BBH, "VAL"
			DB    BCH, "VPOS", 0
			DB    FEH, "WIDTH"
			DB    D3H, "HIMEM"
			DB    D2H, "LOMEM"
			DB    D0H, "PAGE"
			DB    CFH, "PTR"
			DB    D1H, "TIME"
;
; These are indexed from the ERRWDS table
;
			DB    01H, "Missing "
			DB    02H, "No such "
			DB    03H, "Bad "
			DB    04H, " range"
			DB    05H, "variable"
			DB    06H, "Out of"
			DB    07H, "No "
			DB    08H, " space"

KEYWDL:			EQU     $-KEYWDS
			DW    -1
;
; Error messages
;
ERRWDS:			DB    7, "room", 0		;  0: No room
			DB    6, 4, 0			;  1: Out of range
			DB    0				;  2: *
			DB    0				;  3: *
			DB    "Mistake", 0		;  4: Mistake
			DB    1, ",", 0			;  5: Missing ,
			DB    "Type mismatch", 0	;  6: Type mismatch
			DB    7, FN, 0			;  7: No FN
			DB    0				;  8: *
			DB    1, 34, 0			;  9: Missing "
			DB    3, DIM, 0			; 10: Bad DIM
			DB    DIM, 8, 0			; 11: DIM space
			DB    "Not ", LOCAL_, 0		; 12: Not LOCAL
			DB    7, PROC, 0		; 13: No PROC
			DB    "Array", 0		; 14: Array
			DB    "Subscript", 0		; 15: Subscript
			DB    "Syntax error", 0		; 16: Syntax error
			DB    "Escape", 0		; 17: Escape
			DB    "Division by zero", 0	; 18: Division by zero
			DB    "String too long", 0	; 19: String too long
			DB    "Too big", 0		; 20: Too big
			DB    "-ve root", 0		; 21: -ve root
			DB    "Log", 4, 0		; 22: Log range
			DB    "Accuracy lost", 0	; 23: Accuracy lost
			DB    "Exp", 4, 0		; 24: Exp range
			DB    0				; 25: *
			DB    2, 5, 0			; 26: No such variable
			DB    1, ")", 0			; 27: Missing )
			DB    3, "HEX", 0		; 28: Bad HEX
			DB    2, FN, "/", PROC, 0	; 29: No such FN/PROC
			DB    3, "call", 0		; 30: Bad call
			DB    "Arguments", 0		; 31: Arguments
			DB    7, FOR, 0			; 32: No FOR
			DB    "Can't match ", FOR, 0	; 33: Can't match FOR
			DB    FOR, " ", 5, 0		; 34: FOR variable
			DB    0				; 35: *
			DB    7, TO_MN, 0			; 36: No TO
			DB    0				; 37: *
			DB    7, GOSUB, 0		; 38: No GOSUB
			DB    ON_, " syntax", 0		; 39: ON syntax
			DB    ON_, 4, 0			; 40: ON range
			DB    2, "line", 0		; 41: No such line
			DB    6, " ", DATA_MN_, 0		; 42: Out of DATA
			DB    7, REPEAT, 0		; 43: No REPEAT
			DB    0				; 44: *
			DB    1, "#", 0			; 45: Missing #
;
; COMMANDS:
;
; DELETE line,line
;
DELETE:			CALL    SETTOP          	; Set TOP sysvar (first free byte at end of BASIC program)
			CALL    DLPAIR			; Get the line number pair - HL: BASIC program address, BC: second number (or 0 if missing)
DELET1:			LD      A,(HL)			; Check whether it's the last line
			OR      A			
			JP      Z,WARMNC		; Yes, so do nothing
			INC     HL			; Skip the line length byte
			LD	DE, 0			; Clear DE
			LD      E,(HL)			; Fetch the line number in DE
			INC     HL
			LD      D,(HL)
			LD      A,D			; If the line number is zero then
			OR      E
			JR      Z,CLOOP1        	; Do nothing
			DEC     HL			; Decrement BASIC program pointer back to length
			DEC     HL
			EX      DE,HL			; Check if we've gone past the terminating line
			SCF
			SBC     HL,BC
			EX      DE,HL
			JR      NC,WARMNC		; Yes, so exit back to BASIC prompt
			PUSH    BC			
			CALL    DEL			; Delete the line pointed to by HL
			POP     BC
			JR      DELET1			; And loop round to the next line
;
; LISTO expr
;
LISTO:			INC     IY              	; Skip "O" byte
			CALL    EXPRI			; Get expr
			EXX
			LD      A,L
			LD      (LISTON),A		; Store in LISTON sysvar
CLOOP1:			JP      CLOOP
;
; LIST
; LIST line
; LIST line,line [IF string]
; LIST ,line
; LIST line,
;
LIST_:			CP      'O'			; Check for O (LISTO)
			JR      Z,LISTO			; and jump to LISTO if zero
			CALL    DLPAIR			; Get the line number pair - HL: BASIC program address, BC: second number (or 0 if missing)
			CALL    NXT			; Skip space
			CP      TIF             	; Check for IF clause (token IF)
			LD      A,0             	; Initialise the IF clause string length
			JR      NZ,LISTB		; If there is no IF clause, skip the next bit
;
			INC     IY              	; Skip the IF token
			CALL    NXT             	; And skip any spaces
			EX      DE,HL			; DE: Address in memory
			PUSH    IY			; LD IY, HL
			POP     HL              	; HL is now the address of the tokenised line
			LD      A,CR			
			PUSH    BC			; Stack the second line number arg
			LD      BC,256
			CPIR                    	; Locate CR byte
			LD      A,C
			CPL                    	 	; A: Substring length (of IF clause)
			POP     BC			; Restore the second line number arg
			EX      DE,HL			; HL: Address in memory
;
LISTB:			LD      E,A             	; E: IF clause string length
			LD      A,B			; Check whether a second line number was passed (BC!=0)
			OR      C
			JR      NZ,LISTA		; If there isn't a second line number
			DEC     BC			; then we set it to the maximum of 65535
;
LISTA:			EXX
			LD      IX,LISTON		; IX : Pointer to the LISTON (LISTO and OPT) sysvar
			LD      BC,0            	; BC': Indentation counter (C: FOR/NEXT, B: REPEAT/UNTIL)
			EXX
			LD      A,20			; Number of lines to list
;
LISTC:			PUSH    BC              	; Save second line number
			PUSH    DE              	; Save IF clause length
			PUSH    HL              	; Save BASIC program counter
			EX      AF,AF'
;
; BBC BASIC for Z80 lines are stored as follows:
;
; - [LEN] [LSB] [MSB] [DATA...] [0x0D]: LSB, MSB = line number
; - [&00] [&FF] [&FF]: End of program marker
;
; This is the Russell format and different to the Wilson/Acorn format: https://www.beebwiki.mdfs.net/Program_format
;
			LD      A,(HL)			; Check for end of program marker
			OR      A			; If found
			JR      Z,WARMNC		; Jump to WARMNC (F=NC, so will jump to WARM)
;
; Check if past terminating line number
;
			LD      A,E             	; A: IF clause length
			INC     HL			; Skip the length byte	
			LD	DE,0			; Clear DE
			LD      E,(HL)			; Fetch the line number in DE
			INC     HL
			LD      D,(HL)          
			DEC     HL			; Step HL back to the length byte	
			DEC     HL
			PUSH    DE             	 	; Push the line number on the stack
			EX      DE,HL			; HL: line number
			SCF				; Do a 16-bit compare of HL and DE
			SBC     HL,BC
			EX      DE,HL
			POP     DE              	; Restore the line number
WARMNC:			JP      NC,WARM			; If exceeded the terminating line number then jump to WARM
			LD      C,(HL)          	; C: Line length + 4
			LD      B,A             	; B: IF clause length
;
; Check if "UNLISTABLE":
;
			LD      A,D			; TODO: What is "UNLISTABLE?"
			OR      E
			JP      Z,CLOOP
;
; Check for IF clause:
;
			INC     HL			; Skip the length
			INC     HL			; Skip the line number
			INC     HL              	; HL: Address of the tokenised BASIC line
			DEC     C			;  C: Line length
			DEC     C
			DEC     C
			DEC     C              	
			PUSH    DE              	; Save the line number
			PUSH    HL              	; Save the BASIC program address
			XOR     A               	;
			CP      B              	 	; Check for an IF clause (B!=0)
			PUSH    IY			; LD IY, DE
			POP     DE              	; DE: Address of the IF clause string in the input buffer
			CALL    NZ,SEARCH      		; If there is an IF clause (B!=0) then search for it
			POP     HL              	; Restore BASIC program address
			POP     DE              	; Restore line number
			PUSH    IY			
			CALL    Z,LISTIT        	; List if no IF clause OR there is an IF clause match
			POP     IY
;
			EX      AF,AF'
			DEC     A			; Decrement line list counter
			CALL    LTRAP			; TODO: This destroys A - is this a bug I've introduced in LTRAP?
			POP     HL             	 	; Restore BASIC program address to beginning of line
			LD	DE,0
			LD      E,(HL)			; Fetch the length of line in DE
			ADD     HL,DE           	; Go to the next line
			POP     DE              	; Restore IF clause length
			POP     BC              	; Restore second line number
			JR      LISTC			; Loop back to do next line
;
; RENUMBER
; RENUMBER start
; RENUMBER start,increment
; RENUMBER ,increment
;
RENUM:			CALL    CLEAR           	; Uses the heap so clear all dynamic variables and function/procedure pointers
			CALL    PAIR            	; Fetch the parameters - HL: start (NEW line number), BC: increment
			EXX
			LD      HL,(PAGE_)		; HL: Top of program
			LD      DE,(LOMEM)		; DE: Start address of the heap
;
; Build the table
;
RENUM1:			LD      A,(HL)          	; Fetch the line length byte
			OR      A			; Is it zero, i.e. the end of program marker?
			JR      Z,RENUM2		; Yes, so skip to the next part
			INC     HL
			LD      C,(HL)          	; BC: The OLD line number
			INC     HL
			LD      B,(HL)
			LD      A,B			; Check whether the line number is zero - we only need to check the LSW
			OR      C
			JP      Z,CLOOP        		; If the line number is zero, then exit back to the command line
			EX      DE,HL			; DE: Pointer to BASIC program, HL: Pointer to heap
			LD      (HL),C			; Store the OLD line number in the heap
			INC     HL
			LD      (HL),B
			INC     HL
			EXX				; HL: line number, BC: increment (16-bit values)
			PUSH    HL			; HL: Stack the NEW line number value
			ADD.S   HL,BC           	; Add the increment
			JP      C,TOOBIG        	; If > 65535, then error: "Too big"
			EXX				; DE: Pointer to BASIC program, HL: Pointer to heap
			POP     BC			; BC: Pop the NEW line number value off the stack
			LD      (HL),C			; Store the NEW line number in the heap
			INC     HL
			LD      (HL),B
			INC     HL
			EX      DE,HL			; HL: Pointer to BASIC program, DE: Pointer to heap
			DEC     HL			; Back up to the line length byte
			DEC     HL
			LD	BC, 0
			LD      C,(HL)			; BC: Line length
			ADD	HL,BC           	; Advance HL to next line
			EX      DE,HL			; DE: Pointer to BASIC program, HL: Pointer to heap
			PUSH    HL
			INC     H			; Increment to next page
			SBC     HL,SP			; Subtract from SP
			POP     HL			
			EX      DE, HL			; HL: Pointer to BASIC program, DE: Pointer to heap
			JR      C,RENUM1        	; Loop, as the heap pointer has not strayed into the stack page
			CALL    EXTERR          	; Otherwise throw error: "RENUMBER space'
			DB    	REN
			DB    	8
			DB    	0
;
; At this point a list of BASIC line numbers have been written to the heap
; as word pairs:
; - DW: The OLD line number
; - DW: The NEW line number
;
RENUM2:			EX      DE,HL			; HL: Pointer to the end of the heap
			LD      (HL),-1			; Mark the end with FFFFh
			INC     HL
			LD      (HL),-1
			LD      DE,(LOMEM)		; DE: Pointer to the start of the heap
			EXX				
			LD      HL,(PAGE_)		; HL: Start of the BASIC program area
RENUM3:			LD      C,(HL)			; Fetch the first line length byte
			LD      A,C			; If it is zero, then no program, so...
			OR      A
			JP      Z,WARM			; Jump to warm start
			EXX				; HL: Pointer to end of heap, DE: Pointer to start of heap
			EX      DE,HL			; DE: Pointer to end of heap, HL: Pointer to start of heap
			INC     HL			; Skip to the NEW line number	
			INC     HL
			LD      E,(HL)			; DE: The NEW line number
			INC     HL
			LD      D,(HL)
			INC     HL
			PUSH    DE			; Stack the NEW line number
			EX      DE,HL			; HL: The NEW line number, DE: Pointer to the end of heap
			LD      (LINENO),HL		; Store the line number in LINENO
			EXX				; HL: Pointer to the BASIC program area
			POP     DE			; DE: The NEW line number
			INC     HL
			LD      (HL),E          	; Write out the NEW line number to the BASIC program
			INC     HL
			LD      (HL),D
			INC     HL
			DEC     C			; Subtract 3 from the line length to compensate for increasing HL by 3 above
			DEC     C
			DEC     C
			LD	A,C
			LD	BC,0
			LD	C,A			; BC: Line length
;
RENUM7:			LD      A,LINO_MN			; A: The token code that precedes any line number encoded in BASIC (i.e. GOTO/GOSUB)
			CPIR                    	; Search for the token
			JR      NZ,RENUM3		; If not found, then loop to process the next line
;
; Having established this line contains at least one encoded line number, we need to update it to point to the new line number
;
			PUSH    BC			; Stack everything
			PUSH    HL
			PUSH    HL			; HL: Pointer to encoded line number
			POP     IY			; IY: Pointer to encoded line number
			EXX				 
			CALL    DECODE			; Decode the encoded line number (in HL')
			EXX				; HL: Decoded line number
			LD      B,H			; BC: Decoded line number
			LD      C,L
			LD      HL,(LOMEM)		; HL: Pointer to heap
;
; This section of code cross-references the decoded (OLD) line number with the list
; created previously in the global heap
;
RENUM4:			LD      E,(HL)          	; DE: The OLD line number
			INC     HL
			LD      D,(HL)
			INC     HL
			EX      DE,HL			; HL: The OLD line number, DE: Pointer in the global heap
			OR      A               	; Clear the carry and...
			SBC.S   HL,BC			; Compare by means of subtraction the OLD line number against the one in the heap
			EX      DE,HL			; HL: Pointer in the global heap
			LD      E,(HL)          	; DE: The NEW line number
			INC     HL
			LD      D,(HL)
			INC     HL
			JR      C,RENUM4		; Loop until there is a match (Z) or not (NC)
			EX      DE,HL			; DE: Pointer in the global heap
			JR      Z,RENUM5        	; If Z flag is set, there is an exact match to the decoded line number on the heap
;
			CALL    TELL			; Display this error if the line number is not found
			DB    	"Failed at "
			DB    	0
			LD      HL,(LINENO)
			CALL    PBCDL
			CALL    CRLF
			JR      RENUM6			; And carry on renumbering
;
; This snippet re-encodes the line number in the BASIC program
;
RENUM5:			POP     DE			; DE: Pointer to the encoded line number in the listing
			PUSH    DE
			DEC     DE			; Back up a byte to the LINO token
			CALL    ENCODE          	; Re-write the new line number out
RENUM6:			POP     HL			; HL: Pointer to the encoded line number in the listing
			POP     BC			; BC: The remaining line length
			JR      RENUM7			; Carry on checking for any more encoded line numbers in this line
;
; AUTO
; AUTO start,increment
; AUTO start
; AUTO ,increment
;
AUTO:			CALL    PAIR			; Get the parameter pair (HL: first parameter, BC: second parameter)
			LD      (AUTONO),HL		; Store the start in AUTONO
			LD      A,C			; Increment is 8 bit (0-255)
			LD      (INCREM),A		; Store that in INCREM
			JR      CLOOP0			; Jump back indirectly to the command loop via CLOOP0 (optimisation for size)
;
; BAD
; NEW
;
BAD:			CALL    TELL            	; Output "Bad program" error
			DB    3				; Token for "BAD"
			DB    "program"
			DB    CR
			DB    LF
			DB    0				; Falls through to NEW	
;
NEW:			CALL    NEWIT			; Call NEWIT (clears program area and variables)
			JR      CLOOP0			; Jump back indirectly to the command loop via CLOOP0 (optimisation for size)	
;
; OLD
;
OLD:			LD      HL,(PAGE_)		; HL: The start of the BASIC program area
			PUSH    HL			; Stack it
			INC     HL			; Skip the potential length byte of first line of code
			INC     HL			; And the line number word
			INC     HL
			LD      BC,252			; Look for a CR in the first 252 bytes of code; maximum line length
			LD      A,CR
			CPIR
			JR      NZ,BAD			; If not found, then the first line of code is not a valid BBC BASIC code
			LD      A,L			; It could still be garbage though! Store the position in A; this requires
			POP     HL			; PAGE to be on a 256 page boundary, and is now the length of the first line
			LD      (HL),A			; Restore the length byte (this will have been set to 0 by NEW)
			CALL    CLEAN			; Further checks for bad program, set TOP, write out &FFFF end of program marker
CLOOP0:			JP      CLOOP			; Jump back to the command loop
;
; LOAD filename
;
LOAD:			CALL    EXPRS           	; Get the filename
			LD      A,CR			; DE points to the last byte of filename in ACCS
			LD      (DE),A			; Terminate filename with a CR
			CALL    LOAD0			; Load the file in, then CLEAN
			CALL    CLEAR			; Further checks for bad program, set TOP, write out &FFFF end of program marker
			JR      WARM0			; Jump back to the command loop
;
; SAVE filename
;
SAVE:			CALL    SETTOP          	; Set TOP sysvar
			CALL    EXPRS           	; Get the filename
			LD      A,CR			; Terminate the filename with a CR
			LD      (DE),A
			LD      DE,(PAGE_)		; DE: Start of program memory
			LD      HL,(TOP)		; HL: Top of program memory
			OR      A			; Calculate program size (TOP-PAGE)
			SBC     HL,DE
			LD      B,H             	; BC: Length of program in bytes
			LD      C,L
			LD      HL,ACCS			; HL: Address of the filename
			CALL    OSSAVE			; Call the SAVE routine in patch.asm
WARM0:			JP      WARM			; Jump back to the command loop

;
; ERROR
; Called whenever BASIC needs to halt with an error
; Error messages are indexed from 0
; Inputs:
;  A: Error number
;
ERROR_:			LD      SP,(HIMEM)		; Set SP to HIMEM
			LD      HL,ERRWDS		; Index into the error string table
			OR      A			; We don't need to search for the first error
			JR      Z,ERROR1		; So skip the search routine
;
; Search the error table for error #A
; HL will end up being the pointer into the correct error
; There is no bounds checking on this, so invalid error numbers will probably output garbage
;
			LD      B,A             	; Store error number in B
			EX      AF,AF'			; Store error number in AF'
			XOR     A
ERROR0:			CP      (HL)			; Compare the character with 0 (the terminator byte)
			INC     HL			; Increment the string pointer
			JR      NZ,ERROR0		; Loop until with hit a 0
			DJNZ    ERROR0			; Decrements the error number and loop until 0
			EX      AF,AF'			; Restore the error number from AF'
;
; At this point HL points to the tokenised error string
;
ERROR1:			PUSH    HL			; Stack the error string pointer and fall through to EXTERR

; 
; EXTERR
; Inputs:
;  A: Error number
;
; This is the entry point for external errors, i.e. ones not in the ERRWDS table
; The error text immediately follows the CALL to EXTERR, for example:
; > CALL  EXTERR
; > DB    "Silly", 0
; So we can get the address of the string by popping the return address off the stack
;			
EXTERR:			POP     HL			; Pop the error string pointer
			LD      (ERRTXT),HL		; Store in ERRTXT sysvar
			LD      SP,(HIMEM)		; Set SP to HIMEM
			LD      (ERR),A			; Store error number in ERR sysvar
			CALL    SETLIN			; Get line number
			LD      (ERL),HL		; Store in ERL sysvar
			OR      A			; Is error number 0?
			JR      Z,ERROR2		; Yes, so skip the next bit as error number 0 is untrappable
;
			LD      HL,(ERRTRP)		; Check whether the error is trapped
			LD      A,H
			OR      L
			PUSH    HL			; HL: Error line
			POP     IY			; IY: HL
			JP      NZ,XEQ         	 	; If error trapped, jump to XEQ
;
ERROR2:			LD      HL,0
			LD      (AUTONO),HL		; Cancel AUTO
			LD      (TRACEN),HL     	; Cancel TRACE
			CALL    RESET           	; Reset OPSYS
			CALL    CRLF			; Output newline
			CALL    REPORT          	; Output the error message
			CALL    SAYLN			; Output " at line nnnn" message.
			LD      E,0			; Close all files
			CALL    C,OSSHUT        	
			CALL    CRLF			; Output newline
			JP      CLOOP			; Back to CLOOP
;
; SUBROUTINES:
;
; LEX - SEARCH FOR KEYWORDS
;   Inputs: HL = start of keyword table
;           IY = start of match text
;  Outputs: If found, Z-flag set, A=token.
;           If not found, Z-flag reset, A=(IY).
;           IY updated (if NZ, IY unchanged).
; Destroys: A,B,H,L,IY,F
;
LEX:			LD      HL,KEYWDS		; Address of the keywords table
;
LEX0:			LD      A,(IY)			; Fetch the character to match
			LD      B,(HL)			; B: The token from the keywords table
			INC     HL			; Increment the pointer in the keywords table
			CP      (HL)			; Compare the first characters
			JR      Z,LEX2			; If there is a match, then skip to LEX2
			RET     C               	; No match, so fail
;
; This snippet of code skips to the next token in the KEYWDS table
;
LEX1:			INC     HL			; Increment the pointer
			BIT     7,(HL)			; Check if bit 7 set (all token IDs have bit 7 set)
			JR      Z,LEX1			; No, so loop
			JR      LEX0			; At this point HL is pointing to the start of the next keyword
;
LEX2:			PUSH    IY              	; Save the input pointer
LEX3:			INC     HL			; Increment the keyword pointer
			BIT     7,(HL)			; If we've reached the end (marked by the start of the next token) then
			JR      NZ,LEX6         	; Jump to here as we've found a token
			INC     IY			; Increment the text pointer
			LD      A,(IY)			; Fetch the character
			CP      '.'			; Is it an abbreviated keyword?
			JR      Z,LEX6          	; Yes, so we'll return with the token we've found
			CP      (HL)			; Compare with the keywords list
			JR      Z,LEX3			; It's a match, so continue checking this keyword
			CALL    RANGE1			; Is it alphanumeric, '@', '_' or '`'
			JR      C,LEX5			; No, so check whether keyword needs to be immediately delimited
;	
LEX4:			POP     IY              	; Restore the input pointer ready for the next search
			JR      LEX1			; And loop back to start again
;
; This section handles the 0 byte at the end of keywords that indicate the keyword needs to be
; immediately delimited
;
LEX5:			LD      A,(HL)			; Fetch the byte from the keywords table	
			OR      A			; If it is not zero, then...
			JR      NZ,LEX4			; Keep searching
			DEC     IY			; If it is zero, then skip the input pointer back one byte
;
; We've found a token at this point
;
LEX6:			POP     AF			; Discard IY input pointer pushed on the stack
			XOR     A			; Set the Z flag
			LD      A,B			; A: The token
			RET
;
; DEL - DELETE A PROGRAM LINE.
;   Inputs: HL addresses program line.
; Destroys: B,C,F
;
; This simply erases the line by moving all of the code after the line to be deleted back over
; it using an LDIR
;
DEL:			PUSH    DE
			PUSH    HL
			PUSH    HL			; HL: Address of the program line
			LD      B,0			; BC: Length of the line
			LD      C,(HL)
			ADD     HL,BC			; HL: Advanced to the start of the next line
			PUSH    HL
			EX      DE,HL			; DE: Pointer to the next line
			LD      HL,(TOP)		; HL: Pointer to the end of the program
			SBC     HL,DE			
			LD      B,H			; BC: Size of block to move
			LD      C,L
			POP     HL			; HL: Pointer to next line
			POP     DE			; DE: Pointer to this line
			LDIR                    	; Delete the line
			LD      (TOP),DE		; Adjust TOP
			POP     HL
			POP     DE
			RET
;
;LOAD0 - LOAD A DISK FILE THEN CLEAN.
;   Inputs: Filename in ACCS (term CR)
; Destroys: A,B,C,D,E,H,L,F
;
;CLEAN - CHECK FOR BAD PROGRAM, FIND END OF TEXT
; AND WRITE FF FF, THEN LOAD (TOP).
; Destroys: A,B,C,H,L,F
;
LOAD0: 			LD      DE,(PAGE_)		; DE: Beginning of BASIC program area
			LD      HL,-256			
			ADD     HL,SP			
			SBC     HL,DE           	; Find available space
			LD      B,H
			LD      C,L
			LD      HL,ACCS
			CALL    OSLOAD          	; Call the OSLOAD function in patch
			CALL    NC,NEWIT		; If NC then NEW
			LD      A,0
			JP      NC,ERROR_        	; And trigger a "No room" error, otherwise...
;							
CLEAN:			CALL    SETTOP			; Set TOP sysvar
			DEC     HL			; Write out the end of program markers
			LD      (HL),-1         	
			DEC     HL
			LD      (HL),-1
			JR      CLEAR			; Clear all dynamic variables and function/procedure pointers
;
; Set the TOP sysvar; the first free location after the end of the current program
; Returns:
; - HL: TOP
;
SETTOP:			LD      HL,(PAGE_)		; Start at beginning of BASIC program area
			LD	BC, 0			; BC: 0
			LD      A,CR			; End of line marker
SETOP1:			LD      C,(HL)			; BC: Get first byte of program line (line length)
			INC     C			; Check for zero
			DEC     C
			JR      Z,SETOP2		; If it is zero, we've reached the end
			ADD     HL,BC			; Skip to next line 
			DEC     HL			; Check end of previous line
			CP      (HL)
			INC     HL
			JR      Z,SETOP1		; If CR then loop
			JP      BAD			; If anything else, then something has gone wrong - trip a Bad Program error
;
SETOP2:			INC     HL             		; Skip the 3 byte end of program marker (&00, &FF, &FF)
			INC     HL			; NB: Called from NEWIT
			INC     HL
			LD      (TOP),HL		; Store in TOP sysvar
			RET
;
; NEWIT - NEW PROGRAM THEN CLEAR
;   Destroys: H,L
;
; CLEAR - CLEAR ALL DYNAMIC VARIABLES INCLUDING
; FUNCTION AND PROCEDURE POINTERS.
;   Destroys: Nothing
;
NEWIT:			LD      HL,(PAGE_)		; HL: First byte of BASIC program area
			LD      (HL),0			; Stick a 0 in there
			CALL    SETOP2			; Skip three bytes to get to end of empty BASIC program area and set TOP sysvar
;
CLEAR:			PUSH    HL			; Stack the BASIC program pointer
			LD      HL,(TOP)		; Get the TOP sysvar - first available byte after BASIC
			LD      (LOMEM),HL		; Set the LOMEM sysvar
			LD      (FREE),HL		; And the FREE sysvar with that value
			LD      HL,DYNVAR		; Get the pointer to the dynamic variable pointers buffer in RAM
			PUSH    BC			
			; LD      B,3*(54+2)		; Loop counter
			LD      B,54+2*3		; ez80asm doesn't do () in expressions
CLEAR1:			LD      (HL),0			; Clear the dynamic variable pointers
			INC     HL
			DJNZ    CLEAR1
			POP     BC
			POP     HL			; Restore the BASIC program pointer
			RET
;
;LISTIT - LIST A PROGRAM LINE.
;    Inputs: HL addresses line
;            DE = line number (binary)
;            IX = Pointer to LISTON
;             B = FOR/NEXT indent level
;             C = REPEAT/UNTIL indent level 
;  Destroys: A,D,E,B',C',D',E',H',L',IY,F
;
LISTIT:			PUSH    HL			; Stack the address of the line
			EX      DE,HL			; HL: Line number
			PUSH    BC
			CALL    PBCD			; Print the line number
			POP     BC
			POP     HL			; HL: Address of the first token/character
			LD      A,(HL)			; Fetch the token
			CP      NEXT			; Is it NEXT...
			CALL    Z,INDENT		; Yes, so indent in
			CP      UNTIL			; Or is it UNTIL...
			CALL    Z,INDENT		; Yes, so indent in
			EXX
			LD      A,' '
			BIT     0,(IX)			; If BIT 0 of LISTON is set
			CALL    NZ,OUTCHR		; Then print a space after the line number
			LD      A,B			; Fetch the FOR/NEXT indent level
			ADD     A,A			; Multiply by 2
			BIT     1,(IX)			; If BIT 1 of LISTON is set
			CALL    NZ,FILL			; Then print the FOR/NEXT indent
			LD      A,C			; Fetch the REPEAT/UNTIL indent level
			ADD     A,A			; Multiply by 2
			BIT     2,(IX)			; If BIT 2 of LISTON is set
			CALL    NZ,FILL			; Then print the REPEAT/UNTIL indent
			EXX
			LD      A,(HL)			; Fetch the token
			CP      FOR			; Is it FOR?
			CALL    Z,INDENT		; Yes, so indent
			CP      REPEAT			; Is it REPEAT?
			CALL    Z,INDENT		; Yes, so indent
			LD      E,0			; E: The quote counter - reset to 0
LIST8:			LD      A,(HL)			; Fetch a character / token byte
			INC     HL			
			CP      CR			; Is it end of line?
			JR      Z,LISTE			; Yes, so finish (DB: Used to jump to CRLF, modified for *EDIT)
			CP      34			; Is it a quote character?
			JR      NZ,LIST7		; No, so skip to next bit
			INC     E			; Otherwise increment quote counter
LIST7:			CALL    LOUT			; Output the character / token
			JR      LIST8			; And repeat
;
; DB: Modification for *EDIT
; Terminate the line with either a CRLF or a NUL character
; 
LISTE:			BIT 	3,(IX)			; Are we printing to buffer?
			JR	Z, CRLF			; Yes, so print a CRLF
			XOR	A			; Otherwise print a NUL (0)
			JP	OSWRCH
;
; Decode the 3 byte GOTO type line number
;
PRLINO:			PUSH    HL			; Swap HL and IY
			POP     IY			; IY: Pointer to the line number
			PUSH    BC		
			CALL    DECODE			; Decode
			POP     BC
			EXX
			PUSH    BC
			CALL    PBCDL			; Output the line number
			POP     BC
			EXX
			PUSH    IY			; Swap HL and IY
			POP     HL			; HL: Pointer to the next character in the line
			RET
;
; DB: Modification for internationalisation
;
PRREM:			CALL	OUT_			; Output the REM token
@@:			LD	A, (HL)			; Fetch the character
			CP	CR			; If it is end of line, then
			RET	Z			; we have finished
			CALL	OUTCHR			; Ouput the character
			INC	HL			
			JR	@B			; And loop		
;
; DB: End of modification
;
LOUT:			BIT     0,E			; If the quote counter is odd (bit 1 set) then
			JR      NZ,OUTCHR		; don't tokenise, just output the character
			CP	REM			; DB: Is it REM
			JR	Z, PRREM		; DB: Yes so jump to the special case for REM
			CP      LINO_MN			; Is it a line number (following GOTO/GOSUB etc)?
			JR      Z,PRLINO		; Yes, so decode and print the line number
			CALL    OUT_			; Output a character / keyword
			LD      A,(HL)			; Fetch the next character	
;
; This block of code handles the indentation
; B: Counter for FOR/NEXT indent
; C: Counter for REPEAT/UNTIL indent
;
INDENT:			EXX
			CP      FOR			; If the token is FOR
			JR      Z,IND1			; Then INC B
			CP      NEXT			; If it is NEXT
			JR      NZ,IND2_		; Then...
			DEC     B			; DEC B
			JP      P,IND2_			; If we have gone below 0 then
IND1:			INC     B			; Increment back to 0
;
IND2_:			CP      REPEAT			; If the token is REPEAT
			JR      Z,IND3			; Then INC C
			CP      UNTIL			; If it is UNTIL
			JR      NZ,IND4			; Then...
			DEC     C			; DEC C
			JP      P,IND4			; If we have gone below 0 then
IND3:			INC     C			; Incremet back to 0
IND4:			EXX		
			RET
;
;CRLF - SEND CARRIAGE RETURN, LINE FEED.
;  Destroys: A,F
;OUTCHR - OUTPUT A CHARACTER TO CONSOLE.
;    Inputs: A = character
;  Destroys: A,F
;
CRLF:			LD      A,CR			; Output CR
			CALL    OUTCHR
			LD      A,LF			; Output LF
;
OUTCHR:			CALL    OSWRCH			; Output the character in A
			SUB     CR			; Check for CR
			JR      Z,CARRET		; If it is CR then A will be 0, this will clear the count
			RET     C              		; If it is less than CR, it is non-printing, so don't increment the count
			LD      A,(COUNT)		; Increment the count
			INC     A
;
CARRET:			LD      (COUNT),A		; Store the new count value	
			RET     Z			; Return if the count has wrapped to 0
			PUSH    HL			; Now check if count = print width		
			LD      HL,(WIDTH)		; Get the print width; it's a byte value, so
			CP      L			; L is the width. Compare it with count.
			POP     HL
			RET     NZ			; If we've not hit print width, then just return
			JR      CRLF			; Otherwise output CRLF
;
; OUT - SEND CHARACTER OR KEYWORD
;   Inputs: A = character (>=10, <128)
;           A = Token (<10, >=128)
;  Destroys: A,F
;
OUT_:			CP      138			; Neat trick to do condition: If A >= 10 or < 128 then PE flag is set
			JP      PE,OUTCHR		; If so, then it's a character, so just output it
;
; This bit looks up the character in the KEYWDS token table and expands it
; Note the CP 138; this sets the overflow flag as follows:
;
; NB:
;  1. Any 8-bit number between 128 and 255 is negative (two's complement) so 138 is -118, 128 = -128
;  2. CP is effectively a SUB; sets the flags without affecting A
;  3. The operation n - -118 ~ n + 118
;
; So:
;  *   9 CP 138 ~    9 + 118 = 127 = no overflow : token
;  *  10 CP 138 ~   10 + 118 = 128 =    overflow : character
;  * 127 CP 138 ~  127 + 118 = 245 =    overflow : character
;  * 128 CP 138 ~ -128 + 118 = -10 = no overflow : token
;
			PUSH    BC			; Preserve BC and HL
			PUSH    HL
			LD      HL,KEYWDS		; The list of tokens and keywords
			LD      BC,KEYWDL		; The length of the keyword list
			CPIR				; We can just do a straight CPIR as the token characters are unique in the list
;							; At this point HL points to the next byte, the first character of the token
TOKEN1:			LD      A,(HL)			; Fetch the character
			INC     HL			; Increment to the next byte in the token table
			CP      138			; If A >= 10 or < 128, i.e. we've not hit the token code for the next token
			PUSH    AF			; Then...
			CALL    PE,OUTCHR		; Output the character...
			POP     AF			; 
			JP      PE,TOKEN1		; And loop to the next character 
			POP     HL			; Done, so tidy up the stack and exit
			POP     BC
			RET
;
; FINDL - FIND PROGRAM LINE
;   Inputs: HL = line number (binary)
;  Outputs: HL addresses line (if found)
;           DE = line number
;           Z-flag set if found.
; Destroys: A,B,C,D,E,H,L,F
;
FINDL:			EX      DE,HL			; DE: Line number (binary)
			LD      HL,(PAGE_)		; HL: Top of BASIC program area
			XOR     A               	;  A: 0
			CP      (HL)			; Check for end of program marker
			INC     A			;  A: 1
			RET     NC			; Return with 1 if 0 
			XOR     A               	; Clear the carry flag
;			LD      B,A			;  B: 0
			LD	BC, 0			; BC: 0
;
FINDL1:			LD      C,(HL)			;  C: The line length
			PUSH    HL			; Stack the current program counter
			INC     HL			; Skip to the line number bytes
			LD      A,(HL)			; Fetch the line number (in binary) from the BASIC line in HL
			INC     HL
			LD      H,(HL)
			LD      L,A
			SBC.S   HL,DE			; Compare with the line number we're searching for
			POP     HL			; Get the current program counter
			RET     NC              	; Then return if found or past (Z flag will be set if line number matches)
			ADD     HL,BC			; Skip to the next line (B was set to 0 before the loop was entered)
			JP      FINDL1			; And loop
;
; SETLIN - Search program for line containing address
;          Update (LINENO)
;   Inputs: Address in (ERRLIN)
;  Outputs: Line number in HL and (LINENO)
; Destroys: B,C,D,E,H,L,F
;
SETLIN:			LD	BC, 0			; Zero BC for later
;			LD      B, 0			; Zero B for later
			LD      DE, (ERRLIN)		; DE: Address of line
			LD      HL, (PAGE_)		; HL: Start of user program area
			OR      A			; Do a 24 bit compare without destroying HL
			SBC     HL, DE			;  Z: DE = HL, NC: DE <= HL
			ADD     HL, DE			;  C: DE > HL
			JR      NC, SET3		; So skip, as the address is less than or equal to the top of program area
;
SET1:			LD      C, (HL)			; Get the length of the line; zero indicates the end of the BASIC program
			INC     C			; This is a way to check for zero without using the accumulator
			DEC     C			; If it is zero, then...
			JR      Z, SET3			; We've reached the end of the current BASIC program, not found the line
			ADD     HL, BC			; Skip to the next line (we set B to 0 at the top of this subroutine)
			SBC     HL, DE			; Do a 24-bit compare; the previous ADD will have cleared the carry flag
			ADD     HL, DE			
			JR      C, SET1			; Loop whilst DE (the address to search for) is > HL (the current line)
			SBC     HL, BC			; We've found it, so back up to the beginning of the line
			INC     HL			; Skip the length counter
			LD	DE, 0			; Zero DE
			LD      E, (HL)          	; Fetch the line number
			INC     HL
			LD      D, (HL)
			EX      DE, HL			; HL: The line number
SET2:			LD      (LINENO), HL		; Store in the variable LINENO
			RET
;
SET3:			LD      HL, 0			; We've not found the line at this point so
			JR      SET2			; Set LINENO to 0
;
;SAYLN - PRINT " at line nnnn" MESSAGE.
;  Outputs: Carry=0 if line number is zero.
;           Carry=1 if line number is non-zero.
; Destroys: A,B,C,D,E,H,L,F
;
SAYLN:			LD      HL,(LINENO)		; Get the LINENO sysvar
			LD      A,H			; If it is zero then
			OR      L			
			RET     Z			; Don't need to do anything; return with F:C set to 0
			CALL    TELL			; Output the error message
			DB    	" at line ", 0		
PBCDL:			LD      C,0			; C: Leading character (NUL)
			JR      PBCD0			; Output the line number; return with F:C set to 1
;
; PBCD - PRINT NUMBER AS DECIMAL INTEGER.
;   Inputs: HL = number (binary).
;  Outputs: Carry = 1
; Destroys: A,B,C,D,E,H,L,F
;
PBCD:			LD      C,' '			; C: Leading character (" ")
PBCD0:			LD      B,5			; Number of digits in result
			LD      DE,10000		; Start off with the 10,000 column
PBCD1:			XOR     A			; Counter
PBCD2:			SBC     HL,DE			; Loop and count how many 10,000s we have
			INC     A
			JR      NC,PBCD2
			ADD     HL,DE			; The loop overruns by one, so adjust here
			DEC     A			; A: Number of 10,000s
			JR      Z,PBCD3			; If it is 0, then skip the next bit
			SET     4,C			; C: Set to '0' ASCII (30h)
			SET     5,C
PBCD3:			OR      C			; A is then an ASCII character, or 00h if we've not processed any non-zero digits yet
			CALL    NZ,OUTCHR		; If it is not a leading NUL character then output it
			LD      A,B			; If on first transition, skip this
			CP      5			; TODO: Need to find out why 
			JR      Z,PBCD4			 
			ADD     HL,HL			; HL x  2 : We shift the number being tested left,
			LD      D,H			;         : rather than shifting DE right
			LD      E,L			;         : This makes a lot of sense
			ADD     HL,HL			; HL x  4
			ADD     HL,HL			; HL x  8
			ADD     HL,DE			; HL x 10
PBCD4:			LD      DE,1000			; Set the column heading to 1,000s for subsequent runs
			DJNZ    PBCD1			; Loop until done
			SCF				; SCF set for SAYLN in this module
			RET
;
; PUTVAR - CREATE VARIABLE AND INITIALISE TO ZERO.
;   Inputs: HL, IY as returned from GETVAR (NZ).
;  Outputs: As GETVAR.
; Destroys: everything
;
PUTVAR:			CALL    CREATE			; Create the variable
			LD      A,(IY)			; Fetch the next character
			CP      '('			; Check for bad use of array
			JR      NZ,GETVZ        	; It's fine, so set the exit conditions
ARRAY:			LD      A,14            	; Otherwise Error: 'Array'
ERROR3:			JP      ERROR_
;
;GETVAR - GET LOCATION OF VARIABLE, RETURN IN HL & IX
;   Inputs: IY addresses first character.
;  Outputs: Carry set and NZ if illegal character.
;           Z-flag set if variable found, then:
;            A = variable type (0,4,5,128 or 129)
;            HL = IX = variable pointer.
;            IY updated
;           If Z-flag & carry reset, then:
;            HL, IY set for subsequent PUTVAR call.
; Destroys: everything
;
GETVAR:			LD      A,(IY)			; Get the first character
			CP      '$'			; Is it a string?
			JR      Z,GETV4			; Yes, so branch here
			CP      '!'			; Is it indirection (32-bit)?
			JR      Z,GETV5			; Yes, so branch here
			CP      '?'			; Is it indirection (8-bit)?
			JR      Z,GETV6			; Yes, so branch here
;
			CALL    LOCATE			; Locate the variable
			RET     NZ			; And exit here if not found
;
; At this point:
;  HL: Address of variable in memory
;   D: Variable type (4 = Integer, 5 = Floating point, 129 = String)
;
			LD      A,(IY)			; Further checks
			CP      '('             	; Is it an array?
			JR      NZ,GETVX        	; No, so exit
;
; We are processing an array at this point
;
			PUSH    DE              	; Save the variable type (in D)
			LD      A,(HL)          	; Fetch the number of dimensions
			OR      A
			JR      Z,ARRAY			; If there are none, then Error: 'Array'
			INC     HL			; 
			LD      DE,0            	; Accumulator
			PUSH    AF
			INC     IY              	; Skip "("
			JR      GETV3
;
GETV2:			PUSH    AF
			CALL    COMMA
GETV3:			PUSH    HL
			PUSH    DE
			CALL    EXPRI			; Get the subscript
			EXX
			POP     DE			
			EX      (SP),HL
			LD      C,(HL)
			INC     HL
			LD      B,(HL)
			INC     HL
			EX      (SP),HL
			EX      DE,HL
			PUSH    DE			
			CALL    MUL16			; HL=HL*BC
			POP     DE			
			ADD     HL,DE			
			EX      DE,HL
			OR      A
			SBC     HL,BC
			LD      A,15
			JR      NC,ERROR3		; Throw a "Subscript" error
			POP     HL
			POP     AF
			DEC     A               	; Dimension counter
			JR      NZ,GETV2
			CALL    BRAKET          	; Check for closing bracket
			POP     AF              	; Restore the type
			PUSH    HL
			CALL    X4OR5           	; DE=DE*n
			POP     HL
			ADD     HL,DE
			LD      D,A             	; The type
			LD      A,(IY)
GETVX:			CP      '?'
			JR      Z,GETV9
			CP      '!'
			JR      Z,GETV8
GETVZ:			PUSH    HL              	; Set exit conditions
			POP     IX
			LD      A,D
			CP      A
			RET			
;
; Process strings, unary & binary indirection:
;
GETV4:			LD      A,128           	; Static strings
			JR      GETV7
;
GETV5:			LD      A,4             	; Unary 32-bit indirection
			JR      GETV7
;
GETV6:			XOR     A               	; Unary 8-bit indirection
;
GETV7:			LD      HL,0
			PUSH    AF
			JR      GETV0
;
GETV8:			LD      B,4             	; Binary 32-bt indirection
			JR      GETVA
;
GETV9:			LD      B,0             	; Binary 8-bit indirection
;
GETVA:			PUSH    HL
			POP     IX
			LD      A,D            		; Fetch the variable type
			CP      129			; Is it a string?
			RET     Z               	; Yes, so exit here
			PUSH    BC			
			CALL    LOADN           	; Left operand of the binary indirection (var?index or var!index)
			CALL    SFIX
			LD	A,L
			EXX
			LD	(R0+0),HL
			LD	(R0+2),A
			LD	HL,(R0)			; HL: 24-bit address of the variable in memory
;
GETV0:			PUSH    HL			; HL will be 0 for a unary indirection, or the address of the variable for a binary indirection
			INC     IY
			CALL    ITEMI
			LD	A,L			;  A: The MSB of the address
			EXX
			LD	(R0+0),HL		; HL: The LSW of the address
			LD	(R0+2),A		; R0: L'HL or the 24-bit address
			POP     DE
			POP     AF
			LD	HL,(R0)			; HL: L'HL
			ADD     HL,DE
			PUSH    HL
			POP     IX
			CP      A
			RET
;
;GETDEF - Find entry for FN or PROC in dynamic area.
;   Inputs: IY addresses byte following "DEF" token.
;  Outputs: Z flag set if found
;           Carry set if neither FN or PROC first.
;           If Z: HL points to entry
;                 IY addresses delimiter
; Destroys: A,D,E,H,L,IY,F
;
GETDEF:			LD      A,(IY+1)		; Get the next character from the tokenised line (the start of the procedure name)
			CALL    RANGE1			; Is it in range: "0" to "9", "A" to "Z", "a' to "z", "@", "_" or "`"?
			RET     C			; No so return with C set
			LD      A,(IY)			; Fetch the current character from the tokenised line
			LD      HL,FNPTR		; HL: Address of the dynamic function pointer in ram.asm
			CP      FN			; Is it the token FN?
			JR      Z,LOC2			; Yes, so skip to LOC2 with that pointer to find a match
			LD      HL,PROPTR		; HL: Address of the dynamic procedure pointer in ram.asm
			CP      PROC			; Is it the token PROC?
			JR      Z,LOC2			; Yes, so skip to LOC2 with that pointer to find a match
			SCF				; No, so just return with C set
			RET
;
; LOCATE - Try to locate variable name in static or dynamic variables.
; If illegal first character return carry, non-zero.
; If found, return no-carry, zero.
; If not found, return no-carry, non-zero.
;   Inputs: IY=Addresses first character of name.
;            A=(IY)
;  Outputs:  F=Z set if found, then:
;           IY=addresses terminator
;           HL=addresses location of variable
;            D=type of variable: 4 = integer
;                                5 = floating point
;                              129 = string
; Destroys: A,D,E,H,L,IY,F
;
; Variable names can start with any letter of the alphabet (upper or lower case), underscore (_), or the grave accent (`)
; They can contain any alphanumeric character and underscore (_)
; String variables are postfixed with the dollar ($) character
; Integer variables are postfixed with the percent (%) character
; Static integer variables are named @%, A% to Z%
; All other variables are dynamic
;
LOCATE:			SUB     '@'			; Check for valid range
			RET     C			; First character not "@", "A" to "Z" or "a" to "z", so not a variable
			LD      HL, 0			; Clear HL
			CP      'Z'-'@'+1		; Check for static ("@", "A" to "Z"); if it is not static...
			JR      NC,LOC0         	; Then branch here
			LD	L, A			; HL = A
			LD      A,(IY+1)        	; Check the 2nd character
			CP      '%'			; If not "%" then it is not static...
			JR      NZ,LOC1         	; Branch here
			LD      A,(IY+2)		; Check the 3rd character
			CP      '('			; If it is "(" (array) then it is not static...
			JR      Z,LOC1          	; Branch here
;
; At this point we're dealing with a static variable
;
			ADD     HL,HL			; HL: Variable index * 4
			ADD	HL,HL
			LD      DE,STAVAR       	; The static variable area in memory
			ADD     HL,DE			; HL: The address of the static variable
			INC     IY			; Skip the program pointer past the static variable name
			INC     IY	
			LD      D,4             	; Set the type to be integer
			XOR     A			; Set the Z flag
			RET
;
; At this point it's potentially a dynamic variable, just need to do a few more checks
;
LOC0:			CP      '_'-'@'			; Check the first character is in
			RET     C			; the range "_" to 
			CP      'z'-'@'+1		; "z" (lowercase characters only)
			CCF				; If it is not in range then
			DEC     A               	; Set NZ flag and
			RET     C			; Exit here
			SUB     3			; This brings it in the range of 27 upwards (need to confirm)
			LD	L, A			; HL = A
;
; Yes, it's definitely a dynamic variable at this point...
;
LOC1:			LD	A, L			; Fetch variable index
			ADD	A, A			; x 2
			ADD	A, L			; x 3
			SUB	3			; Subtract 2 TODO: Should be 3
			LD	L, A
			LD      DE, DYNVAR       	; The dynamic variable storage
			RET	C			; Bounds check to trap for variable '@'
			ADD     HL, DE			; HL: Address of first entry
;
; Loop through the linked list of variables to find a match
;
LOC2:			LD	DE, (HL)		; Fetch the original pointer
			PUSH	HL			; Need to preserve HL for LOC6
			XOR	A			; Reset carry flag
			SBC	HL, HL			; Set HL to 0
			SBC	HL, DE			; Compare with 0
			POP	HL			; Restore the original pointer
			JR	Z, LOC6			; If the pointer in DE is zero, the variable is undefined at this point
			; LD	HL, DE			; Make a copy of this pointer in HL
			push de
			pop hl ; how was that even possible?
			INC     HL              	; Skip the link (24-bits)
			INC     HL
			INC	HL			; HL: Address of the variable name in DYNVARS
			PUSH    IY			; IY: Address of the variable name in the program
;
LOC3:			LD      A,(HL)         		; Compare
			INC     HL
			INC     IY
			CP      (IY)
			JR      Z, LOC3			; Keep looping whilst we've got a match...
			OR      A               	; Have we hit a terminator?
			JR      Z,LOC5          	; Yes, so maybe we've found a variable
;
LOC4:			POP     IY			; Restore the pointer in the program
			EX      DE, HL			; HL: New pointer in DYNVARS
			JP      LOC2            	; Loop round and try again
;
; We might have located a variable at this point, just need to do a few more tests
;
LOC5:			DEC     IY
			LD      A,(IY)
			CP      '('
			JR      Z,LOC5A         	; FOUND
			INC     IY
			CALL    RANGE
			JR      C,LOC5A         	; FOUND
			CP      '('
			JR      Z,LOC4          	; KEEP LOOKING
			LD      A,(IY-1)
			CALL    RANGE1
			JR      NC,LOC4         	; KEEP LOOKING
LOC5A:			POP     DE
TYPE_:			LD      A,(IY-1)		; Check the string type postfix
			CP      '$'			; Is it a string?
			LD      D,129			; Yes, so return D = 129
			RET     Z               		
			CP      '%'			; Is it an integer?
			LD      D,4			; Yes, so return D = 4
			RET     Z               		
			INC     D			; At this point it must be a float
			CP      A			; Set the flags
			RET
;
; The variable is undefined at this point; HL will be zero
;
LOC6:			INC     A               	; Set NZ flag
			RET
;
; CREATE - CREATE NEW ENTRY, INITIALISE TO ZERO.
;   Inputs: HL, IY as returned from LOCATE (NZ).
;  Outputs: As LOCATE, GETDEF.
; Destroys: As LOCATE, GETDEF.
;
CREATE:			XOR     A				
			LD      DE,(FREE)		; Get the last byte of available RAM
			LD	(HL), DE		; Store 
			EX      DE,HL
			LD      (HL),A			; Clear the link of the new entity
			INC     HL
			LD      (HL),A
			INC     HL
			LD      (HL),A
			INC     HL
LOC7:			INC     IY
			CALL    RANGE           	; END OF VARIABLE?
			JR      C,LOC8
			LD      (HL),A
			INC     HL
			CALL    RANGE1
			JR      NC,LOC7
			CP      '('
			JR      Z,LOC8
			LD      A,(IY+1)
			CP      '('
			JR      Z,LOC7
			INC     IY
LOC8:			LD      (HL),0          	; TERMINATOR
			INC     HL
			PUSH    HL
			CALL    TYPE_			; Get the variable type in D
			LD      A,4			; If it is an integer then it takes up 4 bytes
			CP      D
			JR      Z,LOC9			; So skip the next bit
			INC     A			; Strings and floats take up 5 bytes (NB: Strings take up 4 in BBC BASIC for Z80)
LOC9:			LD      (HL),0          	; Initialise the memory to zero
			INC     HL
			DEC     A
			JR      NZ,LOC9
			LD      (FREE),HL		; Adjust the stack
			CALL    CHECK			; Check whether we are out of space
			POP     HL
			XOR     A
			RET
;
; LINNUM - GET LINE NUMBER FROM TEXT STRING
;   Inputs: IY = Text Pointer
;  Outputs: HL = Line number (zero if none)
;           IY updated
; Destroys: A,D,E,H,L,IY,F
;
; This bit of code performs a BASE 10 shift to build up the number
; So if the string passed is "345", the algorithm does this:
;
;    HL : Digit	: Operation
; ----- : ----- : ---------
; 00000 :	:
; 00003 :     3	: Multiply HL  (0) by 10   (0) and add 3   (3)
; 00034 :     4 : Multiply HL  (3) by 10  (30) and add 4  (34)
; 00345 :     5	: Multiply HL (34) by 10 (340) and add 5 (345)
;
; The multiply by 10 is done by an unrolled shift and add loop
;
LINNUM:			CALL    NXT			; Skip whitespace to the first character
			LD.SIS  HL,0			; The running total
LINNM1:			LD      A,(IY)			; A: Fetch the digit to add in
			SUB     '0'			; Sub ASCII '0' to make a binary number (0-9)
			RET     C			; And return if less than 0
			CP      10			; Or greater than or equal to 10
			RET     NC			; As we've hit a non-numeric character (end of number) at this point
			INC     IY			; Increment the string pointer
			LD      D,H			; This next block multiplys HL by 10, shifting the result left in BASE 10
			LD      E,L			; Store the original number in DE
			ADD.S   HL,HL           	; *2
			JR      C,TOOBIG		; At each point, error if > 65535 (carry flag set)
			ADD.S   HL,HL           	; *4S
			JR      C,TOOBIG
			ADD.S   HL,DE           	; *5
			JR      C,TOOBIG	
			ADD.S   HL,HL           	; *10
			JR      C,TOOBIG
			LD      E,A			; A->DE: the digit to add in
			LD      D,0
			ADD.S   HL,DE           	; Add in the digit to the running total
			JR      NC,LINNM1       	; And if it is still <= 65535, loop
;
TOOBIG:			LD      A,20
			JP      ERROR_           	; Error: "Too big"
;
; PAIR - GET PAIR OF LINE NUMBERS FOR RENUMBER/AUTO.
;   Inputs: IY = text pointer
;  Outputs: HL = first number (10 by default)
;           BC = second number (10 by default)
; Destroys: A,B,C,D,E,H,L,B',C',D',E',H',L',IY,F
;
PAIR:			CALL    LINNUM          	; Parse the first line number
			LD      A,H			; If it is not zero, then...
			OR      L
			JR      NZ,PAIR1		; Skip...
			LD      L,10			; HL: the default value (10)
;
PAIR1:			CALL    TERMQ			; Check for ELSE, : or CR
			INC     IY			; Skip to next character
			PUSH    HL			; Stack the first line number
			LD      HL,10			; HL: the second default (10)
			CALL    NZ,LINNUM       	; Parse the second line number
			EX      (SP),HL			; HL: The first line number (off the stack)
			POP     BC			; BC: Second line number
			LD      A,B			; If the second line number is not zero then...
			OR      C			; We're good...
			RET     NZ			; Exit, otherwise...
			CALL    EXTERR			; Throw error: "Silly"
			DB    	"Silly", 0
;
; DLPAIR - GET PAIR OF LINE NUMBERS FOR DELETE/LIST.
;   Inputs: IY = text pointer
;  Outputs: HL = points to program text
;           BC = second number (0 by default)
; Destroys: A,B,C,D,E,H,L,IY,F
;
DLPAIR:			CALL    LINNUM			; Parse the first line number
			PUSH    HL			; Stack it
			CALL    TERMQ			; Check for ELSE, : or CR
			JR      Z,DLP1			; And exit if so 
			CP      TIF			; Is the token IF?
			JR      Z,DLP1			; Yes, so skip the next bit...
			INC     IY			; Otherwise...
			CALL    LINNUM			; Fetch the second line number
DLP1:			EX      (SP),HL			; HL: The first line number (off the stack)
			CALL    FINDL			; HL: Find the address of the line
			POP     BC			; BC: The second number
			RET
;
; TEST FOR VALID CHARACTER IN VARIABLE NAME:
;   Inputs: IY addresses character
;  Outputs: Carry set if out-of-range.
; Destroys: A,F
;
; It is called here to check the following
; In range: "$", "%" and "("
;   Plus all characters in RANGE1 and RANGE2
;
RANGE:			LD      A,(IY)			; Fetch the character
			CP      '$'			; Postfix for string variable is valid
			RET     Z
			CP      '%'			; Postfix for integer variable is valid
			RET     Z
			CP      '('			; Postfix for array is valid
			RET     Z
;
; It is called here to check the following
; In range: "0" to "9" and "@"
;   Plus all characters in RANGE2
;
RANGE1:			CP      '0'			; If it is between '0'...
			RET     C			 
			CP      '9'+1			; And '9'...
			CCF
			RET     NC			; Then it is valid
			CP      '@'             	; The prefix @ is valid (@% controls numeric print formatting - v2.4)
			RET     Z
;
; It is called here to check the following
; In range: "A" to "Z", "a' to "z", "_" and "`"
;	
RANGE2:			CP      'A'			; If it is between 'A'...
			RET     C
			CP      'Z'+1			; And 'Z'...
			CCF
			RET     NC			; Then it is valid
			CP      '_'			; If it is underscore, grave, or between 'a'
			RET     C
			CP      'z'+1			; And 'z'
			CCF				; Then it is valid
			RET
;
; Throw a 'LINE space' error (line too long)
; This is called from LEXAN
;
SPACE_: 		XOR     A
			CALL    EXTERR          	; "LINE space"
			DB    	LINE_MN_, 8, 0
;
; LEXAN - LEXICAL ANALYSIS.
;  Bit 0,C: 1=left, 0=right
;  Bit 2,C: 1=in BINARY
;  Bit 3,C: 1=in HEX
;  Bit 4,C: 1=accept line number
;  Bit 5,C: 1=in variable, FN, PROC
;  Bit 6,C: 1=in REM, DATA, *
;  Bit 7,C: 1=in quotes
;   Inputs: IY addresses source string
;           DE addresses destination string (must be page boundary)
;            C sets initial mode
;  Outputs: DE, IY updated
;            A holds carriage return
;
LEXAN1:			LD      (DE),A          	; Transfer to buffer
			INC     DE              	; Increment the pointers
			INC     IY			; And fall through to the main function
;
; This is the main entry point
;
LEXAN2:			LD      A,E             	; Destination buffer on page boundary, so E can be used as length
			CP      252             	; If it is >= 252 bytes, then...
			JR      NC,SPACE_        	; Throw a 'LINE space' error (line too long)
			LD      A,(IY)			; Fetch character from source string
			CP      CR			; If it is a CR
			RET     Z               	; Then it is end of line; we're done parsing
			CALL    RANGE1			; Is it alphanumeric, '@', '_' or '`'
			JR      NC,LEXAN3		; Yes, so skip
			RES     5,C             	; FLAG: NOT IN VARIABLE
			RES     3,C             	; FLAG: NOT IN HEX
			RES	2,C			; FLAG: NOT IN BINARY
;
LEXAN3:			CP      ' '			; Ignore spaces
			JR      Z,LEXAN1        	
			CP      ','			; Ignore commas
			JR      Z,LEXAN1 
			CP	'2'			; If less than '2'
			JR	NC, @F			; No, so skip
			RES	2,C			; FLAG: NOT IN BINARY
@@:			CP      'G'			; If less then 'G'
			JR      C,LEXAN4		; Yes, so skip
			RES     3,C             	; FLAG: NOT IN HEX
;
LEXAN4:			CP      34			; Is it a quote character?
			JR      NZ,LEXAN5		; No, so skip
			RL      C			; Toggle bit 7 of C by shifting it into carry flag
			CCF                     	; Toggle the carry
			RR      C			; And then shifting it back into bit 7 of C
;
LEXAN5:			BIT     4,C			; Accept line number?
			JR      Z,LEXAN6		; No, so skip
			RES     4,C			; FLAG: DON'T ACCEPT LINE NUMBER
			PUSH    BC			
			PUSH    DE
			CALL    LINNUM         		; Parse the line number to HL
			POP     DE
			POP     BC
			LD      A,H			; If it is not zero
			OR      L
			CALL    NZ,ENCODE       	; Then encode the line number HL to the destination (DE)
			JR      LEXAN2          	; And loop
;
LEXAN6:			DEC     C			; Check for C=1 (LEFT)
			JR      Z,LEXAN7        	; If so, skip
			INC     C			; Otherwise restore C
			JR      NZ,LEXAN1		; If C was 0 (RIGHT) then...
			OR      A			; Set the flags based on the character
			CALL    P,LEX           	; Tokenise if A < 128
			JR      LEXAN8			; And skip
;
; Processing the LEFT hand side here
; 
LEXAN7:			CP      '*'			; Is it a '*' (for star commands)
			JR      Z,LEXAN9		; Yes, so skip to quit tokenising
			OR      A			; Set the flags based on the character
			CALL    P,LEX           	; Tokenise if A < 128
;
; This bit of code checks if the tokens are one of the pseudo-variables PTR, PAGE, TIME, LOMEM, HIMEM
; These tokens are duplicate in the table with a GET version and a SET version offset by the define OFFSET (40h)
; Examples:
;   LET A% = PAGE : REM This is the GET version
;   PAGE = 40000  : REM This is the SET version
;
			CP      TOKLO			; TOKLO is 8Fh
			JR      C,LEXAN8		; If A is < 8Fh then skip to LEX8
			CP      TOKHI+1			; TOKHI is 93h
			JR      NC,LEXAN8		; If A is >= 94h then skip to LEX8
			ADD     A,OFFSET       		; Add OFFSET (40h) to make the token the SET version
;
LEXAN8:			CP      REM			; If the token is REM
			JR      Z,LEXAN9		; Then stop tokenising
			CP      DATA_MN_			; If it is not DATA then
			JR      NZ,LEXANA		; Skip
LEXAN9:			SET     6,C             	; FLAG: STOP TOKENISING
;
LEXANA:			CP      FN			; If the token is FN
			JR      Z,LEXANB		
			CP      PROC			; Or the token is PROC
			JR      Z,LEXANB		; Then jump to here
			CALL    RANGE2			; Otherwise check the input is alphanumeric, "_" or "`"
			JR      C,LEXANC		; Jump here if out of range
;
LEXANB:			SET     5,C             	; FLAG: IN VARIABLE/FN/PROC
LEXANC:			CP      '&'			; Check for hex prefix
			JR      NZ,LEXAND		; If not, skip
			SET     3,C             	; FLAG: IN HEX
;
LEXAND:			CP	'%'			; Check for binary prefix
			JR	NZ,LEXANE		; If not, skip
			SET	2,C			; FLAG: IN BINARY
;
LEXANE:			LD      HL,LIST1		; List of tokens that must be followed by a line number	
			PUSH    BC			
			LD      BC,LIST1L		; The list length
			CPIR				; Check if the token is in this list
			POP     BC
			JR      NZ,LEXANF		; If not, then skip
			SET     4,C             	; FLAG: ACCEPT LINE NUMBER
;
LEXANF:			LD      HL,LIST2		; List of tokens that switch the lexical analysis back to LEFT mode
			PUSH    BC
			LD      BC,LIST2L		; The list length
			CPIR				; Check if the token is in this list
			POP     BC		
			JR      NZ,LEXANG		; If not, then skip
			SET     0,C             	; FLAG: ENTER LEFT MODE
LEXANG:			JP      LEXAN1			; And loop

;
; LIST1: List of tokens that must be followed by line numbers
; LIST2: List of tokens that switch the lexical analysis back to LEFT mode
;
LIST1:			DB	GOTO
			DB	GOSUB
			DB	RESTOR
			DB	TRACE
LIST2:			DB	THEN_MN_
			DB	ELSE_MN_
LIST1L:			EQU     $-LIST1
			DB	REPEAT
			DB	TERROR_MN
			DB    	':'
LIST2L:			EQU     $-LIST2
;
; ENCODE - ENCODE LINE NUMBER INTO PSEUDO-BINARY FORM.
;   Inputs: HL=line number, DE=string pointer
;  Outputs: DE updated, BIT 4,C set.
; Destroys: A,B,C,D,E,F
;
; Thanks to Matt Godblot for this explanation (https://xania.org/200711/bbc-basic-line-number-format)
;
; The line number is spread over three bytes and kept in the range of normal ASCII values so the interpreter
; can make this short cut in skipping to the non-ASCII token ELSE. The algorithm used splits the top two bits off
; each of the two bytes of the 16-bit line number. These bits are combined (in binary as 00LlHh00),
; exclusive-ORred with 0x54, and stored as the first byte of the 3-byte sequence. The remaining six bits of
; each byte are then stored, in LO/HI order, ORred with 0x40.
;
ENCODE:			SET     4,C			; Set bit 4 of C (for lexical analysis - accept line number)
			EX      DE, HL			; HL: string pointer, DE: line number
			LD      (HL), LINO_MN		; Store 8Dh first to flag next bytes as an encoded line number
			INC     HL
			LD      A,D			; Get the high byte
			AND     0C0H			; Get the top two bits	DD000000
			RRCA				; Shift right		00DD0000
			RRCA
			LD      B,A			; Store in B
			LD      A,E			; Get the low byte
			AND     0C0H			; Get the top two bits	EE000000
			OR      B			; Combine with D	EEDD0000
			RRCA				; Shift right		00EEDD00
			RRCA
			XOR     01010100B		; XOR with 54h
			LD      (HL),A			; Store this as the second byte
			INC     HL
			LD      A,E			; Get the low byte
			AND     3FH			; Strip the top two bits off
			OR      '@'			; OR with 40h
			LD      (HL),A			; Store
			INC     HL		
			LD      A,D			; Get the high byte
			AND     3FH			; Strip the top two bits off
			OR      '@'			; OR with 40h
			LD      (HL),A			; Store
			INC     HL
			EX      DE,HL			; DE: string pointer, HL: line number	
			RET
;
; TEXT - OUTPUT MESSAGE.
;   Inputs: HL addresses text (terminated by nul)
;  Outputs: HL addresses character following nul.
; Destroys: A,H,L,F
;
REPORT:			LD      HL, (ERRTXT)		; Output an error message pointed to by ERRTXT
;
TEXT_:			LD      A, (HL)			; Fetch the character
			INC     HL			; Increment pointer to next character
			OR      A			; Check for the nul (0) string terminator
			RET     Z			; And return if so
			CALL    OUT_			; Output the character; note that OUT_ will detokenise tokens
			JR      TEXT_			; And loop
;
; TELL - OUTPUT MESSAGE.
;   Inputs: Text follows subroutine call (term=nul)
; Destroys: A,F
;
; Example usage:
;
;	CALL	TELL			Call the function
;	DB	"Hello World", 0	Followed by a zero terminated string
;	LD	A, (1234H)		Program execution will carry on here after the message is output
;
TELL:			EX      (SP), HL		; Get the return address off the stack into HL, this is the
			CALL    TEXT_			; first byte of the string that follows it. Print it, then
			EX      (SP), HL		; HL will point to the next instruction, swap this back onto the stack	
			RET				; at this point we'll return to the first instruction after the message; --- End main.asm ---

; --- Begin misc.asm ---
;
; Title:	BBC Basic for AGON - Miscellaneous helper functions
; Author:	Dean Belfield
; Created:	12/05/2023
; Last Updated:	12/05/2023
;
; Modinfo:

			; INCLUDE	"equs.inc"
			; INCLUDE	"macros.inc"

			; .ASSUME	ADL = 1

			; SEGMENT CODE
				
			; XDEF	ASC_TO_NUMBER
			; XDEF	SWITCH_A
			; XDEF	NULLTOCR
			; XDEF	CRTONULL
			; XDEF	CSTR_FNAME
			; XDEF	CSTR_LINE
			; XDEF	CSTR_FINDCH
			; XDEF	CSTR_ENDSWITH
			; XDEF	CSTR_CAT
				
			; XREF	OSWRCH
			; XREF	KEYWDS
			; XREF	KEYWDL

; Read a number and convert to binary
; If prefixed with &, will read as hex, otherwise decimal
;   Inputs: HL: Pointer in string buffer
;  Outputs: HL: Updated text pointer
;           DE: Value
;            A: Terminator (spaces skipped)
; Destroys: A,D,E,H,L,F
;
ASC_TO_NUMBER:		PUSH	BC			; Preserve BC
			LD	DE, 0			; Initialise DE
			CALL	SKIPSPC			; Skip whitespace
			LD	A, (HL)			; Read first character
			CP	'&'			; Is it prefixed with '&' (HEX number)?
			JR	NZ, ASC_TO_NUMBER3	; Jump to decimal parser if not
			INC	HL			; Otherwise fall through to ASC_TO_HEX
;
ASC_TO_NUMBER1:		LD	A, (HL)			; Fetch the character
			CALL    UPPERC			; Convert to uppercase  
			SUB	'0'			; Normalise to 0
			JR 	C, ASC_TO_NUMBER4	; Return if < ASCII '0'
			CP 	10			; Check if >= 10
			JR 	C,ASC_TO_NUMBER2	; No, so skip next bit
			SUB 	7			; Adjust ASCII A-F to nibble
			CP 	16			; Check for > F
			JR 	NC, ASC_TO_NUMBER4	; Return if out of range
ASC_TO_NUMBER2:		EX 	DE, HL 			; Shift DE left 4 times
			ADD	HL, HL	
			ADD	HL, HL	
			ADD	HL, HL	
			ADD	HL, HL	
			EX	DE, HL	
			OR      E			; OR the new digit in to the least significant nibble
			LD      E, A
			INC     HL			; Onto the next character
			JR      ASC_TO_NUMBER1		; And loop
;
ASC_TO_NUMBER3:		LD	A, (HL)
			SUB	'0'			; Normalise to 0
			JR	C, ASC_TO_NUMBER4	; Return if < ASCII '0'
			CP	10			; Check if >= 10
			JR	NC, ASC_TO_NUMBER4	; Return if >= 10
			EX 	DE, HL 			; Stick DE in HL
			LD	B, H 			; And copy HL into BC
			LD	C, L	
			ADD	HL, HL 			; x 2 
			ADD	HL, HL 			; x 4
			ADD	HL, BC 			; x 5
			ADD	HL, HL 			; x 10
			EX	DE, HL
			ADD8U_DE 			; Add A to DE (macro)
			INC	HL
			JR	ASC_TO_NUMBER3
ASC_TO_NUMBER4:		POP	BC 			; Fall through to SKIPSPC here

; Skip a space
; HL: Pointer in string buffer
; 
SKIPSPC:			LD      A, (HL)
			CP      ' '
			RET     NZ
			INC     HL
			JR      SKIPSPC

; Skip a string
; HL: Pointer in string buffer
;
SKIPNOTSP:		LD	A, (HL)
			CP	' '
			RET	Z 
			INC	HL 
			JR	SKIPNOTSP

; Convert a character to upper case
;  A: Character to convert
;
UPPERC:  		AND     7FH
			CP      '`'
			RET     C
			AND     5FH			; Convert to upper case
			RET			

; Switch on A - lookup table immediately after call
;  A: Index into lookup table
;
SWITCH_A:		EX	(SP), HL		; Swap HL with the contents of the top of the stack
			ADD	A, A			; Multiply A by two
			ADD8U_HL 			; Add to HL (macro)
			LD	A, (HL)			; follow the call. Fetch an address from the
			INC	HL 			; table.
			LD	H, (HL)
			LD	L, A
			EX	(SP), HL		; Swap this new address back, restores HL
			RET				; Return program control to this new address

; Convert the buffer to a null terminated string and back
; HL: Buffer address
;			
NULLTOCR:		PUSH 	BC
			LD	B, 0
			LD	C, CR 
			JR	CRTONULL0
;			
CRTONULL:		PUSH	BC
			LD	B, CR
			LD	C, 0	
;			
CRTONULL0:		PUSH	HL
CRTONULL1:		LD	A, (HL)
			CP 	B 
			JR	Z, CRTONULL2
			INC	HL 
			JR	CRTONULL1
CRTONULL2:		LD	(HL), C
			POP 	HL 
			POP	BC
			RET
			
; Copy a filename to DE and zero terminate it
; HL: Source
; DE: Destination (ACCS)
;
CSTR_FNAME:		LD	A, (HL)			; Get source
			CP	32			; Is it space
			JR	Z, @F	
			CP	CR			; Or is it CR
			JR	Z, @F
			LD	(DE), A			; No, so store
			INC	HL			; Increment
			INC	DE			
			JR	CSTR_FNAME		; And loop
@@:			XOR	A			; Zero terminate the target string
			LD	(DE), A
			INC	DE			; And point to next free address
			RET
			
; Copy a CR terminated line to DE and zero terminate it
; HL: Source
; DE: Destination (ACCS)
;
CSTR_LINE:		LD	A, (HL)			; Get source
			CP	CR			; Is it CR
			JR	Z, @F
			LD	(DE), A			; No, so store
			INC	HL			; Increment
			INC	DE			
			JR	CSTR_LINE		; And loop
@@:			XOR	A			; Zero terminate the target string
			LD	(DE), A
			INC	DE			; And point to next free address
			RET
			
; Find the first occurrence of a character (case sensitive)
; HL: Source
;  C: Character to find
; Returns:
; HL: Pointer to character, or end of string marker
;
CSTR_FINDCH:		LD	A, (HL)			; Get source
			CP	C			; Is it our character?
			RET	Z			; Yes, so exit
			OR	A			; Is it the end of string?
			RET	Z			; Yes, so exit
			INC	HL
			JR	CSTR_FINDCH
			
; Check whether a string ends with another string (case insensitive)
; HL: Source
; DE: The substring we want to test with
; Returns:
;  F: Z if HL ends with DE, otherwise NZ
;
CSTR_ENDSWITH:		LD	A, (HL)			; Get the source string byte
			CALL	UPPERC			; Convert to upper case
			LD	C, A
			LD	A, (DE)			; Get the substring byte
			CP	C
			RET	NZ			; Return NZ if at any point the strings don't match
			OR	C			; Check whether both bytes are zero
			RET	Z			; If so, return, as we have reached the end of both strings
			INC	HL
			INC	DE
			JR	CSTR_ENDSWITH		; And loop
			
; Concatenate a string onto the end of another string
; HL: Source
; DE: Second string
;
CSTR_CAT:		LD	A, (HL)			; Loop until we find the end of the first string
			OR	A
			JR	Z, CSTR_CAT_1
			INC	HL
			JR	CSTR_CAT
;
CSTR_CAT_1:		LD	A, (DE)			; Copy the second string onto the end of the first string
			LD	(HL), A
			OR	A			; Check for end of string
			RET	Z			; And return
			INC	HL
			INC	DE
			JR	CSTR_CAT_1		; Loop until finished						; --- End misc.asm ---

; --- Begin patch.asm ---
;
; Title:	BBC Basic for AGON
; Author:	Dean Belfield
; Created:	12/05/2023
; Last Updated:	15/11/2023
;
; Modinfo:
; 11/07/2023:	Fixed *BYE for ADL mode
; 15/11/2023:	Improved OSLOAD_TXT; now handles LF terminated files, files with no trailing LF or CR/LF at end

			; .ASSUME	ADL = 1
				
			; INCLUDE	"equs.inc"
			; INCLUDE "macros.inc"
			; INCLUDE "mos_api.inc"	; In MOS/src
		
			; SEGMENT CODE
				
			; XDEF	OSWRCH
			; XDEF	OSLINE
			; XDEF	ESCSET
			; XDEF	PUTIME
			; XDEF	GETIME
			; XDEF	PUTCSR
			; XDEF 	GETCSR
			; XDEF	OSRDCH
			; XDEF	PROMPT
			; XDEF	OSKEY
			; XDEF	TRAP
			; XDEF	LTRAP
			; XDEF	OSINIT
			; XDEF	OSCLI
			; XDEF	OSBPUT
			; XDEF	OSBGET
			; XDEF	OSSTAT
			; XDEF	OSSHUT
			; XDEF	OSOPEN
			; XDEF	OSCALL
			; XDEF	GETPTR
			; XDEF	PUTPTR
			; XDEF	GETEXT
			; XDEF	GETIMS
			; XDEF	RESET
			; XDEF	OSLOAD
			; XDEF	OSSAVE
			; XDEF	EXPR_W2
			; XDEF	STAR_VERSION

			; XREF	_end			; In init.asm

			; XREF	ASC_TO_NUMBER
			; XREF	RAM_START
			; XREF	RAM_END
			; XREF	FLAGS
			; XREF	ESCAPE
			; XREF	USER
			; XREF	RAM_Top
			; XREF	EXTERR
			; XREF	COUNT0
			; XREF	EXPRI
			; XREF	COMMA
			; XREF	XEQ
			; XREF	NXT
			; XREF	NULLTOCR
			; XREF	CRLF
			; XREF	CSTR_FNAME
			; XREF	CSTR_LINE
			; XREF	CSTR_FINDCH
			; XREF	CSTR_ENDSWITH
			; XREF	CSTR_CAT
			; XREF	FINDL
			; XREF	OUT_
			; XREF	ERROR_
			; XREF	ONEDIT
			; XREF	TELL
			; XREF	OSWRCHPT
			; XREF	OSWRCHCH
			; XREF	OSWRCHFH
			; XREF	LISTON
			; XREF	LISTIT
			; XREF	PAGE_
			; XREF	ONEDIT1
			; XREF	CLEAN
			; XREF	NEWIT
			; XREF	BAD
			; XREF	VBLANK_INIT
			; XREF	VBLANK_STOP
			; XREF	KEYDOWN
			; XREF	KEYASCII
			; XREF	WIDTH
			; XREF	ASSEM

; OSLINE: Invoke the line editor
;
OSLINE:			LD 	E, 1			; Default is to clear the buffer

; Entry point to line editor that does not clear the buffer
; 
OSLINE1:		PUSH	IY			
			PUSH	HL			; Buffer address
			LD	BC, 256			; Buffer length
			MOSCALL	mos_editline		; Call the MOS line editor
			POP	HL			; Pop the address
			POP	IY
			PUSH	AF			; Stack the return value (key pressed)
			CALL	NULLTOCR		; Turn the 0 character to a CR
			CALL	CRLF			; Display CRLF
			POP	AF
			CP	1Bh 			; Check if ESC terminated the input
			JP	Z, LTRAP1 		; Yes, so do the ESC thing
			LD	A, (FLAGS)		; Otherwise
			RES	7, A 			; Clear the escape flag
			LD	(FLAGS), A 
			CALL	WAIT_VBLANK 		; Wait a frame 
 			XOR	A			; Return A = 0
			LD	(KEYDOWN), A 
			LD	(KEYASCII), A
			RET		

; PUTIME: set current time to DE:HL, in centiseconds.
;
PUTIME:			PUSH 	IX
			MOSCALL	mos_sysvars
			LD	(IX + sysvar_time + 0), L
			LD	(IX + sysvar_time + 1), H
			LD	(IX + sysvar_time + 2), E
			LD	(IX + sysvar_time + 3), D
			POP	IX
			RET

; GETIME: return current time in DE:HL, in centiseconds
;
GETIME:			PUSH 	IX
			MOSCALL	mos_sysvars
			LD	L, (IX + sysvar_time + 0)
			LD	H, (IX + sysvar_time + 1)
			LD	E, (IX + sysvar_time + 2)
			LD	D, (IX + sysvar_time + 3)
			POP	IX
			RET

; PUTCSR: move to cursor to x=DE, y=HL
;
PUTCSR:			LD	A, 1Fh			; TAB
			RST.LIL	10h
			LD	A, E			; X
			RST.LIL 10h
			LD	A, L			; Y
			RST.LIL 10h
			RET

; GETCSR: return cursor position in x=DE, y=HL
;
GETCSR:			PUSH	IX			; Get the system vars in IX
			MOSCALL	mos_sysvars		; Reset the semaphore
			RES	0, (IX+sysvar_vpd_pflags)
			VDU	23
			VDU	0
			VDU	vdp_cursor
@@:			BIT	0, (IX+sysvar_vpd_pflags)
			JR	Z, @B			; Wait for the result
			LD 	D, 0
			LD	H, D
			LD	E, (IX + sysvar_cursorX)
			LD	L, (IX + sysvar_cursorY)			
			POP	IX			
			RET			

; PROMPT: output the input prompt
;
PROMPT: 		LD	A,'>'
			JP	OSWRCH
			
; OSWRCH: Write a character out to the ESP32 VDU handler via the MOS
; A: Character to write
;
OSWRCH:			PUSH	HL
			LD	HL, LISTON		; Fetch the LISTON variable
			BIT	3, (HL)			; Check whether we are in *EDIT mode
			JR	NZ, OSWRCH_BUFFER	; Yes, so just output to buffer
;
			LD	HL, (OSWRCHCH)		; L: Channel #
			DEC	L			; If it is 1
			JR	Z, OSWRCH_FILE		; Then we are outputting to a file
;
			POP	HL			; Otherwise
			RST.LIL	10h			; Output the character to MOS
			RET
;	
OSWRCH_BUFFER:		LD	HL, (OSWRCHPT)		; Fetch the pointer buffer
			LD	(HL), A			; Echo the character into the buffer
			INC	HL			; Increment pointer
			LD	(OSWRCHPT), HL		; Write pointer back
			POP	HL			
			RET
;
OSWRCH_FILE:		PUSH	DE
			LD	E, H			; Filehandle to E
			CALL	OSBPUT			; Write the byte out
			POP	DE
			POP	HL
			RET

; OSRDCH: Read a character in from the ESP32 keyboard handler
; This is only called in GETS (eval.asm)
;
OSRDCH:			MOSCALL	mos_getkey		; Read keyboard
			CP	1Bh
			JR	Z, LTRAP1 
			RET

			
;OSKEY - Read key with time-limit, test for ESCape.
;Main function is carried out in user patch.
;   Inputs: HL = time limit (centiseconds)
;  Outputs: Carry reset if time-out
;           If carry set A = character
; Destroys: A,H,L,F
;
OSKEY:			CALL	READKEY			; Read the keyboard 
			JR	Z, @F 			; Skip if we have a key
			LD	A, H 			; Check loop counter
			OR 	L
			RET 	Z 			; Return, we've not got a key at this point
			CALL	WAIT_VBLANK 		; Wait a frame
			DEC 	HL			; Decrement
			JR	OSKEY 			; And loop
;
@@:			LD	HL, KEYDOWN		; We have a key, so 
			LD	(HL), 0			; clear the keydown flag
			CP	1BH			; If we are not pressing ESC, 
			SCF 				; then flag we've got a character
			RET	NZ		
;
; ESCSET
; Set the escape flag (bit 7 of FLAGS = 1) if escape is enabled (bit 6 of FLAGS = 0)
;
ESCSET: 		PUSH    HL
        		LD      HL,FLAGS		; Pointer to FLAGS
        		BIT     6,(HL)			; If bit 6 is set, then
        		JR      NZ,ESCDIS		; escape is disabled, so skip
        		SET     7,(HL)			; Set bit 7, the escape flag
ESCDIS: 		POP     HL
        		RET	
;
; ESCTEST
; Test for ESC key
;
ESCTEST:		CALL	READKEY			; Read the keyboard
			RET	NZ			; Skip if no key is pressed				
			CP	1BH			; If ESC pressed then
			JR	Z,ESCSET		; jump to the escape set routine
			RET

; Read the keyboard
; Returns:
; - A: ASCII of the pressed key
; - F: Z if the key is pressed, otherwise NZ
;
READKEY:		LD	A, (KEYDOWN)		; Get key down
			DEC	A 			; Set Z flag if keydown is 1
			LD	A, (KEYASCII)		; Get key ASCII value
			RET 
;
; TRAP
; This is called whenever BASIC needs to check for ESC
;
TRAP:			CALL	ESCTEST			; Read keyboard, test for ESC, set FLAGS
;
LTRAP:			LD	A,(FLAGS)		; Get FLAGS
			OR	A			; This checks for bit 7; if it is not set then the result will
			RET	P			; be positive (bit 7 is the sign bit in Z80), so return
LTRAP1:			LD	HL,FLAGS 		; Escape is pressed at this point, so
			RES	7,(HL)			; Clear the escape pressed flag and
			JP	ESCAPE			; Jump to the ESCAPE error routine in exec.asm

;OSINIT - Initialise RAM mapping etc.
;If BASIC is entered by BBCBASIC FILENAME then file
;FILENAME.BBC is automatically CHAINed.
;   Outputs: DE = initial value of HIMEM (top of RAM)
;            HL = initial value of PAGE (user program)
;            Z-flag reset indicates AUTO-RUN.
;  Destroys: A,D,E,H,L,F
;
OSINIT:			CALL	VBLANK_INIT
			XOR	A
			LD 	HL, USER
			LD	DE, RAM_Top
			LD	E, A			; Page boundary
			RET	

;
;OSCLI - Process a MOS command
;
OSCLI: 			CALL    SKIPSP
			CP      CR
			RET     Z
			CP      '|'
			RET     Z
			EX      DE,HL
			LD      HL,COMDS
OSCLI0:			LD      A,(DE)
			CALL    UPPRC
			CP      (HL)
			JR      Z,OSCLI2
			JR      C,OSCLI6
OSCLI1:			BIT     7,(HL)
			INC     HL
			JR      Z,OSCLI1
			INC     HL
			INC     HL
			JR      OSCLI0
;
OSCLI2:			PUSH    DE
OSCLI3:			INC     DE
			INC     HL
			LD      A,(DE)
			CALL    UPPRC
			CP      '.'			; ABBREVIATED?
			JR      Z,OSCLI4
			XOR     (HL)
			JR      Z,OSCLI3
			CP      80H
			JR      Z,OSCLI4
			POP     DE
			JR      OSCLI1
;
OSCLI4:			POP     AF
		        INC     DE
OSCLI5:			BIT     7,(HL)
			INC     HL
			JR      Z,OSCLI5
			LD      A,(HL)
			INC     HL
			LD      H,(HL)
			LD      L,A
			PUSH    HL
			EX      DE,HL
			JP      SKIPSP
;
OSCLI6:			EX	DE, HL			; HL: Buffer for command
			LD	DE, ACCS		; Buffer for command string is ACCS (the string accumulator)
			PUSH	DE			; Store buffer address
			CALL	CSTR_LINE		; Fetch the line
			POP	HL			; HL: Pointer to command string in ACCS
			PUSH	IY
			MOSCALL	mos_oscli		; Returns OSCLI error in A
			POP	IY
			OR	A			; 0 means MOS returned OK
			RET	Z			; So don't do anything
			JP 	OSERROR			; Otherwise it's a MOS error

HUH:    		LD      A,254			; Bad command error
        		CALL    EXTERR
        		DB    	"Bad command"
        		DEFB    0			

SKIPSP:			LD      A,(HL)			
        		CP      ' '
        		RET     NZ
        		INC     HL
        		JR      SKIPSP	

UPPRC:  		AND     7FH
			CP      '`'
			RET     C
			AND     5FH			; CONVERT TO UPPER CASE
			RET					

; Each command has bit 7 of the last character set, and is followed by the address of the handler
; These must be in alphabetical order
;		
; BEGIN NOT FOUND IN BINARY
; COMDS:  		DB	"AS","M"+80h		; ASM
; 			DW	STAR_ASM
; 			DB	"BY","E"+80h		; BYE
; 			DW	STAR_BYE
; 			DB	"EDI","T"+80h		; EDIT
; 			DW	STAR_EDIT
; 			DB	"F","X"+80h		; FX
; 			DW	STAR_FX
; 			DB	"VERSIO","N"+80h	; VERSION
; 			DW	STAR_VERSION
; 			DB	FFh
; END NOT FOUND IN BINARY
; BEGIN INSERTED FROM BINARY
; Each command has bit 7 of the last character set, and is followed by the address of the handler
; These must be in alphabetical order
;
COMDS:
	db 0x42 ; 044013 41     11404 COMDS:  		DB	"AS","M"+80h		; ASM
	db 0x59 ; 044014
	db 0xc5 ; 044015
	db 0x2c ; 044016 31     11405 DW	STAR_ASM
	db 0x40 ; 044017
	db 0x45 ; 044018 42     11406 DB	"BY","E"+80h		; BYE
	db 0x44 ; 044019
	db 0x49 ; 04401A
	db 0xd4 ; 04401B 3D     11407 DW	STAR_BYE
	db 0x61 ; 04401C
	db 0x40 ; 04401D 45     11408 DB	"EDI","T"+80h		; EDIT
	db 0x46 ; 04401E
	db 0xd8 ; 04401F
	db 0xa1 ; 044020
	db 0x40 ; 044021 72     11409 DW	STAR_EDIT
	db 0x56 ; 044022
	db 0x45 ; 044023 46     11410 DB	"F","X"+80h		; FX
	db 0x52 ; 044024
	db 0x53 ; 044025 B2     11411 DW	STAR_FX
	db 0x49 ; 044026
	db 0x4f ; 044027 56     11412 DB	"VERSIO","N"+80h	; VERSION
	db 0xce ; 044028
	db 0x38 ; 044029
	db 0x40 ; 04402A
	db 0xff ; 04402B
; END INSERTED FROM BINARY

; BEGIN NOT FOUND IN BINARY						
; ; *ASM string
; ;
; STAR_ASM:		PUSH	IY			; Stack the BASIC pointer
; 			PUSH	HL			; HL = IY
; 			POP	IY
; 			CALL	ASSEM			; Invoke the assembler
; 			POP	IY
; 			RET
; END NOT FOUND IN BINARY

; *BYE
;
STAR_BYE:		CALL	VBLANK_STOP		; Restore MOS interrupts
			LD	HL, 0			; The return value
			JP	_end 			; Jump back to the end routine in init.asm
	
; *VERSION
;
STAR_VERSION:		CALL    TELL			; Output the welcome message
			DB    	"BBC BASIC (Agon ADL) Version 1.03\n\r",0
			RET
	
; *EDIT linenum
;
STAR_EDIT:		CALL	ASC_TO_NUMBER		; DE: Line number to edit
			EX	DE, HL			; HL: Line number
			CALL	FINDL			; HL: Address in RAM of tokenised line			
			LD	A, 41			; F:NZ If the line is not found
			JP	NZ, ERROR_		; Do error 41: No such line in that case
;
; Use LISTIT to output the line to the ACCS buffer
;
			INC	HL			; Skip the length byte
			LD	E, (HL)			; Fetch the line number
			INC	HL
			LD	D, (HL)
			INC	HL
			LD	IX, ACCS		; Pointer to where the copy is to be stored
			LD	(OSWRCHPT), IX
			LD	IX, LISTON		; Pointer to LISTON variable in RAM
			LD	A, (IX)			; Store that variable
			PUSH	AF
			LD	(IX), 09h		; Set to echo to buffer
			CALL	LISTIT
			POP	AF
			LD	(IX), A			; Restore the original LISTON variable			
			LD	HL, ACCS		; HL: ACCS
			LD	E, L			;  E: 0 - Don't clear the buffer; ACCS is on a page boundary so L is 0
			CALL	OSLINE1			; Invoke the editor
			JP	ONEDIT			; Jump back to the BASIC loop just after the normal line edit

; OSCLI FX n
;
STAR_FX:		CALL	ASC_TO_NUMBER
			LD	C, E			; C: Save FX #
			CALL	ASC_TO_NUMBER
			LD	A, D  			; Is first parameter > 255?
			OR 	A 			
			JR	Z, STAR_FX1		; Yes, so skip next bit 
			EX	DE, HL 			; Parameter is 16-bit
			JR	STAR_FX2 
;
STAR_FX1:		LD	B, E 			; B: Save First parameter
			CALL	ASC_TO_NUMBER		; Fetch second parameter
			LD	L, B 			; L: First parameter
			LD	H, E 			; H: Second parameter
;
STAR_FX2:		LD	A, C 			; A: FX #, and fall through to OSBYTE	
;
; OSBYTE
;  A: FX #
;  L: First parameter
;  H: Second parameter
;
OSBYTE:			CP	0BH			; *FX 11, n: Keyboard auto-repeat delay
			JR	Z, OSBYTE_0B
			CP	0CH			; *FX 12, n: Keyboard auto-repeat rate
			JR	Z, OSBYTE_0C
			CP	13H			; *FX 19: Wait for vblank
			JR	Z, OSBYTE_13		
			CP	76H			; *FX 118, n: Set keyboard LED
			JP	Z, OSBYTE_76
			CP	A0H
			JP	Z, OSBYTE_A0		
			JP	HUH			; Anything else trips an error

; OSBYTE 0x0B (FX 11,n): Keyboard auto-repeat delay
; Parameters:
; - HL: Repeat delay
;
OSBYTE_0B:		VDU	23
			VDU	0
			VDU	vdp_keystate
			VDU	L
			VDU	H 
			VDU	0
			VDU 	0
			VDU	255
			RET 

; OSBYTE 0x0C (FX 12,n): Keyboard auto-repeat rate
; Parameters:
; - HL: Repeat rate
;
OSBYTE_0C:		VDU	23
			VDU	0
			VDU	vdp_keystate
			VDU	0
			VDU 	0
			VDU	L
			VDU	H 
			VDU	255
			RET 

; OSBYTE 0x13 (FX 19): Wait for vertical blank interrupt
;
OSBYTE_13:		CALL	WAIT_VBLANK
			LD	L, 0			; Returns 0
			JP	COUNT0
;
WAIT_VBLANK:		PUSH 	IX			; Wait for VBLANK interrupt
			MOSCALL	mos_sysvars		; Fetch pointer to system variables
			LD	A, (IX + sysvar_time + 0)
@@:			CP 	A, (IX + sysvar_time + 0)
			JR	Z, @B
			POP	IX
			RET

; OSBYTE 0x76 (FX 118,n): Set Keyboard LED
; Parameters:
; - L: LED (Bit 0: Scroll Lock, Bit 1: Caps Lock, Bit 2: Num Lock)
;
OSBYTE_76:		VDU	23
			VDU	0
			VDU	vdp_keystate
			VDU	0
			VDU 	0
			VDU	0
			VDU	0 
			VDU	L
			RET 
			
; OSBYTE 0xA0: Fetch system variable
; Parameters:
; - L: The system variable to fetch
;
OSBYTE_A0:		PUSH	IX
			MOSCALL	mos_sysvars		; Fetch pointer to system variables
			LD	BC, 0			
			LD	C, L			; BCU = L
			ADD	IX, BC			; Add to IX
			LD	L, (IX + 0)		; Fetch the return value
			POP	IX
			JP 	COUNT0

;OSLOAD - Load an area of memory from a file.
;   Inputs: HL addresses filename (CR terminated)
;           DE = address at which to load
;           BC = maximum allowed size (bytes)
;  Outputs: Carry reset indicates no room for file.
; Destroys: A,B,C,D,E,H,L,F
;
OSLOAD:			PUSH	BC			; Stack the size
			PUSH	DE			; Stack the load address
			LD	DE, ACCS		; Buffer address for filename
			CALL	CSTR_FNAME		; Fetch filename from MOS into buffer
			LD	HL, ACCS		; HL: Filename
			CALL	EXT_DEFAULT		; Tack on the extension .BBC if not specified
			CALL	EXT_HANDLER		; Get the default handler
			POP	DE			; Restore the load address
			POP	BC			; Restore the size
			OR	A
			JR 	Z, OSLOAD_BBC
;
; Load the file in as a text file
;
OSLOAD_TXT:		XOR	A			; Set file attributes to read
			CALL	OSOPEN			; Open the file			
			LD 	E, A 			; The filehandle
			OR	A
			LD	A, 4			; File not found error
			JR	Z, OSERROR		; Jump to error handler
			CALL	NEWIT			; Call NEW to clear the program space
;
OSLOAD_TXT1:		LD	HL, ACCS 		; Where the input is going to be stored
;
; First skip any whitespace (indents) at the beginning of the input
;
@@:			CALL	OSBGET			; Read the byte into A
			JR	C, OSLOAD_TXT3		; Is it EOF?
			CP	LF 			; Is it LF?
			JR	Z, OSLOAD_TXT3 		; Yes, so skip to the next line
			CP	21h			; Is it less than or equal to ASCII space?
			JR	C, @B 			; Yes, so keep looping
			LD	(HL), A 		; Store the first character
			INC	L
;
; Now read the rest of the line in
;
OSLOAD_TXT2:		CALL	OSBGET			; Read the byte into A
			JR	C, OSLOAD_TXT4		; Is it EOF?
			CP	20h			; Skip if not an ASCII character
			JR	C, @F
			LD	(HL), A 		; Store in the input buffer			
			INC	L			; Increment the buffer pointer
			JP	Z, BAD			; If the buffer is full (wrapped to 0) then jump to Bad Program error
@@:			CP	LF			; Check for LF
			JR	NZ, OSLOAD_TXT2		; If not, then loop to read the rest of the characters in
;
; Finally, handle EOL/EOF
;
OSLOAD_TXT3:		LD	(HL), CR		; Store a CR for BBC BASIC
			LD	A, L			; Check for minimum line length
			CP	2			; If it is 2 characters or less (including CR)
			JR	C, @F			; Then don't bother entering it
			PUSH	DE			; Preserve the filehandle
			CALL	ONEDIT1			; Enter the line in memory
			CALL	C,CLEAN			; If a new line has been entered, then call CLEAN to set TOP and write &FFFF end of program marker
			POP	DE
@@:			CALL	OSSTAT			; End of file?
			JR	NZ, OSLOAD_TXT1		; No, so loop
			CALL	OSSHUT			; Close the file
			SCF				; Flag to BASIC that we're good
			RET
;
; Special case for BASIC programs with no blank line at the end
;
OSLOAD_TXT4:		CP	20h			; Skip if not an ASCII character
			JR	C, @F
			LD	(HL), A			; Store the character
			INC	L
			JP	Z, BAD
@@:			JR	OSLOAD_TXT3
			
;
; Load the file in as a tokenised binary blob
;
OSLOAD_BBC:		MOSCALL	mos_load		; Call LOAD in MOS
			RET	NC			; If load returns with carry reset - NO ROOM
			OR	A			; If there is no error (A=0)
			SCF				; Need to set carry indicating there was room
			RET	Z			; Return
;
OSERROR:		PUSH	AF			; Handle the MOS error
			LD	HL, ACCS		; Address of the buffer
			LD	BC, 256			; Length of the buffer
			LD	E, A			; The error code
			MOSCALL	mos_getError		; Copy the error message into the buffer
			POP	AF			
			PUSH	HL			; Stack the address of the error (now in ACCS)		
			ADD	A, 127			; Add 127 to the error code (MOS errors start at 128, and are trappable)
			JP	EXTERR			; Trigger an external error

;OSSAVE - Save an area of memory to a file.
;   Inputs: HL addresses filename (term CR)
;           DE = start address of data to save
;           BC = length of data to save (bytes)
; Destroys: A,B,C,D,E,H,L,F
;
OSSAVE:			PUSH	BC			; Stack the size
			PUSH	DE			; Stack the save address
			LD	DE, ACCS		; Buffer address for filename
			CALL	CSTR_FNAME		; Fetch filename from MOS into buffer
			LD	HL, ACCS		; HL: Filename
			CALL	EXT_DEFAULT		; Tack on the extension .BBC if not specified
			CALL	EXT_HANDLER		; Get the default handler
			POP	DE			; Restore the save address
			POP	BC			; Restore the size
			OR	A			; Is the extension .BBC
			JR	Z, OSSAVE_BBC		; Yes, so use that
;
; Save the file out as a text file
;
OSSAVE_TXT:		LD 	A, (OSWRCHCH)		; Stack the current channel
			PUSH	AF
			XOR	A
			INC	A			; Make sure C is clear, A is 1, for OPENOUT
			LD	(OSWRCHCH), A
			CALL	OSOPEN			; Open the file
			LD	(OSWRCHFH), A		; Store the file handle for OSWRCH
			LD	IX, LISTON		; Required for LISTIT
			LD	HL, (PAGE_)		; Get start of program area
			EXX
			LD	BC, 0			; Set the initial indent counters
			EXX			
OSSAVE_TXT1:		LD	A, (HL)			; Check for end of program marker
			OR	A		
			JR	Z, OSSAVE_TXT2
			INC	HL			; Skip the length byte
			LD	DE, 0			; Clear DE to ensure we get a 16-bit line number
			LD	E, (HL)			; Get the line number
			INC	HL
			LD	D, (HL)
			INC	HL
			CALL	LISTIT			; List the line
			JR	OSSAVE_TXT1
OSSAVE_TXT2:		LD	A, (OSWRCHFH)		; Get the file handle
			LD	E, A
			CALL	OSSHUT			; Close it
			POP	AF			; Restore the channel
			LD	(OSWRCHCH), A		
			RET
;
; Save the file out as a tokenised binary blob
;
OSSAVE_BBC:		MOSCALL	mos_save		; Call SAVE in MOS
			OR	A			; If there is no error (A=0)
			RET	Z			; Just return
			JR	OSERROR			; Trip an error

; Check if an extension is specified in the filename
; Add a default if not specified
; HL: Filename (CSTR format)
;
EXT_DEFAULT:		PUSH	HL			; Stack the filename pointer	
			LD	C, '.'			; Search for dot (marks start of extension)
			CALL	CSTR_FINDCH
			OR	A			; Check for end of string marker
			JR	NZ, @F			; No, so skip as we have an extension at this point			
			LD	DE, EXT_LOOKUP		; Get the first (default extension)
			CALL	CSTR_CAT		; Concat it to string pointed to by HL
@@:			POP	HL			; Restore the filename pointer
			RET
			
; Check if an extension is valid and, if so, provide a pointer to a handler
; HL: Filename (CSTR format)
; Returns:
;  A: Filename extension type (0=BBC tokenised, 1=ASCII untokenised)
;
EXT_HANDLER:		PUSH	HL			; Stack the filename pointer
			LD	C, '.'			; Find the '.'
			CALL	CSTR_FINDCH
			LD	DE, EXT_LOOKUP		; The lookup table
;
EXT_HANDLER_1:		PUSH	HL			; Stack the pointer to the extension
			CALL	CSTR_ENDSWITH		; Check whether the string ends with the entry in the lookup
			POP	HL			; Restore the pointer to the extension
			JR	Z, EXT_HANDLER_2	; We have a match!
;
@@:			LD	A, (DE)			; Skip to the end of the entry in the lookup
			INC	DE
			OR	A
			JR	NZ, @B
			INC	DE			; Skip the file extension # byte
;
			LD	A, (DE)			; Are we at the end of the table?
			OR	A
			JR	NZ, EXT_HANDLER_1	; No, so loop
;			
			LD      A,204			; Throw a "Bad name" error
        		CALL    EXTERR
        		DB    	"Bad name", 0
;
EXT_HANDLER_2:		INC	DE			; Skip to the file extension # byte
			LD	A, (DE)		
			POP	HL			; Restore the filename pointer
			RET
;


; Extension lookup table
; CSTR, TYPE
; 	- 0: BBC (tokenised BBC BASIC for Z80 format)
; 	- 1: Human readable plain text
;
EXT_LOOKUP:		DB	".BBC", 0, 0		; First entry is the default extension
			DB	".TXT", 0, 1
			DB	".ASC", 0, 1
			DB	".BAS", 0, 1
			DB	0			; End of table
			
;OSCALL - Intercept page &FF calls and provide an alternative address
;
;&FFF7:	OSCLI	Execute *command.
;&FFF4:	OSBYTE	Various byte-wide functions.
;&FFF1:	OSWORD	Various control block functions.
;&FFEE:	OSWRCH	Write character to output stream.
;&FFE7:	OSNEWL	Write NewLine to output stream.
;&FFE3:	OSASCI	Write character or NewLine to output stream.
;&FFE0:	OSRDCH	Wait for character from input stream.
;&FFDD:	OSFILE	Perform actions on whole files or directories.
;&FFDA:	OSARGS	Read and write information on open files or filing systems.
;&FFD7:	OSBGET	Read a byte from an a channel.
;&FFD4:	OSBPUT	Write a byte to a channel.
;&FFD1:	OSGBPB	Read and write blocks of data.
;&FFCE:	OSFIND	Open or close a file.
;
OSCALL:			LD	HL, OSCALL_TABLE
OSCALL_1:		LD	A, (HL)
			INC	HL
			CP	FFh
			RET	Z 
			CP	A, IYL
			JR	Z, OSCALL_2
			RET	NC
			INC	HL 
			INC	HL 
			INC	HL
			JR	OSCALL_1
OSCALL_2:		LD	IY,(HL)
			RET
OSCALL_TABLE:		DB 	D4h
			DW24 	OSBPUT
			DB 	D7h
			DW24 	OSBGET
			DB 	EEh
			DW24 	OSWRCH
			DB	F4h
			DW24 	OSBYTE
			DB	F7h
			DW24	OSCLI
			DB	FFh	

; OSOPEN
; HL: Pointer to path
;  F: C Z
;     x x OPENIN
; 	  OPENOUT
;     x	  OPENUP
; Returns:
;  A: Filehandle, 0 if cannot open
;
OSOPEN:			LD	C, fa_read
			JR	Z, @F
			LD	C, fa_write | fa_open_append
			JR	C, @F
			LD	C, fa_write | fa_create_always
@@:			MOSCALL	mos_fopen			
			RET

;OSSHUT - Close disk file(s).
; E = file channel
;  If E=0 all files are closed (except SPOOL)
; Destroys: A,B,C,D,E,H,L,F
;
OSSHUT:			PUSH	BC
			LD	C, E
			MOSCALL	mos_fclose
			POP	BC
			RET
	
; OSBGET - Read a byte from a random disk file.
;  E = file channel
; Returns
;  A = byte read
;  Carry set if LAST BYTE of file
; Destroys: A,B,C,F
;
OSBGET:			PUSH	BC
			LD	C, E
			MOSCALL	mos_fgetc
			POP	BC
			RET
	
; OSBPUT - Write a byte to a random disk file.
;  E = file channel
;  A = byte to write
; Destroys: A,B,C,F
;	
OSBPUT:			PUSH	BC
			LD	C, E
			LD	B, A
			MOSCALL	mos_fputc
			POP	BC
			RET

; OSSTAT - Read file status
;  E = file channel
; Returns
;  F: Z flag set - EOF
;  A: If Z then A = 0
; Destroys: A,D,E,H,L,F
;
OSSTAT:			PUSH	BC
			LD	C, E
			MOSCALL	mos_feof
			POP	BC
			CP	1
			RET
	
; GETPTR - Return file pointer.
;    E = file channel
; Returns:
; DEHL = pointer (0-&7FFFFF)
; Destroys: A,B,C,D,E,H,L,F
;
GETPTR:			PUSH		IY
			LD		C, E 
			MOSCALL		mos_getfil 	; HLU: Pointer to FIL structure
			PUSH		HL
			POP		IY		; IYU: Pointer to FIL structure
			LD		L, (IY + FIL.fptr + 0)
			LD		H, (IY + FIL.fptr + 1)
			LD		E, (IY + FIL.fptr + 2)
			LD		D, (IY + FIL.fptr + 3)
			POP		IY
			RET

; PUTPTR - Update file pointer.
;    A = file channel
; DEHL = new pointer (0-&7FFFFF)
; Destroys: A,B,C,D,E,H,L,F
;
PUTPTR:			PUSH		IY 			
			LD		C, A  		; C: Filehandle
			PUSH		HL 		
			LD		HL, 2
			ADD		HL, SP
			LD		(HL), E 	; 3rd byte of DWORD set to E
			POP		HL
			LD		E, D  		; 4th byte passed as E
			MOSCALL		mos_flseek
			POP		IY 
			RET
	
; GETEXT - Find file size.
;    E = file channel
; Returns:
; DEHL = file size (0-&800000)
; Destroys: A,B,C,D,E,H,L,F
;
GETEXT:         PUSH    IY 
                LD      C, E 
                MOSCALL mos_getfil  ; HLU: Pointer to FIL structure
                PUSH    HL
                POP     IY          ; IYU: Pointer to FIL structure
                ; Access the obj.objsize field using the offset values
                LD      L, (IY + FIL.obj + FFOBJID.objsize + 0)
                LD      H, (IY + FIL.obj + FFOBJID.objsize + 1)
                LD      E, (IY + FIL.obj + FFOBJID.objsize + 2)
                LD      D, (IY + FIL.obj + FFOBJID.objsize + 3)            

                POP     IY 
			RET	

; GETIMS - Get time from RTC
;
GETIMS:			PUSH	IY
			LD	HL, ACCS 		; Where to store the time string
			MOSCALL	mos_getrtc
			LD	DE, ACCS		; DE: pointer to start of string accumulator
			LD	E, A 			;  E: now points to the end of the string
			POP	IY
			RET 
	
; Get two word values from EXPR in DE, HL
; IY: Pointer to expression string
; Returns:
; DE: P1
; HL: P2
;
EXPR_W2:		CALL	EXPRI			; Get first parameter	
			EXX
			PUSH	HL
			CALL	COMMA 
			CALL	EXPRI			; Get second parameter
			EXX
			POP	DE
			RET

; Stuff not implemented yet
;
RESET:			RET; --- End patch.asm ---

; --- Begin sorry.asm ---
;
; Title:	BBC Basic Interpreter - Z80 version
;		Catch-all for unimplemented functionality
; Author:	Dean Belfield
; Created:	12/05/2023
; Last Updated:	12/05/2023
;
; Modinfo:

			; .ASSUME	ADL = 1

			; SEGMENT CODE
			
			; XDEF	ENVEL
			; XDEF	ADVAL
			; XDEF	PUTIMS
			
			; XREF	EXTERR
			
ENVEL:
ADVAL:
PUTIMS:
			XOR     A
			CALL    EXTERR
			DEFB    "Sorry"
			DEFB    0
; --- End sorry.asm ---

; --- Begin agon_graphics.asm ---
;
; Title:	BBC Basic for AGON - Graphics stuff
; Author:	Dean Belfield
; Created:	12/05/2023
; Last Updated:	07/06/2023
;
; Modinfo:
; 07/06/2023:	Modified to run in ADL mode
			
			; .ASSUME	ADL = 1
				
			; INCLUDE	"equs.inc"
			; INCLUDE "macros.inc"
			; INCLUDE "mos_api.inc"	; In MOS/src
		
			; SEGMENT CODE
				
			; XDEF	CLG
			; XDEF	CLRSCN
			; XDEF	MODE
			; XDEF	COLOUR
			; XDEF	GCOL
			; XDEF	MOVE
			; XDEF	PLOT
			; XDEF	DRAW
			; XDEF	POINT
			; XDEF	GETSCHR
			
			; XREF	OSWRCH
			; XREF	ASC_TO_NUMBER
			; XREF	EXTERR
			; XREF	EXPRI
			; XREF	COMMA
			; XREF	XEQ
			; XREF	NXT
			; XREF	BRAKET
			; XREF	COUNT0
			; XREF	CRTONULL
			; XREF	NULLTOCR
			; XREF	CRLF
			; XREF	EXPR_W2
			; XREF	INKEY1

; CLG: clears the graphics area
;
CLG:			

			VDU	10h
			JP	XEQ

; CLS: clears the text area
;
CLRSCN:			LD	A, 0Ch
			JP	OSWRCH
				
; MODE n: Set video mode
;
MODE:			PUSH	IX			; Get the system vars in IX
			MOSCALL	mos_sysvars		; Reset the semaphore
			RES	4, (IX+sysvar_vpd_pflags)
			CALL    EXPRI
			EXX
			VDU	16H			; Mode change
			VDU	L
			MOSCALL	mos_sysvars		
@@:			BIT	4, (IX+sysvar_vpd_pflags)
			JR	Z, @B			; Wait for the result			
			POP	IX
			JP	XEQ
			
; GET(x,y): Get the ASCII code of a character on screen
;
GETSCHR:		INC	IY
			CALL    EXPRI      		; Get X coordinate
			EXX
			LD	(VDU_BUFFER+0), HL
			CALL	COMMA		
			CALL	EXPRI			; Get Y coordinate
			EXX 
			LD	(VDU_BUFFER+2), HL
			CALL	BRAKET			; Closing bracket		
;
			PUSH	IX			; Get the system vars in IX
			MOSCALL	mos_sysvars		; Reset the semaphore
			RES	1, (IX+sysvar_vpd_pflags)
			VDU	23
			VDU	0
			VDU	vdp_scrchar
			VDU	(VDU_BUFFER+0)
			VDU	(VDU_BUFFER+1)
			VDU	(VDU_BUFFER+2)
			VDU	(VDU_BUFFER+3)
@@:			BIT	1, (IX+sysvar_vpd_pflags)
			JR	Z, @B			; Wait for the result
			LD	A, (IX+sysvar_scrchar)	; Fetch the result in A
			OR	A			; Check for 00h
			SCF				; C = character map
			JR	NZ, @F			; We have a character, so skip next bit
			XOR	A			; Clear carry
			DEC	A			; Set A to FFh
@@:			POP	IX			
			JP	INKEY1			; Jump back to the GET command

; POINT(x,y): Get the pixel colour of a point on screen
;
POINT:			CALL    EXPRI      		; Get X coordinate
			EXX
			LD	(VDU_BUFFER+0), HL
			CALL	COMMA		
			CALL	EXPRI			; Get Y coordinate
			EXX 
			LD	(VDU_BUFFER+2), HL
			CALL	BRAKET			; Closing bracket		
;
			PUSH	IX			; Get the system vars in IX
			MOSCALL	mos_sysvars		; Reset the semaphore
			RES	2, (IX+sysvar_vpd_pflags)
			VDU	23
			VDU	0
			VDU	vdp_scrpixel
			VDU	(VDU_BUFFER+0)
			VDU	(VDU_BUFFER+1)
			VDU	(VDU_BUFFER+2)
			VDU	(VDU_BUFFER+3)
@@:			BIT	2, (IX+sysvar_vpd_pflags)
			JR	Z, @B			; Wait for the result
;
; Return the data as a 1 byte index
;
			LD	L, (IX+sysvar_scrpixelIndex)
			POP	IX	
			JP	COUNT0


; COLOUR colour
; COLOUR L,P
; COLOUR L,R,G,B
;
COLOUR:			CALL	EXPRI			; The colour / mode
			EXX
			LD	A, L 
			LD	(VDU_BUFFER+0), A	; Store first parameter
			CALL	NXT			; Are there any more parameters?
			CP	','
			JR	Z, COLOUR_1		; Yes, so we're doing a palette change next
;
			VDU	11h			; Just set the colour
			VDU	(VDU_BUFFER+0)
			JP	XEQ			
;
COLOUR_1:		CALL	COMMA
			CALL	EXPRI			; Parse R (OR P)
			EXX
			LD	A, L
			LD	(VDU_BUFFER+1), A
			CALL	NXT			; Are there any more parameters?
			CP	','
			JR	Z, COLOUR_2		; Yes, so we're doing COLOUR L,R,G,B
;
			VDU	13h			; VDU:COLOUR
			VDU	(VDU_BUFFER+0)		; Logical Colour
			VDU	(VDU_BUFFER+1)		; Palette Colour
			VDU	0			; RGB set to 0
			VDU	0
			VDU	0
			JP	XEQ
;
COLOUR_2:		CALL	COMMA
			CALL	EXPRI			; Parse G
			EXX
			LD	A, L
			LD	(VDU_BUFFER+2), A
			CALL	COMMA
			CALL	EXPRI			; Parse B
			EXX
			LD	A, L
			LD	(VDU_BUFFER+3), A							
			VDU	13h			; VDU:COLOUR
			VDU	(VDU_BUFFER+0)		; Logical Colour
			VDU	FFh			; Physical Colour (-1 for RGB mode)
			VDU	(VDU_BUFFER+1)		; R
			VDU	(VDU_BUFFER+2)		; G
			VDU	(VDU_BUFFER+3)		; B
			JP	XEQ

; GCOL mode,colour
;
GCOL:			CALL	EXPRI			; Parse MODE
			EXX
			LD	A, L 
			LD	(VDU_BUFFER+0), A	
			CALL	COMMA
;
			CALL	EXPRI			; Parse Colour
			EXX
			LD	A, L
			LD	(VDU_BUFFER+1), A
;
			VDU	12h			; VDU:GCOL
			VDU	(VDU_BUFFER+0)		; Mode
			VDU	(VDU_BUFFER+1)		; Colour
			JP	XEQ
			
; PLOT mode,x,y
;
PLOT:			CALL	EXPRI		; Parse mode
			EXX					
			PUSH	HL		; Push mode (L) onto stack
			CALL	COMMA 	
			CALL	EXPR_W2		; Parse X and Y
			POP	BC		; Pop mode (C) off stack
PLOT_1:			VDU	19H		; VDU code for PLOT				
			VDU	C		;  C: Mode
			VDU	E		; DE: X
			VDU	D
			VDU	L		; HL: Y
			VDU	H
			JP	XEQ

; MOVE x,y
;
MOVE:			CALL	EXPR_W2		; Parse X and Y
			LD	C, 04H		; Plot mode 04H (Move)
			JR	PLOT_1		; Plot

; DRAW x1,y1
; DRAW x1,y1,x2,y2
;
DRAW:			CALL	EXPR_W2		; Get X1 and Y1
			CALL	NXT		; Are there any more parameters?
			CP	','
			LD	C, 05h		; Code for LINE
			JR	NZ, PLOT_1	; No, so just do DRAW x1,y1
			VDU	19h		; Move to the first coordinates
			VDU	04h
			VDU	E
			VDU	D
			VDU	L
			VDU	H
			CALL	COMMA
			PUSH	BC
			CALL	EXPR_W2		; Get X2 and Y2
			POP	BC
			JR	PLOT_1		; Now DRAW the line to those positions
			
			
			
; --- End agon_graphics.asm ---

; --- Begin agon_sound.asm ---
;
; Title:	BBC Basic for AGON - Audio stuff
; Author:	Dean Belfield
; Created:	12/05/2023
; Last Updated:	12/05/2023
;
; Modinfo:
			
			; .ASSUME	ADL = 1
				
			; INCLUDE	"equs.inc"
			; INCLUDE "macros.inc"
			; INCLUDE "mos_api.inc"	; In MOS/src
		
			; SEGMENT CODE
			
			; XDEF	SOUND
			
			; XREF	COMMA
			; XREF	EXPR_W2
			; XREF	XEQ
			; XREF	LTRAP
			; XREF	OSWRCH
			; XREF	VDU_BUFFER
			
				
; SOUND channel,volume,pitch,duration
; volume: 0 (off) to -15 (full volume)
; pitch: 0 - 255
; duration: -1 to 254 (duration in 20ths of a second, -1 = play forever)
;
SOUND:			CALL	EXPR_W2			; DE: Channel/Control, HL: Volume
			LD	A, L 			;  A: Volume
			PUSH	AF
			PUSH	DE
			CALL	COMMA
			CALL	EXPR_W2			; DE: Pitch, HL: Duration
			LD	D, E			;  D: Pitch
			LD	E, L 			;  E: Duration
			POP	HL 			; HL: Channel/Control
			POP	AF
			NEG
			CP	16			; Check volume is in bounds
			JP	NC, XEQ			; Out of bounds, do nothing
;
; Store	in VDU vars
; 
			LD	C, A			; Store Volume in C
			LD	A, L
			LD	(VDU_BUFFER+0), A	; Channel
			XOR	A
			LD	(VDU_BUFFER+1), A	; Waveform
; 
; Calculate the volume
; 
			LD	B, 6			; C already contains the volume
			MLT	BC			; Multiply by 6 (0-15 scales to 0-90)
			LD	A, C
			LD	(VDU_BUFFER+2), A
;
; And the frequency
;
			LD	C, E			; Store duration in C
			LD	H, 0			; Lookup the frequency
			LD	L, D
			LD	DE, SOUND_FREQ_LOOKUP
			ADD	HL, HL
			ADD	HL, DE
			LD	A, (HL)
			LD	(VDU_BUFFER+3), A
			INC	HL
			LD	A, (HL)
			LD	(VDU_BUFFER+4), A
;
; And now the duration - multiply it by 50 to convert from 1/20ths of seconds to milliseconds
;
			LD	B, 50			; C contains the duration, so MLT by 50
			MLT	BC
			LD	(VDU_BUFFER+5), BC
;
			PUSH	IX			; Get the system vars in IX
			MOSCALL	mos_sysvars		; Reset the semaphore
SOUND0:			RES.LIL	3, (IX+sysvar_vpd_pflags)
;
			VDU	23			; Send the sound command
			VDU	0
			VDU	vdp_audio
			VDU	(VDU_BUFFER+0)		; 0: Channel
			VDU	(VDU_BUFFER+1)		; 1: Waveform (0)
			VDU	(VDU_BUFFER+2)		; 2: Volume (0-100)
			VDU	(VDU_BUFFER+3)		; 3: Frequency L
			VDU	(VDU_BUFFER+4)		; 4: Frequency H
			VDU	(VDU_BUFFER+5)		; 5: Duration L
			VDU	(VDU_BUFFER+6)		; 6: Duration H
;
; Wait for acknowledgement
;
@@:			BIT.LIL	3, (IX+sysvar_vpd_pflags)
			JR	Z, @B			; Wait for the result
			CALL	LTRAP			; Check for ESC
			LD.LIL	A, (IX+sysvar_audioSuccess)
			AND	A			; Check if VDP has queued the note
			JR	Z, SOUND0		; No, so loop back and send again
;
			POP	IX
			JP	XEQ

; Frequency Lookup Table
; Set up to replicate the BBC Micro audio frequencies
;
; Split over 5 complete octaves, with 53 being middle C
; * C4: 262hz
; + A4: 440hz
;
;	2	3	4	5	6	7	8
;
; B	1	49	97	145	193	241	
; A#	0	45	93	141	189	237	
; A		41	89+	137	185	233	
; G#		37	85	133	181	229	
; G		33	81	129	177	225	
; F#		29	77	125	173	221	
; F		25	73	121	169	217	
; E		21	69	117	165	213	
; D#		17	65	113	161	209	
; D		13	61	109	157	205	253
; C#		9	57	105	153	201	249
; C		5	53*	101	149	197	245
;
SOUND_FREQ_LOOKUP:	DW	 117,  118,  120,  122,  123,  131,  133,  135
			DW	 137,  139,  141,  143,  145,  147,  149,  151
			DW	 153,  156,  158,  160,  162,  165,  167,  170
			DW	 172,  175,  177,  180,  182,  185,  188,  190
			DW	 193,  196,  199,  202,  205,  208,  211,  214
			DW	 217,  220,  223,  226,  230,  233,  236,  240
			DW	 243,  247,  251,  254,  258,  262,  265,  269
			DW	 273,  277,  281,  285,  289,  294,  298,  302
			DW	 307,  311,  316,  320,  325,  330,  334,  339
			DW	 344,  349,  354,  359,  365,  370,  375,  381
			DW	 386,  392,  398,  403,  409,  415,  421,  427
			DW	 434,  440,  446,  453,  459,  466,  473,  480
			DW	 487,  494,  501,  508,  516,  523,  531,  539
			DW	 546,  554,  562,  571,  579,  587,  596,  605
			DW	 613,  622,  631,  641,  650,  659,  669,  679
			DW	 689,  699,  709,  719,  729,  740,  751,  762
			DW	 773,  784,  795,  807,  819,  831,  843,  855
			DW	 867,  880,  893,  906,  919,  932,  946,  960
			DW	 974,  988, 1002, 1017, 1032, 1047, 1062, 1078
			DW	1093, 1109, 1125, 1142, 1158, 1175, 1192, 1210
			DW	1227, 1245, 1263, 1282, 1300, 1319, 1338, 1358
			DW	1378, 1398, 1418, 1439, 1459, 1481, 1502, 1524
			DW	1546, 1569, 1592, 1615, 1638, 1662, 1686, 1711
			DW	1736, 1761, 1786, 1812, 1839, 1866, 1893, 1920
			DW	1948, 1976, 2005, 2034, 2064, 2093, 2123, 2154
			DW	2186, 2217, 2250, 2282, 2316, 2349, 2383, 2418
			DW	2453, 2489, 2525, 2562, 2599, 2637, 2675, 2714
			DW	2754, 2794, 2834, 2876, 2918, 2960, 3003, 3047
			DW	3091, 3136, 3182, 3228, 3275, 3322, 3371, 3420
			DW	3470, 3520, 3571, 3623, 3676, 3729, 3784, 3839
			DW	3894, 3951, 4009, 4067, 4126, 4186, 4247, 4309
			DW	4371, 4435, 4499, 4565, 4631, 4699, 4767, 4836	


; --- End agon_sound.asm ---

; --- Begin interrupts.asm ---
;
; Title:	BBC Basic for AGON - Interrupts
; Author:	Dean Belfield
; Created:	12/05/2023
; Last Updated:	07/06/2023
;
; Modinfo:
; 07/06/2023:	Modified to run in ADL mode

			; .ASSUME	ADL = 1
				
			; INCLUDE	"macros.inc"
			; INCLUDE	"equs.inc"
			; INCLUDE "mos_api.inc"	; In MOS/src

			; SEGMENT CODE
				
			; XDEF	VBLANK_INIT
			; XDEF	VBLANK_STOP
			; XDEF	VBLANK_HANDLER	

			; XREF	ESCSET	
			; XREF	KEYDOWN		; In ram.asm
			; XREF	KEYASCII 	; In ram.asm
			; XREF	KEYCOUNT	; In ram.asm

; Hook into the MOS VBLANK interrupt
;
VBLANK_INIT:		DI
			LD		HL, VBLANK_HANDLER		; this interrupt handler routine who's
			LD		E, 32h				; Set up the VBlank Interrupt Vector
			MOSCALL		mos_setintvector
			; EX		HL, DE 				; DEU: Pointer to the MOS interrupt vector
			ex de,hl
			LD		HL, VBLANK_HANDLER_JP + 1	; Pointer to the JP address in this segment
			LD		(HL), DE			; Self-modify the code
			EI	
			RET

; Unhook the custom VBLANK interrupt
;
VBLANK_STOP:		DI
			LD		HL, VBLANK_HANDLER_JP + 1	; Pointer to the JP address in this segment
			LD		DE, (HL)			
			EX		DE, HL 				; HLU: Address of MOS interrupt vector
			LD		E, 32h
			MOSCALL		mos_setintvector		; Restore the MOS interrupt vector
			EI
			RET 

; A safe LIS call to ESCSET
; 
DO_KEYBOARD:		MOSCALL		mos_sysvars			; Get the system variables
			LD		HL, KEYCOUNT 			; Check whether the keycount has changed
			LD		A, (IX + sysvar_vkeycount)	; by comparing the MOS copy
			CP 		(HL)				; with our local copy
			JR		NZ, DO_KEYBOARD_1		; Yes it has, so jump to the next bit
;
DO_KEYBOARD_0:		XOR		A 				; Clear the keyboard values 
			LD		(KEYASCII), A
			LD		(KEYDOWN), A 
			RET	 					; And return
;
DO_KEYBOARD_1:		LD		(HL), A 			; Store the updated local copy of keycount 
			LD		A, (IX + sysvar_vkeydown)	; Fetch key down value (1 = key down, 0 = key up)
			OR		A 
			JR		Z, DO_KEYBOARD_0		; If it is key up, then clear the keyboard values
;			
			LD		(KEYDOWN), A 			; Store the keydown value
			LD		A, (IX + sysvar_keyascii)	; Fetch key ASCII value
			LD		(KEYASCII), A 			; Store locally
			CP		1Bh				; Is it escape?
			CALL		Z, ESCSET			; Yes, so set the escape flags
			RET						; Return to the interrupt handler

VBLANK_HANDLER:		DI 
			PUSH		AF 
			PUSH		HL
			PUSH		IX
			CALL		DO_KEYBOARD
			POP		IX 
			POP		HL
			POP		AF 
;
; Finally jump to the MOS interrupt
;
VBLANK_HANDLER_JP:	JP		0				; This is self-modified by VBLANK_INIT				; --- End interrupts.asm ---

; --- Begin ram.asm ---
;
; Title:	BBC Basic Interpreter - Z80 version
;		RAM Module for BBC Basic Interpreter
;		For use with Version 2.0 of BBC BASIC
;		Standard CP/M Distribution Version
; Author:	(C) Copyright  R.T.Russell 31-12-1983
; Modified By:	Dean Belfield
; Created:	12/05/2023
; Last Updated:	26/06/2023
;
; Modinfo:
; 06/06/2023:	Modified to run in ADL mode
; 26/06/2023:	Added temporary stores R0 and R1

			; .ASSUME	ADL = 1

			; DEFINE	LORAM, SPACE = ROM
			; SEGMENT LORAM

			; XDEF	ACCS
			; XDEF	BUFFER
			; XDEF	STAVAR
			; XDEF	DYNVAR
			; XDEF	FNPTR
			; XDEF	PROPTR
			; XDEF	PAGE_
			; XDEF	TOP
			; XDEF	LOMEM
			; XDEF 	FREE
			; XDEF	HIMEM
			; XDEF	LINENO
			; XDEF	TRACEN
			; XDEF	AUTONO
			; XDEF	ERRTRP
			; XDEF	ERRTXT
			; XDEF	DATPTR
			; XDEF	ERL
			; XDEF	ERRLIN
			; XDEF	RANDOM
			; XDEF	COUNT
			; XDEF	WIDTH
			; XDEF	ERR
			; XDEF	LISTON
			; XDEF	INCREM
			
			; XDEF	FLAGS
			; XDEF	OSWRCHPT
			; XDEF	OSWRCHCH
			; XDEF	OSWRCHFH
			; XDEF	KEYDOWN 
			; XDEF	KEYASCII
			; XDEF	KEYCOUNT

			; XDEF	R0
			; XDEF	R1
			
			; XDEF	RAM_START
			; XDEF	RAM_END
			; XDEF	USER

end_binary: ;  for assemble.py to know where to truncate the binary file
			ALIGN 		256		; ACCS, BUFFER & STAVAR must be on page boundaries			
RAM_START:		
;
ACCS:           BLKB    256,0             ; String Accumulator
BUFFER:         BLKB    256,0             ; String Input Buffer
STAVAR:         BLKB    27*4,0            ; Static Variables
DYNVAR:         BLKB    54*3,0            ; Dynamic Variable Pointers
FNPTR:          BLKB    3,0               ; Dynamic Function Pointers
PROPTR:         BLKB    3,0               ; Dynamic Procedure Pointers
;
PAGE_:          BLKB    3,0               ; Start of User Program
TOP:            BLKB    3,0               ; First Location after User Program
LOMEM:          BLKB    3,0               ; Start of Dynamic Storage
FREE:           BLKB    3,0               ; First Free Space Byte
HIMEM:          BLKB    3,0               ; First Protected Byte
;
LINENO:         BLKB    3,0               ; Line Number
TRACEN:         BLKB    3,0               ; Trace Flag
AUTONO:         BLKB    3,0               ; Auto Flag
ERRTRP:         BLKB    3,0               ; Error Trap
ERRTXT:         BLKB    2,0               ; Error Message Pointer
DATPTR:         BLKB    2,0               ; Data Pointer
ERL:            BLKB    2,0               ; Error Line
ERRLIN:         BLKB    3,0               ; The "ON ERROR" Line
RANDOM:         BLKB    5,0               ; Random Number
COUNT:          BLKB    1,0               ; Print Position
WIDTH:          BLKB    1,0               ; Print Width
ERR:            BLKB    1,0               ; Error Number
LISTON:         BLKB    1,0               ; LISTO (bottom nibble)
                                ; - BIT 0: If set, output a space after the line number
                                ; - BIT 1: If set, then indent FOR/NEXT loops
                                ; - BIT 2: If set, then indent REPEAT/UNTIL loops
                                ; - BIT 3: If set, then output to buffer for *EDIT
                                ; OPT FLAG (top nibble)
                                ; - BIT 4: If set, then list whilst assembling
                                ; - BIT 5: If set, then assembler errors are reported
                                ; - BIT 6: If set, then place the code starting at address pointed to by O%
                                ; - BIT 7: If set, then assemble in ADL mode, otherwise assemble in Z80 mode
INCREM:         BLKB    1,0               ; Auto-Increment Value
;
; --------------------------------------------------------------------------------------------
; BEGIN MODIFIED CODE
; --------------------------------------------------------------------------------------------
; Originally in equs.inc
;
OC:			EQU     15*4+STAVAR     ; CODE ORIGIN (O%)
PC:			EQU     16*4+STAVAR     ; PROGRAM COUNTER (P%)
VDU_BUFFER:		EQU	ACCS		; Storage for VDU commands
; --------------------------------------------------------------------------------------------
; END MODIFIED CODE
; --------------------------------------------------------------------------------------------

; Extra Agon-implementation specific system variables
;
FLAGS:          BLKB    1,0       ; Miscellaneous flags
                                ; - BIT 7: Set if ESC pressed
                                ; - BIT 6: Set to disable ESC
OSWRCHPT:       BLKB    2,0       ; Pointer for *EDIT
OSWRCHCH:       BLKB    1,0       ; Channel of OSWRCH
                                ; - 0: Console
                                ; - 1: File
OSWRCHFH:       BLKB    1,0       ; File handle for OSWRCHCHN
KEYDOWN:        BLKB    1,0       ; Keydown flag
KEYASCII:       BLKB    1,0       ; ASCII code of pressed key
KEYCOUNT:       BLKB    1,0       ; Counts every time a key is pressed
R0:             BLKB    3,0       ; General purpose storage for 8/16 to 24 bit operations
R1:             BLKB    3,0

;
; This must be at the end
;
RAM_END:
			ALIGN	256			
USER:							; Must be aligned on a page boundary
	; --- End ram.asm ---

