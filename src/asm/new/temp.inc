temp:
    ld hl,128 ; random bufferId
    ld bc,@temp_string_end-@temp_string
    ld de,@temp_string
    call vdu_load_buffer

    ld hl,@temp_string
    call printString
    jp exit
@temp_string: db "\r\nTemporary routine ran successfully!\r\n",0
@temp_string_end: