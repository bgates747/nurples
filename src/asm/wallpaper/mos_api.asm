;
; Title:	AGON MOS - API code
; Author:	Dean Belfield
; Created:	24/07/2022
; Last Updated:	10/11/2023
;
; Modinfo:
; 03/08/2022:	Added a handful of MOS API calls and stubbed FatFS calls
; 05/08/2022:	Added mos_FEOF, saved affected registers in fopen, fclose, fgetc, fputc and feof
; 09/08/2022:	mos_api_sysvars now returns pointer to _sysvars
; 05/09/2022:	Added mos_REN
; 24/09/2022:	Error codes returned for MOS commands
; 13/10/2022:	Added mos_OSCLI and supporting code
; 20/10/2022:	Tweaked error handling
; 13/03/2023:	Renamed keycode to keyascii, fixed mos_api_getkey, added parameter to mos_api_dir
; 15/03/2023:	Added mos_api_copy, mos_api_getrtc, mos_api_setrtc
; 21/03/2023:	Added mos_api_setintvector
; 24/03/2023:	Fixed bugs in mos_api_setintvector
; 28/03/2023:	Function mos_api_setintvector now only accepts a 24-bit pointer
; 29/03/2023:	Added mos_api_uopen, mos_api_uclose, mos_api_ugetc, mos_api_uputc
; 14/04/2023:	Added ffs_api_fopen, ffs_api_fclose, ffs_api_stat, ffs_api_fread, ffs_api_fwrite, ffs_api_feof, ffs_api_flseek
; 15/04/2023:	Added mos_api_getfil, mos_api_fread, mos_api_fwrite and mos_api_flseek
; 30/05/2023:	Fixed mos_api_fgetc to set carry if at end of file
; 03/08/2023:	Added mos_api_setkbvector
; 10/08/2023:	Added mos_api_getkbmap
; 10/11/2023:	Added mos_api_i2c_close, mos_api_i2c_open, mos_api_i2c_read, mos_api_i2c_write


; Get keycode
; Returns:
;  A: ASCII code of key pressed, or 0 if no key pressed
;
mos_api_getkey:
    MOSCALL mos_getkey
    RET

; Load an area of memory from a file.
; HLU: Address of filename (zero terminated)
; DEU: Address at which to load
; BCU: Maximum allowed size (bytes)
; Returns:
; - A: File error, or 0 if OK
; - F: Carry reset indicates no room for file.
;
mos_api_load:
    MOSCALL mos_load
    RET

; Save a file to the SD card from RAM
; HLU: Address of filename (zero terminated)
; DEU: Address to save from
; BCU: Number of bytes to save
; Returns:
; - A: File error, or 0 if OK
; - F: Carry reset indicates no room for file
;
mos_api_save:
    MOSCALL mos_save
    RET

; Change directory
; HLU: Address of path (zero terminated)
; Returns:
; - A: File error, or 0 if OK
;			
mos_api_cd:
    MOSCALL mos_cd
    RET

; Directory listing
; HLU: Address of path (zero terminated)
; Returns:
; - A: File error, or 0 if OK
;	
mos_api_dir:
    MOSCALL mos_dir
    RET
			
; Delete a file from the SD card
; HLU: Address of filename (zero terminated)
; Returns:
; - A: File error, or 0 if OK
;
mos_api_del:
    MOSCALL mos_del
    RET

; Rename a file on the SD card
; HLU: Address of filename1 (zero terminated)
; DEU: Address of filename2 (zero terminated)
; Returns:
; - A: File error, or 0 if OK
;
mos_api_ren:
    MOSCALL mos_ren
    RET

; Copy a file on the SD card
; HLU: Address of filename1 (zero terminated)
; DEU: Address of filename2 (zero terminated)
; Returns:
; - A: File error, or 0 if OK
;
mos_api_copy:
    MOSCALL mos_copy
    RET

; Make a folder on the SD card
; HLU: Address of filename (zero terminated)
; Returns:
; - A: File error, or 0 if OK
;
mos_api_mkdir:
    MOSCALL mos_mkdir
    RET

; Get a pointer to a system variable
; Returns:
; IXU: Pointer to system variables (see mos_api.asm for more details)
;
mos_api_sysvars:
    MOSCALL mos_sysvars
    RET
			
; Invoke the line editor
; HLU: Address of the buffer
; BCU: Buffer length
;   E: 0 to not clear buffer, 1 to clear
; Returns:
;   A: Key that was used to exit the input loop (CR=13, ESC=27)
;
mos_api_editline:
    MOSCALL mos_editline
    RET

; Open a file
; HLU: Filename
;   C: Mode
; Returns:
;   A: Filehandle, or 0 if couldn't open
;
mos_api_fopen:
    MOSCALL mos_fopen
    RET

; Close a file
;   C: Filehandle
; Returns
;   A: Number of files still open
;
mos_api_fclose:
    MOSCALL mos_fclose
    RET

