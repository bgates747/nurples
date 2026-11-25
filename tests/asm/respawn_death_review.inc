    include "test_death_key.inc"

; The same no-damage return used for the turret diagnostic. Collision
; callbacks still detect hits and dispose of actors by their normal rules.
update_shields:
    ld a,(player_shields)
    or a
    ret
