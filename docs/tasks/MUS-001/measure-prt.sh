#!/bin/bash
# Run inside a private Xvfb display on Oryx, only with checkout/assert.
# Keep isolated automated profiles separate from Smith's untouched review card.
set -euo pipefail
[[ $(hostname) == oryx-linuxlite && -n ${DISPLAY:-} && $PWD == /home/smith/Agon/mystuff/nurples/.emulator/mus001-prt-@(candidate|control)-r01 ]]
sha256sum --check --status .staged-card.sha256
SDL_AUDIODRIVER=dummy ./fab-agon-emulator > measure-runtime.log 2>&1 &
launcher=$!
# Only this Xvfb server contains the owned emulator window. Its SDL process
# handles TERM normally. The outer watchdog bounds startup and teardown.
win=$(timeout 12 xdotool search --sync --onlyvisible --name 'Fab Agon Emulator' | head -1)
pid=$(xdotool getwindowpid "$win")
[[ "$pid" =~ ^[1-9][0-9]+$ && $(readlink -f "/proc/$pid/exe") == "$PWD/runtime/fab-agon-emulator" ]]
trap 'kill -TERM "${pid:?}" 2>/dev/null || :; wait "$launcher" || :' EXIT
xdotool windowfocus "$win"
sleep 5
for attempt in 1 2; do
    xdotool type --delay 100 run
    xdotool key Return
    sleep 22
    import -window "$win" "measure-prompt-$attempt.png"
    xdotool keydown Escape
    sleep 0.22
    xdotool keyup Escape
    sleep 2
    import -window "$win" "measure-exit-$attempt.png"
done
kill -TERM "$pid"
wait "$launcher"
trap - EXIT
mcopy -i card.img ::/test/nurples-mode20-splash/LOADPRT.BIN measure.bin
