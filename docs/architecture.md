# How Donald Duck: Goin' Quackers Recompiled fits together

The facts about the game and its recompilation that the code can't say on its own. CLAUDE.md points here; a
finding that explains the game's code goes here or beside that code.

- **The code.** Three sections, stored raw: the boot segment (ROM 0x1000, VRAM 0x80000400), which the IPL3 copies with
  the first MB of ROM, and the main (0x80026D80) and support (0x800F2900) segments, which the game thread reads out of
  ROM once at boot and keeps. Nothing else is code: no overlay, nothing compressed. The runtime maps the MB after ROM
  0x1000 at 0x80000400 at start, so `src/main/register_overlays.cpp` maps main and support where the game reads them.
- **Spins.** The runtime runs one game thread at a time and switches only when the running one calls into it, where a
  console's interrupts preempt any thread. The game spins in three places that wait on its controller thread: for a
  Controller Pak request to end, and each frame for the controllers' read. Each pass yields (`us.toml`). Without them
  the game freezes at boot, on the screen after it reads the pak. A new hang
  with one thread busy and every other in `osRecvMesg` is another such loop: `--stacks` finds it.
- **The idle watchdog.** The idle thread waits for the main thread's OSThread state to read libultra's
  OS_STATE_STOPPED (1) and then breaks into the game's crash handler. ultramodern keeps its own states in that field,
  where 1 is QUEUED, so the hook in `us.toml` parks the idle thread for good instead. Without it the program ends
  with 0xC0000409 a few seconds in.
- **The clock.** The game zeroes libultra's time (`osSetTime`) at boot and before each of its intro screens, which wait
  on `osGetTime`, and its engine's timers read it too. The runtime's `osGetTime` counts from the program's start, so
  the game's `osSetTime` (0x80004320, which n64sym's signatures don't name) must carry its name for the runtime to
  replace it (`tools/gen_syms.py`). Without it the launcher's time was the game's: after a minute or two there the
  intro screens and the title ended at once, and the game went from one attract demo to the next, Start included.
- **The RSP's done message.** ultramodern tells a game a graphics task's RSP work is done as RT64 starts it, which
  holds for games that keep a list's inputs until the RDP is done too. This one changes them once the RSP is done, so
  `runtime-patches/` sends the message once RT64 has read the list. Without it a slow renderer (lavapipe, a page on a
  busy host) read lists being rewritten: a crash in `RT64::Interpreter::processDisplayLists` or "Unable to find a
  matching GBI".
- **One frame a VI.** The game keys its cache of texture loads by the retrace count a frame starts in, and has one
  display list buffer, so it takes for granted that no two frames start in one VI: a console's CPU never builds them
  that fast. The recomp's does, and such a frame calls the load blocks the frame before wrote, where its own commands
  now go: textures from the wrong place, or a call to itself that RT64 follows until the page ends with Aborted(OOM).
  A hook in `us.toml` holds a frame that would start in the same count until the next retrace.
- **The sound queue.** The game's sound engine queues calls in an array of 250 with no check of its size, emptied
  as each audio frame is made and filled each retrace with the stops of finished voices. A graphics list RT64 takes
  long to read (lavapipe building a pipeline) holds back the audio task, and after about 84 retraces the queue
  overwrites the voice table after it: "Failed to find function at 0x8019B..." at the next retrace. A hook in
  `us.toml` drops what would go past its end. The music's sequence player fills its own pool of events then too, with
  the MIDI messages the game sends each retrace, and once the pool is empty the event that wakes the player each audio
  frame can't post itself again: its loop spins for good on the next audio frame (a freeze, the audio thread at a whole
  core). A hook on `alEvtqPostEvent` keeps the pool's last event for it.
- **An animation's last frame.** A talk's script gives Donald his controls back once the speaker's end-of-talk
  animation reads finished, a flag the animation player sets at its last frame. The game's own 30 frames a second
  nearly always move an animation each frame; at 60 most frames don't, and the player's quick step for such a frame
  reached the last one and set the flag, only for the full step that the people Donald talks to also take each frame
  to loop the animation and clear it before the script looked. Donald stood frozen after the talk while the speaker
  talked on. A hook in `us.toml` leaves the last frame to the full step, which sets the flag and loops a frame
  later, as at 30.
