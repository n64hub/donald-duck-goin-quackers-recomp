// Skip Newscast (General tab): Start skips the newscast that a new game opens on in Gyro's lab, which the game plays
// through with no button to skip it. The lab's script decides at its start whether to play it, by two bytes it keeps
// across levels (0x800E282C and 0x800E282D, read into the lab's script by its procedure 0x62): New Game zeroes them,
// and leaving the lab or loading a save sets both to 10, after which the lab starts with Donald standing in it. So
// Start while the newscast plays (the lab, mapmonde, with the engine's cinematic flag 0x800C3D51 set and both bytes
// still 0) sets them to 10 and reloads the lab, as a load of New Game's own save would start it. Original plays the
// newscast through, as the console does. main.cpp passes port 1's buttons here; us.toml's hook calls the frame's half
// as each frame starts.
#include <atomic>
#include <cstdint>
#include <string>
#include <vector>

#include "recomp.h"
#include "recompui/config.h"
#include "ddgq.h"

namespace {
    enum class SkipNewscast {
        Original,
        On,
        OptionCount
    };

    const std::vector<recomp::config::ConfigOptionEnumOption> skip_newscast_options = {
        {SkipNewscast::Original, "Original", "Original"},
        {SkipNewscast::On, "On", "On"},
    };

    const std::string skip_newscast_option = "skip_newscast";

    constexpr uint16_t start_button = 0x1000;

    // Start pressed since the last frame (a press, not a hold).
    std::atomic<bool> start_pressed = false;
}

void dq::add_newscast_option(recomp::config::Config& general) {
    general.add_enum_option(skip_newscast_option, "Skip Newscast",
        "Lets Start skip the newscast that a new game opens on."
        "<br/><b>On:</b> Start during the newscast skips it, and Donald stands in Gyro's lab."
        "<br/><b>Original:</b> the newscast plays through, as on the console.",
        skip_newscast_options, SkipNewscast::On);
}

void dq::newscast_input(uint16_t buttons) {
    static bool held = false;
    bool now = (buttons & start_button) != 0;
    if (now && !held) {
        start_pressed = true;
    }
    held = now;
}

extern "C" void dq_newscast_frame(uint8_t* rdram) {
    if (!start_pressed.exchange(false)) {
        return;
    }
    if (std::get<uint32_t>(recompui::config::get_general_config().get_option_value(skip_newscast_option)) !=
        (uint32_t)SkipNewscast::On) {
        return;
    }
    // The engine's state: its mode (9 plays a level, 6 changes to the next one) and the current and next levels' names.
    constexpr int32_t mode = (int32_t)0x800E1B50;
    constexpr int32_t level = (int32_t)0x800E1B54;
    constexpr int32_t next_level = (int32_t)0x800E1B72;
    const char lab[] = "mapmonde";
    for (int i = 0; i < (int)sizeof(lab); i++) {
        if (MEM_BU(i, (gpr)level) != (uint8_t)lab[i]) {
            return;
        }
    }
    if (MEM_W(0, (gpr)mode) != 9 || MEM_BU(0, (gpr)(int32_t)0x800C3D51) == 0 ||
        MEM_BU(0, (gpr)(int32_t)0x800E282C) != 0 || MEM_BU(0, (gpr)(int32_t)0x800E282D) != 0) {
        return;
    }
    MEM_B(0, (gpr)(int32_t)0x800E282C) = 10;
    MEM_B(0, (gpr)(int32_t)0x800E282D) = 10;
    // A change of level, as the game's own makes one (func_8003F8E4 copies the name, func_8003F648 sets the mode).
    for (int i = 0; i < (int)sizeof(lab); i++) {
        MEM_B(i, (gpr)next_level) = lab[i];
    }
    MEM_W(0, (gpr)mode) = 6;
}
