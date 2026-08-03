; Stable numeric state/event ABI shared by runtime telemetry and the replacement
; state machine. These values are externally observable; do not renumber them.

STATE_NEW_BOOT:                equ 0
STATE_NEW_LOADING:             equ 1
STATE_NEW_TITLE:               equ 2
STATE_NEW_NEW_GAME:            equ 3
STATE_NEW_LEVEL_ENTER:         equ 4
STATE_NEW_COUNTDOWN:           equ 5
STATE_NEW_PLAYING:             equ 6
STATE_NEW_PAUSED:              equ 7
STATE_NEW_LEVEL_TRANSITION:    equ 8
STATE_NEW_RESPAWN:             equ 9
STATE_NEW_GAME_OVER:           equ 10
STATE_NEW_VICTORY:             equ 11
STATE_NEW_EXIT:                equ 12
STATE_NEW_FAULT:               equ 13
STATE_NEW_COUNT:               equ 14
STATE_NEW_NONE:                equ 255

; Legacy-only phases remain distinguishable while the pointer dispatcher is
; being retired. They are never valid replacement-coordinator table indices.
STATE_LEGACY_CUTSCENE:         equ 128
STATE_LEGACY_INVENTORY:        equ 129
STATE_LEGACY_MAP:              equ 130

STATE_NEW_EVENT_START:         equ %00000001
STATE_NEW_EVENT_LOAD_COMPLETE: equ %00000010
STATE_NEW_EVENT_PAUSE:         equ %00000100
STATE_NEW_EVENT_LEVEL_COMPLETE:equ %00001000
STATE_NEW_EVENT_PLAYER_DEAD:   equ %00010000
STATE_NEW_EVENT_NO_LIVES:      equ %00100000
STATE_NEW_EVENT_VICTORY:       equ %01000000
STATE_NEW_EVENT_QUIT:          equ %10000000

STATE_NEW_POLICY_INPUT:        equ %00000001
STATE_NEW_POLICY_ACTOR_UPDATE: equ %00000010
STATE_NEW_POLICY_ACTOR_RENDER: equ %00000100
STATE_NEW_POLICY_WORLD_UPDATE: equ %00001000
STATE_NEW_POLICY_WORLD_RENDER: equ %00010000
STATE_NEW_POLICY_STATE_RENDER: equ %00100000

STATE_NEW_PLAYER_INACTIVE:     equ 0
STATE_NEW_PLAYER_READY:        equ 1
STATE_NEW_PLAYER_ACTIVE:       equ 2
STATE_NEW_PLAYER_DYING:        equ 3
STATE_NEW_PLAYER_DEAD:         equ 4
STATE_NEW_PLAYER_COUNT:        equ 5

STATE_TELEMETRY_ENEMY_INACTIVE:   equ 0
STATE_TELEMETRY_ENEMY_ACTIVE:     equ 1
STATE_TELEMETRY_ENEMY_DESPAWNING: equ 2
STATE_TELEMETRY_ENEMY_DEAD:       equ 3
STATE_TELEMETRY_ENEMY_RESPAWNING: equ 4
STATE_TELEMETRY_ENEMY_COUNT:      equ 5
