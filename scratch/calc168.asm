 ASSUME ADL = 1 
    ORG 0x0B0000 ; Is a moslet
    ; include "mos_api.inc"
    JP _start 

; Storage for the argv array pointers
min_args: equ 1
argv_ptrs_max: EQU 16 ; Maximum number of arguments allowed in argv
argv_ptrs: BLKP argv_ptrs_max, 0	

_exec_name:
    ASCIZ "calc168" ; The executable name, only used in argv
    ALIGN 64
    DB "MOS" ; Flag for MOS - to confirm this is a valid MOS command
    DB 00h ; MOS header version 0
    DB 01h ; Flag for run mode (0: Z80, 1: ADL) 

_start: 
    PUSH AF ; Preserve the registers
    PUSH BC
    PUSH DE
    PUSH IX
    PUSH IY
    LD A, MB ; Save MB
    PUSH AF
    XOR A
    LD MB, A ; Clear to zero so MOS API calls know how to use 24-bit addresses.
    LD IX, argv_ptrs ; The argv array pointer address
    PUSH IX
    CALL _parse_params ; Parse the parameters
    POP IX ; IX: argv 
    LD B, 0 ; C: argc

    CALL main ; Start user code
    
    POP AF
    LD MB, A
    POP IY ; Restore registers
    POP IX
    POP DE
    POP BC
    POP AF
    RET

_main_end_error:
    call printInline
    asciz "An error occurred!\r\n"
    ld hl,19 ; return error code 19
    ret

_main_end_ok:
    call printInline
    asciz "\r\n\r\n"
    ld hl,0 ; return 0 for success
    ret

;--- APPLICATION INCLUDES ---
    include "fixed168.inc"

; --- MAIN PROGRAM ---
main:
    dec c ; decrement the argument count to skip the program name

; compute distance between the origin and a point entered by the user
    call get_arg_s168
    ld (@arg1),de ; dx
    call get_arg_s168
    ld (@arg2),de ; dy

; echo the input
    call printInline
    asciz "distance to target at: "
    ld hl,(@arg1)
    call print_s168
    ld a,',' ; print a comma
    rst.lil 10h
    ld hl,(@arg2)
    call print_s168
    call printInline
    asciz " = "

; compute the distance
    ld ix,(@arg1) ; x1
    ld iy,(@arg2) ; y1
    ld bc,0 ; x1 0
    ld de,0 ; y1 0
    call distance168
    call print_s168

; compute angle between the origin and the point entered by the user
    call printInline
    asciz "\r\ntarget bearing: "
    ld bc,(dx168)
    ld de,(dy168)
    call atan2_168fast ; uh.l = atan2(dx,-dy) in deg256
    ; call deg_256_to_360 ; convert to 360 degree circle
    call print_s168

    jp _main_end_ok

@arg1: dl 0
@arg2: dl 0

; ========== HELPER FUNCTIONS ==========
; get the next argument after ix as a string
; inputs: ix = pointer to the argument string
; outputs: HL = pointer to the argument string, ix points to the next argument
; destroys: a, h, l, f
get_arg_text:
    lea ix,ix+3 ; point to the next argument
    ld hl,(ix) ; get the argument string
    ret

; get the next argument after ix as a signed 16.8 fixed point number
; inputs: ix = pointer to the argument string
; outputs: ude = signed 16.8 fixed point number
; destroys: a, d, e, h, l, f
get_arg_s168:
    lea ix,ix+3 ; point to the next argument
    ld hl,(ix) ; get the argument string
    call asc_to_s168 ; convert the string to a number
    ret ; return with the value in DE

; Inputs: ix = pointer to the argument string
; Outputs: ude = signed 24-bit integer
; Destroys: a, d, e, h, l, f
get_arg_s24:
    lea ix,ix+3 ; point to the next argument
    ld hl,(ix) ; get the argument string
    call asc_to_s24 ; convert the string to a number
    ret ; return with the value in DE