; Get a character from a file
;   C: Filehandle
; Returns:
;   A: Character read
;   F: C set if last character in file, otherwise NC
;
mos_api_fgetc:
    MOSCALL mos_fgetc
    RET

; Write a character to a file
;   C: Filehandle
;   B: Character to write
;
mos_api_fputc:
    MOSCALL mos_fputc
    RET

; Check whether we're at the end of the file
;   C: Filehandle
; Returns:
;   A: 1 if at end of file, otherwise 0
;     
mos_api_feof:
    MOSCALL mos_feof
    RET

; Copy an error message
;   E: The error code
; HLU: Address of buffer to copy message into
; BCU: Size of buffer
;
mos_api_getError:
    MOSCALL mos_getError
    RET

; Execute a MOS command
; HLU: Pointer the the MOS command string
; DEU: Pointer to additional command structure
; BCU: Number of additional commands
; Returns:
;   A: MOS error code
;
mos_api_oscli:
    MOSCALL mos_oscli
    RET

; Fetch a RTC string
; HLU: Pointer to a buffer to copy the string to
; Returns:
;   A: Length of time
;
mos_api_getrtc:
    MOSCALL mos_getrtc
    RET

; Set the RTC
; HLU: Pointer to a buffer with the time data in
;
mos_api_setrtc:
    MOSCALL mos_setrtc
    RET

; Set an interrupt vector
; HLU: Pointer to the interrupt vector (24-bit pointer)
;   E: Vector # to set
; Returns:
; HLU: Pointer to the previous vector
;
mos_api_setintvector:
    MOSCALL mos_setintvector
    RET

; Set a VDP keyboard packet receiver callback
;   C: If non-zero then set the top byte of HLU(callback address)  to MB (for ADL=0 callers)
; HLU: Pointer to callback
;
mos_api_setkbvector:
    MOSCALL mos_setkbvector
    RET

; Get the address of the keyboard map
; Returns:
; IXU: Base address of the keymap
; 
mos_api_getkbmap:
    MOSCALL mos_getkbmap
    RET

; ; == BEGIN NOT IMPLEMENTED ==
; ; Open the I2C bus as master
; ;   C: Frequency ID
; ;
; mos_api_i2c_open:
;     MOSCALL mos_i2c_open
;     RET

; ; Close the I2C bus
; ;
; mos_api_i2c_close:
;     MOSCALL mos_i2c_close
;     RET

; ; Write n bytes to the I2C bus
; ;   C: I2C address
; ;   B: Number of bytes to write, maximum 32
; ; HLU: Address of buffer containing the bytes to send
; ; 
; mos_api_i2c_write:
;     MOSCALL mos_i2c_write
;     RET

; ; Read n bytes from the I2C bus
; ;   C: I2C address
; ;   B: Number of bytes to read, maximum 32
; ; HLU: Address of buffer to read bytes to
; ;
; mos_api_i2c_read:
;     MOSCALL mos_i2c_read
;     RET
; ; == END NOT IMPLEMENTED ==

; Open UART1
; IXU: Pointer to UART struct
;	+0: Baud rate (24-bit, little endian)
;	+3: Data bits
;	+4: Stop bits
;	+5: Parity bits
;	+6: Flow control (0: None, 1: Hardware)
;	+7: Enabled interrupts
; Returns:
;   A: Error code (0 = no error)
;
mos_api_uopen:
    MOSCALL mos_uopen
    RET

; Close UART1
;
mos_api_uclose:
    MOSCALL mos_uclose
    RET

; Get a character from UART1
; Returns:
;   A: Character read
;   F: C if successful
;   F: NC if the UART is not open
;
mos_api_ugetc:
    MOSCALL mos_ugetc
    RET

; Write a character to UART1
;   C: Character to write
; Returns:
;   F: C if successful
;   F: NC if the UART is not open
;
mos_api_uputc:
    MOSCALL mos_uputc
    RET

; Convert a file handle to a FIL structure pointer
;   C: Filehandle
; Returns:
; HLU: Pointer to a FIL struct
;
mos_api_getfil:
    MOSCALL mos_getfil
    RET

; Read a block of data from a file
;   C: Filehandle
; HLU: Pointer to where to write the data to
; DEU: Number of bytes to read
; Returns:
; DEU: Number of bytes read
;
mos_api_fread:
    MOSCALL mos_fread
    RET

; Write a block of data to a file
;  C: Filehandle
; HLU: Pointer to where the data is
; DEU: Number of bytes to write
; Returns:
; DEU: Number of bytes read
;
mos_api_fwrite:
    MOSCALL mos_fwrite
    RET

; Move the read/write pointer in a file
;   C: Filehandle
; HLU: Least significant 3 bytes of the offset from the start of the file (DWORD)
;   E: Most significant byte of the offset
; Returns:
;   A: FRESULT
;
mos_api_flseek:
    MOSCALL mos_flseek
    RET

