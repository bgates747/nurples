#!/bin/bash
# Automated clock/cue checks only; private Xvfb and current Oryx lease.
set -euo pipefail
[[ $(hostname) == oryx-linuxlite && -n ${DISPLAY:-} ]]
if [[ $PWD == /home/smith/Agon/mystuff/nurples/.emulator/mus001-prt-cuecheck-r06 ]]; then
    sha256sum --check --status .cuecheck-card.sha256
else
    [[ $PWD == /home/smith/Agon/mystuff/nurples/.emulator/mus001-prt-@(all50|flow50)-r05 ]]
    sha256sum --check --status .staged-card.sha256
fi
[[ -x ./fab-agon-emulator && ! -L ./fab-agon-emulator ]]
evidence=$(mktemp -d -- "$PWD/measure.XXXXXXXX")
[[ -n "$evidence" && -d "$evidence" && "$evidence" == "$PWD"/measure.* ]]
echo "Evidence: $evidence"
SDL_AUDIODRIVER=dummy setsid ./fab-agon-emulator > "$evidence/r05-runtime.log" 2>&1 &
leader=$!
cleanup() {
    # Only signal the process group we just created; never match by emulator name.
    if kill -0 "$leader" 2>/dev/null; then
        [[ $(ps -o pgid= -p "$leader" | tr -d ' ') == "$leader" ]] || return 1
        kill -TERM -- "-$leader"
    fi
    wait "$leader" || [[ $? == 143 ]]
}
trap cleanup EXIT
win=$(timeout 15 xdotool search --sync --onlyvisible --name 'Fab Agon Emulator' | head -1)
pid=$(xdotool getwindowpid "$win")
[[ "$pid" =~ ^[1-9][0-9]+$ && $(readlink -f "/proc/$pid/exe") == "$PWD/runtime/fab-agon-emulator" ]]
[[ $(ps -o pgid= -p "$pid" | tr -d ' ') == "$leader" ]]
xdotool windowfocus "$win"
command_line() {
    xdotool type --delay 95 "$1"
    xdotool key Return
    sleep 2 # MOS may still be opening a file; do not lose the next command's first key.
}
tap() { xdotool keydown "$1"; sleep 0.2; xdotool keyup "$1"; }
sleep 5
if [[ $PWD == */mus001-prt-cuecheck-r06 ]]; then
    command_line 'cd /r06check'
    command_line 'exec checks.txt'
    sleep 8
    import -window "$win" "$evidence/mos.png"
    kill -TERM "$pid"
    wait "$leader"
    trap - EXIT
    mcopy -n -i card.img '::/r06check/o*.bin' "$evidence/"
    exit
fi
if [[ $PWD == */mus001-prt-flow50-r05 ]]; then
    # Resolve and validate the two DATA fields against this binary,
    # rather than hard-coding addresses or patching any executable code.
    address() {
        local symbol=$1 offset=$2 expected=$3 size=$4 raw addr actual length
        raw=$(awk -v s="$symbol" '$1==s {print $2}' payload/nurples.symbols | tr -d '\r' | sort -u) || return 1
        [[ "$raw" =~ ^\$([0-9a-fA-F]+)$ ]] || return 1
        addr=$((16#${BASH_REMATCH[1]}+offset))
        length=$(stat -c%s payload/nurples.bin) || return 1
        [[ "$length" =~ ^[1-9][0-9]*$ && $addr -ge 262144 && $((addr+size)) -le $((262144+length)) ]] || return 1
        actual=$(od -An -tx1 -N "$size" -j "$((addr-262144))" payload/nurples.bin | xargs) || return 1
        [[ "$actual" == "$expected" ]] || return 1
        printf '%d' "$addr"
    }
    rows=$(address tileset_00_level_00 1 '00 01 00' 3)
    shields=$(address player_shields_max 0 '40' 1)
    command_line "load row.bin $rows"
    command_line "load one.bin $shields"
    # Three lives untouched: unattended collisions should exercise death and
    # respawn before Game Over. The second level and every actor are unmodified.
fi
command_line run
sleep 23
tap n
sleep 4
tap space
if [[ $PWD == */mus001-prt-flow50-r05 ]]; then
    for sample in 1 2 3 4 5; do
        sleep 15
        import -window "$win" "$evidence/r05-flow-$sample.png"
    done
    tap Escape
    sleep 2
    kill -TERM "$pid"
    wait "$leader"
    trap - EXIT
    mcopy -n -i card.img ::/test/nurples-mode20-splash/MUSIC50.BIN "$evidence/r05-FLOW.BIN"
    exit
fi
sleep 8
tap p
sleep 3
tap p
sleep 5
tap Escape
sleep 2
command_line 'rename music50.bin FIRST.BIN'
sleep 1
command_line run
sleep 23
import -window "$win" "$evidence/r05-second-prompt.png"
tap Escape
sleep 2
import -window "$win" "$evidence/r05-second-exit.png"
kill -TERM "$pid"
wait "$leader"
trap - EXIT
for name in FIRST.BIN MUSIC50.BIN; do
    mcopy -n -i card.img "::/test/nurples-mode20-splash/$name" "$evidence/r05-$name"
done
