// The settings: RecompFrontend's General, Controls, Graphics, Sound and Mods tabs, with its default bindings. Its own
// Graphics options carry what a recomp needs: Resolution and MS Anti-Aliasing (high resolution), Framerate
// (interpolation to the display's rate) and Aspect Ratio and HUD Placement (widescreen). Its default bindings change in
// one way: the arrow keys and a pad's D-pad move Donald too, as the game's PC version's arrows do. General adds one
// option of the game's, Skip Newscast (newscast.cpp).

#include <filesystem>

#include "recompinput/recompinput.h"
#include "recompui/config.h"
#include "util/file.h"
#include "ddgq.h"

// The stock defaults put the C buttons on the arrow keys and leave the stick on WASD alone. Here the arrows are a second
// stick (walking and menus, like WASD) and the C buttons move to a T/F/G/H diamond; a pad's D-pad moves the stick as well
// as pressing the N64 D-pad. Only defaults: saved bindings still apply.
static void set_default_bindings() {
    using recompinput::GameInput;
    using recompinput::InputField;
    recompinput::set_default_mapping_for_keyboard(GameInput::X_AXIS_NEG,
        { InputField::keyboard(SDL_SCANCODE_A), InputField::keyboard(SDL_SCANCODE_LEFT) });
    recompinput::set_default_mapping_for_keyboard(GameInput::X_AXIS_POS,
        { InputField::keyboard(SDL_SCANCODE_D), InputField::keyboard(SDL_SCANCODE_RIGHT) });
    recompinput::set_default_mapping_for_keyboard(GameInput::Y_AXIS_POS,
        { InputField::keyboard(SDL_SCANCODE_W), InputField::keyboard(SDL_SCANCODE_UP) });
    recompinput::set_default_mapping_for_keyboard(GameInput::Y_AXIS_NEG,
        { InputField::keyboard(SDL_SCANCODE_S), InputField::keyboard(SDL_SCANCODE_DOWN) });
    recompinput::set_default_mapping_for_keyboard(GameInput::C_UP, { InputField::keyboard(SDL_SCANCODE_T) });
    recompinput::set_default_mapping_for_keyboard(GameInput::C_LEFT, { InputField::keyboard(SDL_SCANCODE_F) });
    recompinput::set_default_mapping_for_keyboard(GameInput::C_DOWN, { InputField::keyboard(SDL_SCANCODE_G) });
    recompinput::set_default_mapping_for_keyboard(GameInput::C_RIGHT, { InputField::keyboard(SDL_SCANCODE_H) });

    recompinput::set_default_mapping_for_controller(GameInput::X_AXIS_NEG, {
        InputField::controller_analog(SDL_CONTROLLER_AXIS_LEFTX, false),
        InputField::controller_digital(SDL_CONTROLLER_BUTTON_DPAD_LEFT) });
    recompinput::set_default_mapping_for_controller(GameInput::X_AXIS_POS, {
        InputField::controller_analog(SDL_CONTROLLER_AXIS_LEFTX, true),
        InputField::controller_digital(SDL_CONTROLLER_BUTTON_DPAD_RIGHT) });
    recompinput::set_default_mapping_for_controller(GameInput::Y_AXIS_POS, {
        InputField::controller_analog(SDL_CONTROLLER_AXIS_LEFTY, false),
        InputField::controller_digital(SDL_CONTROLLER_BUTTON_DPAD_UP) });
    recompinput::set_default_mapping_for_controller(GameInput::Y_AXIS_NEG, {
        InputField::controller_analog(SDL_CONTROLLER_AXIS_LEFTY, true),
        InputField::controller_digital(SDL_CONTROLLER_BUTTON_DPAD_DOWN) });
}

void dq::init_config() {
    set_default_bindings();

    std::filesystem::path app_dir = recompui::file::get_app_folder_path();
    if (!app_dir.empty()) {
        std::filesystem::create_directories(app_dir);
    }

    recompui::config::GeneralTabOptions general_options{};
    general_options.has_rumble_strength = true;
    general_options.has_gyro_sensitivity = true;
    general_options.has_mouse_sensitivity = true;
    dq::add_newscast_option(recompui::config::create_general_tab(general_options));
    recompui::config::create_controls_tab();
    recompui::config::create_graphics_tab();
    recompui::config::create_sound_tab();
    recompui::config::create_mods_tab();
    recompui::config::finalize();
}