; Open a file
; HLU: Pointer to a blank FIL struct
; DEU: Pointer to the filename (0 terminated)
;   C: File mode
; Returns:
;   A: FRESULT
;
ffs_api_fopen:
    MOSCALL ffs_fopen
    RET

; Close a file
; HLU: Pointer to a blank FIL struct
; Returns:
;   A: FRESULT
;
ffs_api_fclose:
    MOSCALL ffs_fclose
    RET

; Read data from a file
; HLU: Pointer to a FIL struct
; DEU: Pointer to where to write the file out
; BCU: Number of bytes to read
; Returns:
;   A: FRESULT
; BCU: Number of bytes read
;
ffs_api_fread:
    MOSCALL ffs_fread
    RET

; Write data to a file
; HLU: Pointer to a FIL struct
; DEU: Pointer to the data to write out
; BCU: Number of bytes to write
; Returns:
;   A: FRESULT
; BCU: Number of bytes written
;
ffs_api_fwrite:
    MOSCALL ffs_fwrite
    RET

; Check file exists
; HLU: Pointer to a FILINFO struct
; DEU: Pointer to the filename (0 terminated)
; Returns:
;   A: FRESULT
;
ffs_api_stat:
    MOSCALL ffs_stat
    RET

; Check for EOF
; HLU: Pointer to a FILINFO struct
; Returns:
;   A: 1 if end of file, otherwise 0
;
ffs_api_feof:
    MOSCALL ffs_feof
    RET

; Move the read/write pointer in a file
; HLU: Pointer to a FIL struct
; DEU: Least significant 3 bytes of the offset from the start of the file (DWORD)
;   C: Most significant byte of the offset
; Returns:
;   A: FRESULT
;
ffs_api_flseek:
    MOSCALL ffs_flseek
    RET

; ;		
; ; Commands that have not been implemented yet
; ;
; ffs_api_ftruncate:	
; 			JP mos_api_not_implemented
; ffs_api_fsync:		
; 			JP mos_api_not_implemented
; ffs_api_fforward:	
; 			JP mos_api_not_implemented
; ffs_api_fexpand:	
; 			JP mos_api_not_implemented
; ffs_api_fgets:		
; 			JP mos_api_not_implemented
; ffs_api_fputc:		
; 			JP mos_api_not_implemented
; ffs_api_fputs:		
; 			JP mos_api_not_implemented
; ffs_api_fprintf:	
; 			JP mos_api_not_implemented
; ffs_api_ftell:		
; 			JP mos_api_not_implemented
; ffs_api_fsize:		
; 			JP mos_api_not_implemented
; ffs_api_ferror:		
; 			JP mos_api_not_implemented

; Open a directory
; HLU: Pointer to a blank DIR struct
; DEU: Pointer to the directory path
; Returns:
; A: FRESULT
ffs_api_dopen:
    MOSCALL ffs_dopen
    RET

; Close a directory
; HLU: Pointer to an open DIR struct
; Returns:
; A: FRESULT
ffs_api_dclose:
    MOSCALL ffs_dclose
    RET

; Read the next FILINFO from an open DIR
; HLU: Pointer to an open DIR struct
; DEU: Pointer to an empty FILINFO struct
; Returns:
; A: FRESULT
ffs_api_dread:
    MOSCALL ffs_dread
    RET

; ffs_api_dfindfirst:	
; 			JP mos_api_not_implemented
; ffs_api_dfindnext:	
; 			JP mos_api_not_implemented
; ffs_api_unlink:		
; 			JP mos_api_not_implemented
; ffs_api_rename:		
; 			JP mos_api_not_implemented
; ffs_api_chmod:		
; 			JP mos_api_not_implemented
; ffs_api_utime:		
; 			JP mos_api_not_implemented
; ffs_api_mkdir:		
; 			JP mos_api_not_implemented
; ffs_api_chdir:		
; 			JP mos_api_not_implemented
; ffs_api_chdrive:	
; 			JP mos_api_not_implemented

; Copy the current directory (string) into buffer (hl)
; HLU: Pointer to a buffer
; BCU: Maximum length of buffer
; Returns:
; A: FRESULT
ffs_api_getcwd:
    MOSCALL ffs_getcwd
    RET

; ffs_api_mount:		
; 			JP mos_api_not_implemented
; ffs_api_mkfs:		
; 			JP mos_api_not_implemented
; ffs_api_fdisk		
; 			JP mos_api_not_implemented
; ffs_api_getfree:	
; 			JP mos_api_not_implemented
; ffs_api_getlabel:	
; 			JP mos_api_not_implemented
; ffs_api_setlabel:	
; 			JP mos_api_not_implemented
; ffs_api_setcp:		
; 			JP mos_api_not_implemented
