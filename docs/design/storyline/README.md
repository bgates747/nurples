# Purple Nurples — campaign storyline

Earth is attacked by a strange alien species. We launch to save the facility that keeps the planet alive, fight our way to the attackers’ homeworld, and strike the resources that sustain their campaign. The mission ends where it began: we have to fight our way back to our own airfield. This is the emerging story, not the shipping level list or an implementation contract.

## Campaign beats

| Stage | Where we are | What happens |
| --- | --- | --- |
| 1. Scramble | Earth | Take off from the airfield. Eliminate the attackers **before the end of the level**. If they reach the critical facility, they destroy it and Earth cannot survive. Simply staying alive until the map runs out is not a victory. |
| 2. Pursuit | Outbound space | Waves of enemies against a bare scrolling starfield. We are fighting our way through to the aliens’ outer planetary defenses. |
| 3. Outer defenses | Alien space station | Breach the defenses. The visual reference is the **current third level** (the station-like map that used to open the game). |
| 4. Approach | Deeper space | A second open-starfield run, more intense than the first. Beyond these waves lies the home planet. |
| 5. Planetary strike | Alien homeworld | Destroy their main planetary power center, or an equivalent critical resource. The exact target is undecided. Art for the planet has not yet been developed or sourced. |
| 6. Breakout | Back into space | Fight our way out after the strike. This could be another station-themed level using stage 3’s art. That setting is an option, not yet a decision. |
| 7. Homeward run | Return space | One last bare-starfield passage, this time headed for Earth. Getting away from their planet does not mean the fight at home is over. |
| 8. Final approach | Earth | Use previously unused Xevious terrain for the run-in. A major alien force makes a last-ditch attempt to take Earth: with their homeworld’s resources critically damaged, they need ours more than ever. Defeat them and **land at the same airfield we took off from**. |

See [boss encounters](../enemies/bosses/README.md) for possible placements along this route; none are fixed yet.

The first two stages also contrast in how their enemies move: a formation advance on Earth, then formation breakaways in space. See [opening-stage movement direction](../enemies/README.md#opening-stages--chosen-movement-direction-not-yet-implemented).

## Stage 1 breach ending — undecided

**The rule.** A Space Invaders-style option: a single living member of an Earth wave getting through costs the mission, however many ships the player has left. A shot-down alien does not count. Two possible thresholds: lose when the alien would normally despawn completely off the bottom, or when its leading edge first touches the bottom so the player can still see the culprit. If adopted, the shared Earth march handler could report a breach; the game would finish processing the frame and take the transition once, even if several aliens crossed together. This could be a distinct ending from losing the last player ship. None of these rules are in force.

**What the aliens came to destroy.** “Earth system survival complex” is a working description, not a name. Two alternatives:

- **Earthshield Control** maintains the planetary shield. Why are the attackers already inside it? Perhaps a small vanguard slipped through alongside authorized traffic. Briefing idea: *“Hold the line. If even one invader gets through, Earthshield falls.”*
- **Earth Defense Command** coordinates orbital defenses against the main fleet. The ships on the map are a raiding party: they only need to silence one ground station. This shifts the stakes from immediate loss of a life-support facility to opening Earth to invasion. Briefing idea: *“If they knock out Defense Command, we can’t stop the main fleet.”*

**What the player sees.** On a breach, cut player control and warp to the facility at the far end of the map. Brake in time to see a radar dish turning or a beacon pulsing. The aliens attack; the dish stops or the beacon goes dark. The regular **GAME OVER** graphic flashes while the attackers do a victory dance. One inexpensive choreography idea: reuse their march for two short steps left, two right, in unison. For the Defense Command version, the final shot could show the main fleet sailing overhead unopposed. The facility, staging, choreography and trigger all remain open.

## Keep open

- What is Earth's life-sustaining facility? Which attackers count toward the deadline, and how does the player know any are still alive?
- What exactly do we disable on the alien planet? The premise requires a critical loss of resources, not necessarily destruction of the planet itself.
- Is the final force a rear guard, a force left behind on Earth, or something else? Its desperation and the landing are the important beats; its designation is provisional.

The [Earth City development record](../../tasks/EARTH-001/TASK.md) owns the playable opening and its art pipeline. The current four-level build is a starting point; these notes do not change its maps, objectives, or transitions.
