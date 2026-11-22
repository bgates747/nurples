; Generated from maps/active_tiles.toml; do not edit.
tile_pad_small: equ 1
tile_pad_med: equ 2
tile_pad_large: equ 3
tile_charge_shields: equ 4
tile_turret_fireball: equ 5
tile_laser_turret: equ 6
tile_twin_laser_turret: equ 7
tile_charge_fuel: equ 8
tile_electrode: equ 9
tile_lightning: equ 10
tile_electrode_r: equ 11
tile_12: equ 12
tile_13: equ 13
tile_14: equ 14
tile_15: equ 15
tile_crater: equ 16

tile_activation_routines:
    dl activate_null_tile ; 0: blank
    dl activate_pad_small ; 1: pad_small
    dl activate_null_tile ; 2: pad_med
    dl activate_null_tile ; 3: pad_large
    dl activate_null_tile ; 4: charge_shields
    dl activate_turret_fireball ; 5: turret_fireball
    dl activate_null_tile ; 6: laser_turret
    dl activate_null_tile ; 7: twin_laser_turret
    dl activate_null_tile ; 8: charge_fuel
    dl activate_electrode ; 9: electrode
    dl activate_lightning ; 10: lightning
    dl activate_electrode ; 11: electrode_r
    dl activate_null_tile ; 12: 12
    dl activate_null_tile ; 13: 13
    dl activate_null_tile ; 14: 14
    dl activate_null_tile ; 15: 15
    dl activate_crater ; 16: crater
tile_activation_routines_end:
tile_activation_routines_count: equ [tile_activation_routines_end-tile_activation_routines]/3
