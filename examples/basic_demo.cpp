#ifndef EUI_ENABLE_GLFW_OPENGL_BACKEND
#define EUI_ENABLE_GLFW_OPENGL_BACKEND 1
#endif
#include "EUI.h"

#include <algorithm>

// Global style controls for quick tuning.
eui::Color g_theme_primary = eui::rgba(0.282f, 0.827f, 0.835f, 1.0f);
float g_theme_radius = 10.0f;
bool g_vsync = true;
bool g_animate_progress = true;
double g_max_fps = 240.0;

struct DemoState {
    bool dark_mode{true};
    int active_tab{0};
    bool react_selected{true};
    bool vue_selected{false};
    bool svelte_selected{true};
    bool advanced_open{false};
    bool debug_mode{true};

    float opacity{65.0f};
    float exposure{128.0f};
    float log_level{5.0f};
    float gamma{2.20f};
    float loading_ratio{0.45f};
};

int main() {
    DemoState state{};
    eui::demo::AppOptions options{};
    options.vsync = g_vsync;
    options.continuous_render = false;
    options.max_fps = g_max_fps;
    options.width = 540;
    options.height = 960;


    return eui::demo::run(
        [&](eui::demo::FrameContext frame) {
            auto& ui = frame.ui;

            if (g_animate_progress) {
                state.loading_ratio += frame.dt * 0.20f;
                if (state.loading_ratio > 1.0f) {
                    state.loading_ratio = 0.0f;
                }
                frame.request_next_frame();
            }

            ui.set_theme_mode(state.dark_mode ? eui::ThemeMode::Dark : eui::ThemeMode::Light);
            ui.set_primary_color(g_theme_primary);
            ui.set_corner_radius(g_theme_radius);

            const float content_width =
                std::min(680.0f, std::max(360.0f, static_cast<float>(frame.framebuffer_w) - 80.0f));
            const float content_x = (static_cast<float>(frame.framebuffer_w) - content_width) * 0.5f;
            ui.begin_panel("", content_x, 18.0f, content_width, 0.0f, -1.0f);

            ui.begin_row(2, 10.0f);
            ui.label("EUI SYSTEM", 24.0f, false);
            if (ui.button(state.dark_mode ? "SWITCH TO LIGHT" : "SWITCH TO DARK",
                          eui::ButtonStyle::Secondary, 30.0f)) {
                state.dark_mode = !state.dark_mode;
            }
            ui.end_row();
            ui.label("EFFICIENCY-FOCUSED UI COMPONENTS WITH SHADCN-STYLE DETAILS", 12.0f, true);
            ui.spacer(6.0f);

            ui.begin_card("BUTTONS");
            ui.begin_row(3, 8.0f);
            ui.button("PRIMARY ACTION", eui::ButtonStyle::Primary);
            ui.button("SECONDARY", eui::ButtonStyle::Secondary);
            ui.button("GHOST", eui::ButtonStyle::Ghost);
            ui.end_row();
            ui.end_card();

            ui.begin_card("TAB SELECTION");
            ui.label("SINGLE SELECT (RADIO)", 11.0f, true);
            ui.begin_row(3, 6.0f);
            if (ui.tab("EFFICIENCY", state.active_tab == 0)) {
                state.active_tab = 0;
            }
            if (ui.tab("QUALITY", state.active_tab == 1)) {
                state.active_tab = 1;
            }
            if (ui.tab("SPEED", state.active_tab == 2)) {
                state.active_tab = 2;
            }
            ui.end_row();
            ui.spacer(6.0f);
            ui.label("MULTI SELECT (CHECKBOX)", 11.0f, true);
            ui.begin_row(3, 6.0f);
            if (ui.tab("REACT", state.react_selected)) {
                state.react_selected = !state.react_selected;
            }
            if (ui.tab("VUE", state.vue_selected)) {
                state.vue_selected = !state.vue_selected;
            }
            if (ui.tab("SVELTE", state.svelte_selected)) {
                state.svelte_selected = !state.svelte_selected;
            }
            ui.end_row();
            ui.end_card();

            ui.begin_card("SLIDERS");
            ui.label("RIGHT-CLICK VALUE TO EDIT DIRECTLY", 11.0f, true);
            ui.slider_float("OPACITY", state.opacity, 0.0f, 100.0f, 0);
            ui.slider_float("EXPOSURE", state.exposure, 0.0f, 255.0f, 0);
            ui.end_card();

            ui.begin_card("INPUT");
            ui.input_float("GAMMA", state.gamma, 0.10f, 4.00f, 2);
            ui.end_card();

            ui.begin_card("PROGRESS");
            ui.progress("LOADING ASSETS...", state.loading_ratio, 8.0f);
            ui.end_card();

            ui.begin_card("DROPDOWN PANEL");
            if (ui.begin_dropdown("ADVANCED SETTINGS", state.advanced_open, 0.0f)) {
                ui.begin_row(2, 6.0f);
                if (ui.tab("DEBUG ON", state.debug_mode)) {
                    state.debug_mode = true;
                }
                if (ui.tab("DEBUG OFF", !state.debug_mode)) {
                    state.debug_mode = false;
                }
                ui.end_row();
                ui.slider_float("LOG LEVEL", state.log_level, 0.0f, 10.0f, 1, 34.0f);
                ui.end_dropdown();
            }
            ui.end_card();

            ui.end_panel();
        },
        options);
}
