# M toggles the soundtrack

M is a permanent player control, not a diagnostics flag. After loading, a
fresh M press switches off music without pausing gameplay, SFX or speech.
Stop the owned 50 Hz music timer and its interrupts, silence only the music
voices, and stop servicing or advancing the tracker. A second press resumes
the current track without replaying missed time. If the level changed while
off, start the correct new track. P remains a separate game pause. M alone
must not dismiss an attract or Game Over prompt.

Start each MOS run with music enabled. Retain the setting across death, warp
and New Game. Preserve timer/vector ownership and MOS cleanup even if exiting
with music off. Keep the shield klaxon-to-speech delay independent of music.

Qualify M separately on FAE before committing its production rollback point.
Do not fold the unaccepted hardware-sprite conversion or any rendering/scroll
diagnostic switches into that checkpoint.