- **Microcodes.** Graphics: F3DEX.NoN 1.23, which RT64 knows. Audio: `aspMain` (libultra's ABI 1), recompiled by
  RSPRecomp from `aspMain.us.toml`. librecomp's vector unit uses SSE4.1, so `RecompiledFuncs` builds with
  `-march=nehalem` on x86-64.
- **Saves.** The game saves only to a Controller Pak (libultra's `osPfs*`), so `main.cpp` declares no cartridge save
  and `src/game/controller_pak.cpp` keeps a pak of 123 pages in `saves/controllerpak_1.mpk`, the format emulators use.
  The game reads the controllers' status for a pak first, so port 1 reports one (`main.cpp`,
  `get_connected_device_info`); with none it never leaves the save slots' screen.
  Start held at boot opens the game's Controller Pak manager, whose font the game reads with CPU loads straight from
  the cartridge's address, which the runtime doesn't map: a hook in `us.toml` decodes it from the ROM the runtime holds
  (`controller_pak.cpp`). Without it, a player pressing Start through the logos crashed the program.
- **ROM patches.** A mod's `patch.bps` replaces the stored ROM after the runtime has copied the MB after ROM 0x1000
  into memory and before the game starts. The game reads main and support out of ROM itself, and its music, sounds and
  levels as it plays, so a patch of those takes; the boot segment's (0x1000 to 0x1DDF0) doesn't, and no patch of code
  does, the code being recompiled. The *Music Repair Patch* changes the sequences and banks past 0xCC000 only and plays
  as a mod (the README says how to make one).
- **The Expansion Pak.** The game finds the runtime's 8 MB and says so at boot ("Expansion Pak detected!").
- **Input.** RecompFrontend's default bindings, but for the stick and the C buttons (`src/game/config.cpp`). Keyboard:
  WASD and the arrows the stick, as the game's PC version's arrows are, Space A, Left Shift B, Q Z, R R, E L, Enter
  Start, T/F/G/H the C buttons. A pad's D-pad moves the stick too. The game reads port 1 only: it has no mode for a
  second player, so RecompFrontend's single-player mode plays every pad and the keyboard as port 1 (`main.cpp`), and
  N64Recomp-browser's new-recomp checklist's item 3 (every controller its own port) doesn't apply. Nothing the player
  presses turns the camera, which follows Donald where each level puts it: the C buttons play his gestures, and while
  any of them, L, R or Z is held the camera's matrix stays put (its superobject's pointer at 0x800E1E08, 0x2B8 past
  the engine's state as in Rayman 2, and its matrix at +0x20), in trail and in sidewalk. So the game has no mouse look:
  the General tab leaves out Mouse Sensitivity and Gyro Sensitivity, and the checklist's item 4a asks only for its
  keyboard defaults.
- **The newscast's skip.** A new game opens on a newscast in Gyro's lab that the game plays through, with no button to
  skip it. The lab's script plays it when two bytes the engine keeps across levels (0x800E282C, 0x800E282D) are 0: New
  Game zeroes them, and leaving the lab or loading a save sets both to 10. General's Skip Newscast (On by default) has
  Start during the newscast set them and reload the lab, which then starts as a load of New Game's own save does, with
  Donald standing in it (`src/game/newscast.cpp`); its Original plays it through.
- **Widescreen.** RecompFrontend's Graphics tab defaults to Aspect Ratio Expand, Resolution Auto and Framerate Display;
  RT64 widens the 3D. Original (Aspect Ratio Original) is the console's 4:3. A menu's backdrop (the pause menu's swirl,
  while 0x800C0640 is set) is an opaque gradient at 4:3 over the frame's clear, a fill of the level's fog colour that
  RT64 widens to the window's edges, so a hook in `us.toml` clears black under it: without it the sides show the level's
  colour. `src/game/widescreen.cpp` widens what the game decides for a 4:3 screen: its culling planes, its sprites and
  particles (projected on the CPU to texture rectangles and clipped to the screen, drawn past it with RT64's extended
  GBI under a scissor `RT64_SCISSOR_INSIDE_FRAMEBUFFER` counts only to the frame's edges), its full-screen fades, that
  backdrop's gradient and its HUD, whose bars and icons (a race's timer, the rings at the top corners with the stars'
  and the lives' counts) and the counters beside them it draws from the wide picture's edges with RT64's rectangle
  alignment (a dialogue, its speaker's face and other text stay in the middle), none of them at Original. The rings are
  sprites the game parks just past the 4:3 screen's top left in Gyro's lab, where a console shows none of them, so they
  get the game's own clip and not the sprites' widening, which drew a sliver of them at the top of the picture there.
  The scene transition, a spiral of black the CPU writes into
  the framebuffer, is stretched across the picture by `RT64_STRETCH_CPU_CHANGES`, and `src/game/transition.cpp` keeps
  its map out of a framebuffer that is still shown. The spiral is written into the last frame the game drew, which RT64 also shows
  interpolated at the display's rate from copies that never see the CPU's writes, and RT64 drops a present that falls
  behind and with it its share of them: `runtime-patches/rt64-1-*` shows them, or the last frame could stand still and
  cut to black, or the spiral stop partway. The newscast that starts a new game opens behind a grey stone plate
  drawn in 3D just in front of the camera, sized to the 4:3 screen, which `RT64_STRETCH_SCREEN_COVERS` stretches across
  the picture. `CMakeLists.txt` sets all three options. `widescreen.cpp` also joins the title logo's overlapping tiles
  at every aspect, each tile's clear edge given its neighbour's texels, which upscaled showed their seams.
- **Render to RAM.** RT64's exact copy of each frame into RDRAM, on the page too (`main.cpp` says why): the scene
  transition's spiral is the CPU's writes into the frame the RDP drew, after it is drawn, so neither off nor the copy a
  list late (N64Recomp-browser's checklist, item 7) fits.
- **Testing.** `--boot` starts the game from the launcher's first frame once the ROM is stored (`--boot-after
  <seconds>` that long after it, as a player who leaves the launcher open), `--window <w>x<h>`
  sizes the window, `--input <file>` plays port 1 from a script of timed button states and changes of level, as the
  page's `tools/survey.js` makes them, to reach any level from a new game (`main.cpp` says the format),
  and on Windows `--stacks <seconds>` prints every thread's stack that long after start, for unattended runs and
  hangs.