; match the next argument after ix to the dispatch table at iy
; - arguments and dispatch entries are zero-terminated, case-sensitive strings
; - final entry of dispatch table must be a 3-byte zero or bad things will happen
; returns: NO MATCH: iy=dispatch list terminator a=1 and zero flag reset
; ON MATCH: iy=dispatch address, a=0 and zero flag set
; destroys: a, hl, de, ix, iy, flags
match_next:
    lea ix,ix+3 ; point to the next argument
@loop:
    ld hl,(iy) ; pointer argument dispatch record
    sign_hlu ; check for list terminator
    jp z,@no_match ; if a=0, return error
    inc hl ; skip over jp instruction
    inc hl
    ld de,(ix) ; pointer to the argument string
    call str_equal ; compare the argument to the dispatch table entry
    jp z,@match ; if equal, return success
    lea iy,iy+3 ; if not equal, bump iy to next dispatch table entry
    jp @loop ; and loop 
@no_match:
    inc a ; no match so return a=1 and zero flag reset
    ret
@match:
    ld iy,(iy) ; get the function pointer
    ret ; return a=0 and zero flag set

; compare two zero-terminated strings for equality, case-sensitive
; hl: pointer to first string, de: pointer to second string
; returns: z if equal, nz if not equal
; destroys: a, hl, de
str_equal:
    ld a,(de) ; get the first character
    cp (hl) ; compare to the second character
    ret nz ; if not equal, return
    or a
    ret z ; if equal and zero, return
    inc hl ; next character
    inc de
    jp str_equal ; loop until end of string

; === BOILERPLATE MOSLET CODE ===
; Parse the parameter string into a C array
; Parameters
; - HL: Address of parameter string
; - IX: Address for array pointer storage
; Returns:
; - C: Number of parameters parsed
;
_parse_params: LD BC, _exec_name
    LD (IX+0), BC ; ARGV[0] = the executable name
    LEA IX, IX+3
    CALL _skip_spaces ; Skip HL past any leading spaces
;
    LD BC, 1 ; C: ARGC = 1 - also clears out top 16 bits of BCU
    LD B, argv_ptrs_max - 1 ; B: Maximum number of argv_ptrs
;
_parse_params_1: 
    PUSH BC ; Stack ARGC 
    PUSH HL ; Stack start address of token
    CALL _get_token ; Get the next token
    LD A, C ; A: Length of the token in characters
    POP DE ; Start address of token (was in HL)
    POP BC ; ARGC
    OR A ; Check for A=0 (no token found) OR at end of string
    RET Z
;
    LD (IX+0), DE ; Store the pointer to the token
    PUSH HL ; DE=HL
    POP DE
    CALL _skip_spaces ; And skip HL past any spaces onto the next character
    XOR A
    LD (DE), A ; Zero-terminate the token
    LEA IX, IX+3 ; Advance to next pointer position
    INC C ; Increment ARGC
    LD A, C ; Check for C >= A
    CP B
    JR C, _parse_params_1 ; And loop
    RET

; Get the next token
; Parameters:
; - HL: Address of parameter string
; Returns:
; - HL: Address of first character after token
; - C: Length of token (in characters)
;
_get_token: LD C, 0 ; Initialise length
@@: LD A, (HL) ; Get the character from the parameter string
    OR A ; Exit if 0 (end of parameter string in MOS)
    RET Z
    CP 13 ; Exit if CR (end of parameter string in BBC BASIC)
    RET Z
    CP ' ' ; Exit if space (end of token)
    RET Z
    INC HL ; Advance to next character
    INC C ; Increment length
    JR @B
    
; Skip spaces in the parameter string
; Parameters:
; - HL: Address of parameter string
; Returns:
; - HL: Address of next none-space character
; F: Z if at end of string, otherwise NZ if there are more tokens to be parsed
;
_skip_spaces: LD A, (HL) ; Get the character from the parameter string 
    CP ' ' ; Exit if not space
    RET NZ
    INC HL ; Advance to next character
    JR _skip_spaces ; Increment length