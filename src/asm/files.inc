test_files:
; ; Passes hardware and emulator.
; ; Copy the current directory (string) into buffer (hl)
; ; HLU: Pointer to a buffer
; ; BCU: Maximum length of buffer
; ; Returns:
; ; A: FRESULT
;     ld hl,@dir
;     ld bc,255
;     MOSCALL ffs_getcwd

; ; Passes hardware FAILS emulator.
; ; Open a directory
; ; HLU: Pointer to a blank DIR struct
; ; DEU: Pointer to the directory path
; ; Returns:
; ; A: FRESULT
;     ld hl,@dir_obj
;     ld de,@dir
;     MOSCALL ffs_dopen

;     call @print_dir_struct

; Check file exists
; HLU: Pointer to a FILINFO struct
; DEU: Pointer to the filename (0 terminated)
; Returns:
;   A: FRESULT
    ld hl,@filinfo
    ld de,@filename
    MOSCALL ffs_stat

    ; call @print_filinfo_struct

; Open a file
; HLU: Filename
;   C: Mode
; Returns:
;   A: Filehandle, or 0 if couldn't open
    call printNewLine
    ld hl,@filename
    ld c,fa_read
    MOSCALL mos_fopen
    ld (@filehandle),a
    call printHex8

; Convert a file handle to a FIL structure pointer
;   C: Filehandle
; Returns:
; HLU: Pointer to a FIL struct
    ld a,(@filehandle)
    ld c,a
    MOSCALL mos_getfil
    ld (@fil),hl

    ; call @print_fil_struct

@read_file:
    ; Read a block of data from a file
    ;   C: Filehandle
    ; HLU: Pointer to where to write the data to
    ; DEU: Number of bytes to read
    ; Returns:
    ; DEU: Number of bytes read
        ld a,(@filehandle)
        ld c,a
        ld hl,filedata
        ld de,16384
        MOSCALL mos_fread
        call printNewLine
        printDecDE

        call @print_fil_ptr
        call printNewLine

        ld hl,0
        xor a ; clear carry
        sbc hl,de
        ret z
        jp @read_file

    ret

    align 256

; @filename: db "/mystuff/agonutils/examples/asm/tgt/images/nurples_space_01.rgba2",0
@filename: db "images/nurples_space_01.rgba2",0

    align 256

; Directory object structure (DIR)
@dir:            blkb 256,0    ; Zero-terminated string of the current directory path (256 bytes)
@dir_obj:        blkb 15, FFh  ; Object identifier (15 bytes)
@dir_dptr:       blkb 4, FFh   ; Current read/write offset (4 bytes)
@dir_clust:      blkb 4, FFh   ; Current cluster (4 bytes)
@dir_sect:       blkb 4, FFh   ; Current sector (4 bytes)
@dir_dir:        blkb 3, FFh   ; Pointer to the directory item in win[] (3 bytes)
@dir_fn:         blkb 12, FFh  ; SFN (body[8], ext[3], status[1]) (12 bytes)
@dir_blk_ofs:    blkb 4, FFh   ; Offset of current entry block being processed (4 bytes)

@dir_text:         db "dir:         ",0
@dir_obj_text:     db "dir_obj:     ",0
@dir_dptr_text:    db "dir_dptr:    ",0
@dir_clust_text:   db "dir_clust:   ",0
@dir_sect_text:    db "dir_sect:    ",0
@dir_dir_text:     db "dir_dir:     ",0
@dir_fn_text:      db "dir_fn:      ",0
@dir_blk_ofs_text: db "dir_blk_ofs: ",0


@print_dir_struct:
    call printNewLine
    ld hl,@dir_text
    call printString
    ld hl,@dir
    call printString
    call printNewLine

    ld hl,@dir_obj_text
    call printString
    ld hl,@dir_obj
    ld a,15
    call dumpMemoryHex
    call printNewLine

    ld hl,@dir_dptr_text
    call printString
    ld hl,@dir_dptr
    ld a,4
    call dumpMemoryHex
    call printNewLine

    ld hl,@dir_clust_text
    call printString
    ld hl,@dir_clust
    ld a,4
    call dumpMemoryHex
    call printNewLine

    ld hl,@dir_sect_text
    call printString
    ld hl,@dir_sect
    ld a,4
    call dumpMemoryHex
    call printNewLine

    ld hl,@dir_dir_text
    call printString
    ld hl,@dir_dir
    ld a,3
    call dumpMemoryHex
    call printNewLine

    ld hl,@dir_fn_text
    call printString
    ld hl,@dir_fn
    ld a,12
    call dumpMemoryHex
    call printNewLine

    ld hl,@dir_blk_ofs_text
    call printString
    ld hl,@dir_blk_ofs
    ld a,4
    call dumpMemoryHex
    call printNewLine

    ret

