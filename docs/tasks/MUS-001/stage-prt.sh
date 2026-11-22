#!/usr/bin/env bash
# Stage the private PT3 audition files only. FAE launch needs a separate Oryx checkout.
set -euo pipefail
repo=/home/smith/Agon/mystuff/nurples
arm=${1:?specify control or candidate}
[[ $arm == control || $arm == candidate || $arm == review ]]
work=$repo/.build/mus001-prt-r01
host=oryx-linuxlite
profile=$repo/.emulator/mus001-prt-$arm-r01
ssh_cmd=(ssh -o BatchMode=yes -o ConnectTimeout=6 "$host")
bundle=$work/$arm
if [[ $arm == review ]]; then bundle=$work/candidate; fi

(cd "$bundle"; sha256sum -c SHA256SUMS)

"${ssh_cmd[@]}" bash -s -- "$profile" <<'REMOTE' > "$work/$arm-donor.txt"
set -euo pipefail
repo=/home/smith/Agon/mystuff/nurples
parent=$repo/.emulator
profile=$1
source=$parent/opening-warp-fae-r01
[[ $(hostname) == oryx-linuxlite && $(id -un) == smith ]]
[[ "$profile" == $parent/mus001-prt-@(control|candidate|review)-r01 && -d "$repo" && -O "$repo" ]]
[[ ! -L "$repo" && -d "$parent" && ! -L "$parent" && -d "$source" && ! -L "$source" ]]
[[ ! -e "$profile" && ! -L "$profile" ]]
cd "$source"
sha256sum -c .runtime.sha256
sha256sum -c .mos-release.sha256
sha256sum -c .staged-card.sha256
sha256sum -c - <<'PIN'
78b9fd50b24df761d7f056899d5b94b6f309a469fc323673c6f51d8fd405cedc  fab-agon-emulator.bin
49d4e430870fd3a264b10645166d4b25a1ba2ab12eee0c94fc7bed34fab511fd  vdp_platform.so
d7a51147f3381cd68d56f3fbe4605434a897dfae1b049269afeba09ec2b7eb50  mos_platform.bin
PIN
umask 077
mkdir -- "$profile"
mkdir -- "$profile/"{runtime,sdcard,readback} "$profile/readback/fonts"
cp -- card.img fab-agon-emulator vdp_platform.so mos_platform.bin mos_platform.map \
    .bespoke-vdp-profile .mos-release .mos-release.sha256 .runtime.sha256 "$profile/"
cp -- runtime/fab-agon-emulator "$profile/runtime/"
ln -s runtime/fab-agon-emulator "$profile/fab-agon-emulator.bin"
REMOTE
scp -q -p -o BatchMode=yes -o ConnectTimeout=6 -r "$bundle" "$host:$profile/payload"
"${ssh_cmd[@]}" bash -s -- "$profile" <<'REMOTE' > "$work/$arm-staging.txt"
set -euo pipefail
profile=$1
[[ "$profile" == /home/smith/Agon/mystuff/nurples/.emulator/mus001-prt-@(control|candidate|review)-r01 ]]
[[ $(hostname) == oryx-linuxlite && $(id -un) == smith && -d "$profile" && ! -L "$profile" ]]
cd "$profile"
[[ -x fab-agon-emulator && ! -L fab-agon-emulator && ! -L card.img ]]
[[ ! -e firmware && $(<.bespoke-vdp-profile) == vdp_platform.so ]]
bash -n fab-agon-emulator
sha256sum -c .runtime.sha256
sha256sum -c .mos-release.sha256
(cd payload; sha256sum -c SHA256SUMS)
guest=::/test/nurples-mode20-splash
for item in nurples.bin voice.agnb ui.agnb game.agnb fonts/Lat38-VGA8_8x8.font; do
    mcopy -o -i card.img "payload/$item" "$guest/$item"
    mcopy -i card.img "$guest/$item" "readback/$item"
    cmp "payload/$item" "readback/$item"
done
cp -- payload/nurples.symbols readback/nurples.symbols
startup=$(mtype -i card.img ::/autoexec.txt | tr -d '\r')
[[ "$startup" == $'SET KEYBOARD 1\nCD /test/nurples-mode20-splash\nLOAD nurples.bin &040000' ]]
printf '%s\n' "$startup"
(cd readback; sha256sum -c ../payload/SHA256SUMS)
sha256sum card.img | tee .staged-card.sha256
REMOTE
tail -10 "$work/$arm-staging.txt"
