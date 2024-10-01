    .assume adl=1                       ; ez80 ADL memory mode
    .org $40000                         ; load code here

    jp start_here                       ; jump to start of code

    .align 64                           ; MOS header
    .db "MOS",0,1     

SCREENMODE_320x240_64:   equ 8

start_here:
            
    push af                             ; store all the registers
    push bc
    push de
    push ix
    push iy

; ------------------
; This is our actual code

; prepare the screen

; Set the screen mode
	ld a, 22
	rst.lil $10
	ld a, SCREENMODE_320x240_64
	rst.lil $10

; Sending a VDU byte stream

    ld hl, VDUdata                      ; address of string to use
    ld bc, endVDUdata - VDUdata         ; length of string
    rst.lil $18                         ; Call the MOS API to send data to VDP 

    ld a, $08                           ; code to send to MOS
    rst.lil $08                         ; get IX pointer to System Variables

WAIT_HERE:                              ; loop here until we hit ESC key

    ld a, $1E
	rst.lil $08

    ld a, (ix + $0E)    
    bit 0, a                            ; index $0E, bit 0 is for ESC key in matrix
    jp nz, EXIT_HERE                    ; if pressed, ESC key to exit
                
    ld a, (ix + $08)    
    bit 1, a                            ; index $06 bit 0 is for 'A' key in matrix
    call nz, moveLeft                  ; if pressed, setFrame1

    ld a, (ix + $06)    
    bit 2, a                            ; index $06 bit 0 is for 'D' key in matrix
    call nz, moveRight                  ; if pressed, setFrame1

    ld a, (ix + $04)    
    bit 1, a                            ; index $06 bit 0 is for 'A' key in matrix
    call nz, moveUp                  ; if pressed, setFrame1

    ld a, (ix + $0A)    
    bit 1, a                            ; index $06 bit 0 is for 'D' key in matrix
    call nz, moveDown                  ; if pressed, setFrame1


    ld a, (ix + $0A)    
    bit 4, a                            ; index $0A bit 4 is for 'h' key in matrix
    call nz, hideSprite                 ; if pressed, setFrame2

    ld a, (ix + $0A)    
    bit 1, a                            ; index $0A bit 1 is for 's' key in matrix
    call nz, showSprite                 ; if pressed, setFrame2

    jp WAIT_HERE

; ------------------

moveLeft:
    ld hl, left                         ; address of string to use
    ld bc, endleft - left               ; length of string
    rst.lil $18                         ; Call the MOS API to send data to VDP 
    ret 

left:
    .db 23, 27, 4, 0                    ; select sprite 0 
    .db 23, 27, 13                      ; move currrent sprite to...
    .dw 150, 100                        ; x,y (as words)
endleft:

moveRight:
    ld a, (sprite_x)
    inc a
    ld (sprite_x), a
    ld hl, right                         ; address of string to use
    ld bc, endright - right               ; length of string
    rst.lil $18                         ; Call the MOS API to send data to VDP 
    ret 

right:
    .db 23, 27, 4, 0                    ; select sprite 0 
    .db 23, 27, 13                      ; move currrent sprite to...
sprite_x:    .dw 0
sprite_y:    .dw 0 
endright:

moveUp:
    ld hl, up                         ; address of string to use
    ld bc, endup - up               ; length of string
    rst.lil $18                         ; Call the MOS API to send data to VDP 
    ret 

up:
    .db 23, 27, 4, 0                    ; select sprite 0 
    .db 23, 27, 13                      ; move currrent sprite to...
sprite_x:    .dw 0
sprite_y:    .dw 0 
endup:

moveDown:
    ld hl, down                         ; address of string to use
    ld bc, enddown - down               ; length of string
    rst.lil $18                         ; Call the MOS API to send data to VDP 
    ret 

down:
    .db 23, 27, 4, 0                    ; select sprite 0 
    .db 23, 27, 13                      ; move currrent sprite to...
    .dw 150, 116                        ; x,y (as words)
enddown:


; ------------------

; ------------------

hideSprite:
    ld hl, hide                         ; address of string to use
    ld bc, endhide - hide               ; length of string
    rst.lil $18                         ; Call the MOS API to send data to VDP
    ret 

hide:
    .db 23, 27, 4, 0                    ; select sprite 0
    .db 23, 27, 12                      ; hide current sprite
endhide:

; ------------------

showSprite:
    ld hl, show                         ; address of string to use
    ld bc, endshow - show               ; length of string
    rst.lil $18                         ; Call the MOS API to send data to VDP
    ret 

show:
    .db 23, 27, 4, 0                    ; select sprite 0
    .db 23, 27, 11                      ; show current sprite
endshow:


; ------------------
; This is where we exit the program

EXIT_HERE:

    ld a, 12
	rst.lil $10

    pop iy                              ; Pop all registers back from the stack
    pop ix
    pop de
    pop bc
    pop af
    ld hl,0                             ; Load the MOS API return code (0) for no errors.
    ret                                 ; Return to MOS


; ------------------
; This is the data we send to VDP

zombie:    EQU     0                   ; used for bitmap ID number
sprite: EQU     0                   ; sprite ID - always start at 0 upwards

VDUdata:
    .db 23, 0, 192, 0                   ; set to non-scaled graphics

    ; LOAD THE BITMAP FROM A FILE
    ; file must be 24bit colour plus 8 bit alpha, byte order: RGBA
    ; 16x16 pixels RGBA should be 1,024 bytes in size

    .db 23, 27, 0, zombie              ; select bitmap 0 - crystal
    .db 23, 27, 1                       ; load bitmap data...
    .dw 16, 16                          ; of size 16x16, from file:
    incbin     "src/zombie.data"

    ; SETUP THE SPRITE

    .db 23, 27, 4, sprite           ; select sprite 0
    .db 23, 27, 5                       ; clear frames
    .db 23, 27, 6, zombie              ; add bitmap frame crystal to sprite
    .db 23, 27, 7, 1                    ; activate 1 sprite(s)
    .db 23, 27, 11                      ; show current sprite

    ; MOVE A SPRITE

    .db 23, 27, 4, sprite               ; select sprite 0 
    .db 23, 27, 13                      ; move currrent sprite to...
    .dw 150, 100                        ; x,y (as words)

    .db 23, 27, 15                      ; update sprites in GPU

endVDUdata:

; ------------------