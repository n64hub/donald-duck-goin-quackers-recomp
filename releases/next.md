**Fixed:** the scene transition's spiral draws over the last picture every time; it could stand still on that picture and cut to black, or stop partway.

**Changed:** the renderer and runtime are updated to N64Recomp-browser v0.203.0's. A controller unplugged and plugged in again keeps its port, its player and its rumble; the window's last row of pixels is drawn; and when the renderer falls behind, the game waits for it 30 ms at a time, not 100.
