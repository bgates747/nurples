; scores are unsigned integers
player_score: dl 0
player_starting_score: dl 0 ; to reset after restarting a map

; inputs: iy set to sprite/tile record being tallied
; outputs: hl,player_score = updated score
; destroys: hl,de
update_score:
    ld de,0 ; clear deu and d
    ld e,(iy+sprite_points)
    ld hl,(player_score)
    add hl,de
    ld (player_score),hl
    call print_score
    ret
; end update_score

; text coordinates for score display
score_x: equ 50
score_y: equ 33
; inputs: none
print_score:
; set text colors
    ld a,c_black+128
    call vdu_colour_text
    ld a,c_green
    call vdu_colour_text
; print the score
    ld c,score_x
    ld b,score_y
    call vdu_move_cursor
    ld hl,(player_score)
    call printDec
    ret
; end print_score