@filehandle:     db 0x00         ; handle of active file
@fil:            dl 0x000000     ; pointer to active FIL struct
; File object structure (FIL) local copy if needed
@fil_obj:        blkb 15, 00h    ; Object identifier (15 bytes)
@fil_flag:       blkb  1, 00h    ; File status flags (1 byte)
@fil_err:        blkb  1, 00h    ; Abort flag (error code) (1 byte)
@fil_fptr:       blkb  4, 00h    ; File read/write pointer (4 bytes)
@fil_clust:      blkb  4, 00h    ; Current cluster of fpter (4 bytes)
@fil_sect:       blkb  4, 00h    ; Sector number appearing in buf[] (4 bytes)
@fil_dir_sect:   blkb  4, 00h    ; Sector number containing the directory entry (4 bytes)
@fil_dir_ptr:    blkb  3, 00h    ; Pointer to the directory entry in the win[] (3 bytes)

@fil_obj_text:      db "fil_obj:      ",0
@fil_flag_text:     db "fil_flag:     ",0
@fil_err_text:      db "fil_err:      ",0
@fil_fptr_text:     db "fil_fptr:     ",0
@fil_clust_text:    db "fil_clust:    ",0
@fil_sect_text:     db "fil_sect:     ",0
@fil_dir_sect_text: db "fil_dir_sect: ",0
@fil_dir_ptr_text:  db "fil_dir_ptr:  ",0

@print_fil_ptr:
    ld ix,(@fil)
    ld hl,@fil_fptr_text
    call printString
    ld hl,(ix+fil_fptr)
    printDecHL
    ret

@print_fil_struct:
    ld ix,(@fil)

    call printNewLine
    ld hl,@fil_obj_text
    call printString
    ld hl,(ix+fil_obj)
    ld a,15
    call dumpMemoryHex
    call printNewLine

    ld hl,@fil_flag_text
    call printString
    ld hl,(ix+fil_flag)
    ld a,1
    call dumpMemoryHex
    call printNewLine

    ld hl,@fil_err_text
    call printString
    ld hl,(ix+fil_err)
    ld a,1
    call dumpMemoryHex
    call printNewLine

    ld hl,@fil_fptr_text
    call printString
    ld hl,(ix+fil_fptr)
    ld a,4
    call dumpMemoryHex
    call printNewLine

    ld hl,@fil_clust_text
    call printString
    ld hl,(ix+fil_clust)
    ld a,4
    call dumpMemoryHex
    call printNewLine

    ld hl,@fil_sect_text
    call printString
    ld hl,(ix+fil_sect)
    ld a,4
    call dumpMemoryHex
    call printNewLine
    
    ld hl,@fil_dir_sect_text
    call printString
    ld hl,(ix+fil_dir_sect)
    ld a,4
    call dumpMemoryHex
    call printNewLine

    ld hl,@fil_dir_ptr_text
    call printString
    ld hl,(ix+fil_dir_ptr)
    ld a,3
    call dumpMemoryHex
    call printNewLine

    ret

; File information structure (FILINFO)
@filinfo:
@filinfo_fsize:    blkb   4, FFh ; File size (4 bytes)
@filinfo_fdate:    blkb   2, FFh ; Modified date (2 bytes)
@filinfo_ftime:    blkb   2, FFh ; Modified time (2 bytes)
@filinfo_fattrib:  blkb   1, FFh ; File attribute (1 byte)
@filinfo_altname:  blkb  13, FFh ; Alternative file name (13 bytes)
@filinfo_fname:    blkb 256, FFh ; Primary file name (256 bytes)

@filinfo_fsize_text:    db "filinfo_fsize:   ",0
@filinfo_fdate_text:    db "filinfo_fdate:   ",0
@filinfo_ftime_text:    db "filinfo_ftime:   ",0
@filinfo_fattrib_text:  db "filinfo_fattrib: ",0
@filinfo_altname_text:  db "filinfo_altname: ",0
@filinfo_fname_text:    db "filinfo_fname:   ",0

@print_filinfo_struct:
    ld ix,@filinfo

    call printNewLine
    ld hl,@filinfo_fsize_text
    call printString
    ld hl,(ix+filinfo_fsize)
    printDecHL
    call printNewLine

    ld hl,@filinfo_fdate_text
    call printString
    ld hl,(ix+filinfo_fdate)
    ld a,2
    call dumpMemoryHex
    call printNewLine

    ld hl,@filinfo_ftime_text
    call printString
    ld hl,(ix+filinfo_ftime)
    ld a,2
    call dumpMemoryHex
    call printNewLine

    ld hl,@filinfo_fattrib_text
    call printString
    ld hl,(ix+filinfo_fattrib)
    ld a,1
    call dumpMemoryHex
    call printNewLine

    ld hl,@filinfo_altname_text
    call printString
    ld hl,(ix+filinfo_altname)
    ld a,13
    call dumpMemoryHex
    call printNewLine

    ld hl,@filinfo_fname_text
    call printString
    ld hl,(ix+filinfo_fname)
    ; call printString
    ld a,0 ; 256
    call dumpMemoryHex
    call printNewLine

    ret