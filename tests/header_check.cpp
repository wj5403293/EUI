#include "EUI.h"

int main() {
    eui::Context ui;
    eui::InputState input{};

    ui.begin_frame(1280.0f, 720.0f, input);
    ui.begin_panel("HEADER CHECK", 20.0f, 20.0f, 640.0f);

    float value = 0.5f;
    bool open = false;
    ui.button("BUTTON");
    ui.slider_float("VALUE", value, 0.0f, 1.0f);
    ui.progress("PROGRESS", 0.4f);
    if (ui.begin_dropdown("MORE", open, 80.0f)) {
        ui.end_dropdown();
    }

    ui.end_panel();
    (void)ui.end_frame();
    return 0;
}
