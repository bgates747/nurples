; player state variables
player_state: dl player_alive

player_do:
    ld hl,(player_state)
    callHL
    ret
player_dead:
    ret

player_alive:
    call player_input
    ret

