; scores are unsigned integers
player_score: dl 0
player_score_last: dl 0 ; if equal to player_score, no need to update
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
    ret
; end update_score

; text coordinates for score display
score_x: equ 50
score_y: equ 33
; inputs: none
print_score:
    ld hl,(player_score_last)
    ld de,(player_score)
    ld (player_score_last),de
    xor a ; clear carry
    sbc hl,de
    ret z ; no change to update
    ld c,score_x
    ld b,score_y
    call vdu_move_cursor
    ld hl,(player_score)
    call printDec
    ret
; end print_score