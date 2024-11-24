    macro multiply width,height
    ld b, width
    ld c, height
    mlt bc
    ld a,b
    ld (@size), a
    ld a,c
    ld (@size+1), a
@size: dw 0x0000
    endmacro

main:
    multiply 3,4
    jp main