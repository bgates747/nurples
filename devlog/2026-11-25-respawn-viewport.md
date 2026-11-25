# Respawn: turret in the lives display

Redrawing the remaining ships on a nonterminal death left the lives window's
graphics origin and viewport active for the rest of that frame's world
updates. The respawn path now restores the playfield immediately after the
HUD redraw, before active tiles can plot. Normal frames are unchanged.

Smith observed no problem on dying in FAE and accepted the fix. Death timed
to coincide with a turret entering the field was not independently verified.
