#ifndef __DDGQ_H__
#define __DDGQ_H__

#include <cstdint>
#include <string_view>

namespace dq {
    inline constexpr std::u8string_view program_id = u8"GoinQuackersRecompiled";
    inline constexpr std::string_view program_name = "Donald Duck: Goin' Quackers Recompiled";

    // The game's code sections (register_overlays.cpp) and the patches over it (register_patches.cpp).
    void register_overlays();
    void register_patches();

    // The settings (config.cpp): RecompFrontend's tabs.
    void init_config();

    // Maps the main and support segments at the addresses the game reads them to (register_overlays.cpp), before the
    // game runs.
    void map_code_sections();
}

#endif
