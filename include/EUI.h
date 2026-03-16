#ifndef EUI_H_
#define EUI_H_

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#ifdef EUI_ENABLE_GLFW_OPENGL_BACKEND
#include <GLFW/glfw3.h>

#include <array>
#include <iostream>
#endif

namespace eui {

struct Color {
    float r{1.0f};
    float g{1.0f};
    float b{1.0f};
    float a{1.0f};
};

inline Color rgba(float r, float g, float b, float a = 1.0f) {
    return Color{r, g, b, a};
}

inline Color mix(const Color& lhs, const Color& rhs, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    return Color{
        lhs.r + (rhs.r - lhs.r) * t,
        lhs.g + (rhs.g - lhs.g) * t,
        lhs.b + (rhs.b - lhs.b) * t,
        lhs.a + (rhs.a - lhs.a) * t,
    };
}

struct Rect {
    float x{0.0f};
    float y{0.0f};
    float w{0.0f};
    float h{0.0f};

    bool contains(float px, float py) const {
        return px >= x && px <= x + w && py >= y && py <= y + h;
    }
};

enum class ThemeMode {
    Light,
    Dark,
};

enum class ButtonStyle {
    Primary,
    Secondary,
    Ghost,
};

struct Theme {
    Color background{};
    Color panel{};
    Color panel_border{};
    Color text{};
    Color muted_text{};
    Color primary{};
    Color primary_text{};
    Color secondary{};
    Color secondary_hover{};
    Color secondary_active{};
    Color track{};
    Color track_fill{};
    Color outline{};
    Color input_bg{};
    Color input_border{};
    Color focus_ring{};
    float radius{8.0f};
};

inline Theme make_theme(ThemeMode mode, const Color& primary) {
    Theme theme{};
    theme.primary = primary;
    theme.radius = 8.0f;

    if (mode == ThemeMode::Dark) {
        theme.background = rgba(0.05f, 0.07f, 0.10f, 1.0f);
        theme.panel = rgba(0.09f, 0.12f, 0.16f, 1.0f);
        theme.panel_border = rgba(0.18f, 0.23f, 0.30f, 1.0f);
        theme.text = rgba(0.92f, 0.95f, 0.98f, 1.0f);
        theme.muted_text = rgba(0.63f, 0.70f, 0.79f, 1.0f);
        theme.primary_text = rgba(0.06f, 0.10f, 0.17f, 1.0f);
        theme.secondary = rgba(0.15f, 0.20f, 0.27f, 1.0f);
        theme.secondary_hover = mix(theme.secondary, theme.primary, 0.18f);
        theme.secondary_active = mix(theme.secondary, theme.primary, 0.32f);
        theme.track = rgba(0.18f, 0.23f, 0.31f, 1.0f);
        theme.track_fill = theme.primary;
        theme.outline = rgba(0.25f, 0.31f, 0.40f, 1.0f);
        theme.input_bg = rgba(0.08f, 0.11f, 0.15f, 1.0f);
        theme.input_border = mix(rgba(0.26f, 0.33f, 0.42f, 1.0f), theme.primary, 0.20f);
        theme.focus_ring = mix(theme.primary, rgba(1.0f, 1.0f, 1.0f, 1.0f), 0.18f);
    } else {
        theme.background = rgba(0.96f, 0.97f, 0.99f, 1.0f);
        theme.panel = rgba(1.0f, 1.0f, 1.0f, 1.0f);
        theme.panel_border = rgba(0.84f, 0.88f, 0.93f, 1.0f);
        theme.text = rgba(0.11f, 0.15f, 0.22f, 1.0f);
        theme.muted_text = rgba(0.41f, 0.47f, 0.58f, 1.0f);
        theme.primary_text = rgba(0.96f, 0.98f, 1.0f, 1.0f);
        theme.secondary = rgba(0.92f, 0.94f, 0.97f, 1.0f);
        theme.secondary_hover = mix(theme.secondary, theme.primary, 0.12f);
        theme.secondary_active = mix(theme.secondary, theme.primary, 0.24f);
        theme.track = rgba(0.90f, 0.92f, 0.96f, 1.0f);
        theme.track_fill = theme.primary;
        theme.outline = rgba(0.80f, 0.85f, 0.92f, 1.0f);
        theme.input_bg = rgba(1.0f, 1.0f, 1.0f, 1.0f);
        theme.input_border = mix(rgba(0.79f, 0.84f, 0.91f, 1.0f), theme.primary, 0.28f);
        theme.focus_ring = mix(theme.primary, rgba(1.0f, 1.0f, 1.0f, 1.0f), 0.10f);
    }
    return theme;
}

struct InputState {
    float mouse_x{0.0f};
    float mouse_y{0.0f};
    bool mouse_down{false};
    bool mouse_pressed{false};
    bool mouse_released{false};

    bool mouse_right_down{false};
    bool mouse_right_pressed{false};
    bool mouse_right_released{false};

    bool key_backspace{false};
    bool key_enter{false};
    bool key_escape{false};
    std::string text_input{};
    double time_seconds{0.0};
};

enum class CommandType {
    FilledRect,
    RectOutline,
    Text,
};

enum class TextAlign {
    Left,
    Center,
    Right,
};

struct DrawCommand {
    CommandType type{CommandType::FilledRect};
    Rect rect{};
    Color color{};
    std::uint32_t text_offset{0};
    std::uint32_t text_length{0};
    float font_size{13.0f};
    TextAlign align{TextAlign::Left};
    float radius{0.0f};
    float thickness{1.0f};
};

class Context {
public:
    Context() {
        commands_.reserve(1024);
        scope_stack_.reserve(24);
        text_arena_.reserve(16 * 1024);
        input_buffer_.reserve(64);
        theme_.radius = corner_radius_;
    }

    void set_theme_mode(ThemeMode mode) {
        if (theme_mode_ == mode) {
            return;
        }
        theme_mode_ = mode;
        refresh_theme();
    }

    ThemeMode theme_mode() const {
        return theme_mode_;
    }

    void set_primary_color(const Color& color) {
        primary_color_ = color;
        refresh_theme();
    }

    void set_corner_radius(float radius) {
        corner_radius_ = std::clamp(radius, 0.0f, 28.0f);
        theme_.radius = corner_radius_;
    }

    float corner_radius() const {
        return corner_radius_;
    }

    const Theme& theme() const {
        return theme_;
    }

    void begin_frame(float width, float height, const InputState& input) {
        frame_width_ = std::max(1.0f, width);
        frame_height_ = std::max(1.0f, height);
        input_ = input;
        commands_.clear();
        text_arena_.clear();
        flush_row();
        scope_stack_.clear();

        content_x_ = 16.0f;
        content_width_ = frame_width_ - 32.0f;
        cursor_y_ = 16.0f;
        panel_id_seed_ = 1469598103934665603ull;

        if (input_.mouse_pressed && active_input_id_ != 0) {
            active_input_id_ = 0;
            input_buffer_.clear();
        }
    }

    const std::vector<DrawCommand>& end_frame() {
        flush_row();
        scope_stack_.clear();
        active_slider_id_ = input_.mouse_down ? active_slider_id_ : 0;
        return commands_;
    }

    void begin_panel(std::string_view title, float x, float y, float width, float padding = 14.0f,
                     float radius = 10.0f) {
        flush_row();
        scope_stack_.clear();

        const float panel_w = std::max(140.0f, width);
        const float panel_h = std::max(120.0f, frame_height_ - y - 20.0f);
        panel_rect_ = Rect{x, y, panel_w, panel_h};

        if (radius >= 0.0f) {
            add_filled_rect(panel_rect_, theme_.panel, radius);
            add_outline_rect(panel_rect_, theme_.panel_border, radius);
        }

        content_x_ = x + padding;
        content_width_ = std::max(20.0f, panel_w - 2.0f * padding);
        cursor_y_ = y + padding;

        panel_id_seed_ = hash_sv(title);
        if (!title.empty()) {
            add_text(title, Rect{content_x_, cursor_y_, content_width_, 16.0f}, theme_.text, 14.0f,
                     TextAlign::Left);
            cursor_y_ += 22.0f;
        }
    }

    void end_panel() {
        flush_row();
        scope_stack_.clear();
    }

    void begin_card(std::string_view title, float height = 0.0f, float padding = 14.0f,
                    float radius = 8.0f) {
        flush_row();
        padding = std::clamp(padding, 6.0f, 28.0f);
        const float min_height = std::max(42.0f, height);

        const Rect card{content_x_, cursor_y_, content_width_, min_height};
        const std::size_t fill_cmd_index = commands_.size();
        add_filled_rect(card, theme_.panel, radius);
        const std::size_t outline_cmd_index = commands_.size();
        add_outline_rect(card, theme_.panel_border, radius);

        scope_stack_.push_back(ScopeState{
            ScopeKind::Card,
            content_x_,
            content_width_,
            0.0f,
            fill_cmd_index,
            outline_cmd_index,
            card.y,
            min_height,
            padding,
        });

        content_x_ = card.x + padding;
        content_width_ = std::max(10.0f, card.w - 2.0f * padding);
        cursor_y_ = card.y + padding;

        if (!title.empty()) {
            add_text(title, Rect{content_x_, cursor_y_, content_width_, 16.0f}, theme_.text, 14.0f,
                     TextAlign::Left);
            cursor_y_ += 22.0f;
        }
    }

    void end_card() {
        flush_row();
        if (scope_stack_.empty()) {
            return;
        }
        restore_scope();
    }

    void label(std::string_view text, float font_size = 13.0f, bool muted = false) {
        const float height = font_size + 6.0f;
        const Rect rect = next_rect(height);
        add_text(text, Rect{rect.x, rect.y, rect.w, height}, muted ? theme_.muted_text : theme_.text,
                 font_size, TextAlign::Left);
    }

    void spacer(float height = 8.0f) {
        flush_row();
        cursor_y_ += std::max(0.0f, height);
    }

    bool button(std::string_view label, ButtonStyle style = ButtonStyle::Secondary, float height = 34.0f) {
        const Rect rect = next_rect(height);
        const bool hovered = rect.contains(input_.mouse_x, input_.mouse_y);
        const bool held = hovered && input_.mouse_down;

        Color fill = theme_.secondary;
        Color text_color = theme_.text;
        if (style == ButtonStyle::Primary) {
            fill = theme_.primary;
            text_color = theme_.primary_text;
        } else if (style == ButtonStyle::Ghost) {
            fill = hovered ? mix(theme_.secondary, theme_.panel, 0.5f) : theme_.panel;
            text_color = theme_.text;
        }

        if (held) {
            fill = mix(fill, theme_.secondary_active, 0.35f);
        } else if (hovered && style != ButtonStyle::Ghost) {
            fill = mix(fill, theme_.secondary_hover, 0.30f);
        }

        add_filled_rect(rect, fill, theme_.radius);
        add_outline_rect(rect, style == ButtonStyle::Primary ? theme_.primary : theme_.outline, theme_.radius);
        add_text(label, Rect{rect.x, rect.y + (rect.h - 13.0f) * 0.5f, rect.w, 13.0f}, text_color, 13.0f,
                 TextAlign::Center);

        return hovered && input_.mouse_pressed;
    }

    bool tab(std::string_view label, bool selected, float height = 30.0f) {
        const Rect rect = next_rect(height);
        const bool hovered = rect.contains(input_.mouse_x, input_.mouse_y);
        const bool held = hovered && input_.mouse_down;

        Color fill = selected ? theme_.panel : theme_.secondary;
        if (!selected && hovered) {
            fill = theme_.secondary_hover;
        }
        if (selected && held) {
            fill = mix(theme_.panel, theme_.secondary_active, 0.25f);
        }

        add_filled_rect(rect, fill, theme_.radius - 2.0f);
        add_outline_rect(rect, selected ? theme_.outline : mix(theme_.outline, theme_.panel, 0.6f),
                         theme_.radius - 2.0f);
        add_text(label, Rect{rect.x, rect.y + (rect.h - 12.0f) * 0.5f, rect.w, 12.0f},
                 selected ? theme_.text : theme_.muted_text, 12.0f, TextAlign::Center);

        return hovered && input_.mouse_pressed;
    }

    bool slider_float(std::string_view label, float& value, float min_value, float max_value, int decimals = -1,
                      float height = 40.0f) {
        if (max_value < min_value) {
            std::swap(max_value, min_value);
        }

        const int value_decimals = resolve_decimals(min_value, max_value, decimals);
        const std::uint64_t id = id_for(label) ^ 0x62e2ac4d9d45f7b1ull;
        const Rect rect = next_rect(height);
        const float radius = theme_.radius;
        const Rect value_box{
            rect.x + rect.w - 76.0f,
            rect.y + 6.0f,
            64.0f,
            rect.h - 12.0f,
        };

        const bool hovered = rect.contains(input_.mouse_x, input_.mouse_y);
        const bool value_hovered = value_box.contains(input_.mouse_x, input_.mouse_y);

        if (input_.mouse_right_pressed && value_hovered) {
            start_text_input(id, format_float(value, value_decimals));
        }

        bool changed = false;
        if (is_text_input_active(id)) {
            consume_numeric_typing(min_value < 0.0f, true);
            clamp_live_numeric_buffer(min_value, max_value, value_decimals);
            if (input_.key_escape) {
                active_input_id_ = 0;
                input_buffer_.clear();
            } else if (input_.key_enter || (input_.mouse_pressed && !value_hovered)) {
                changed = commit_text_input(value, min_value, max_value) || changed;
            }
        }

        if (hovered && input_.mouse_pressed && !value_hovered && !is_text_input_active(id)) {
            active_slider_id_ = id;
        }

        if (active_slider_id_ == id) {
            if (input_.mouse_down) {
                float t = (input_.mouse_x - rect.x) / rect.w;
                t = std::clamp(t, 0.0f, 1.0f);
                const float new_value = min_value + (max_value - min_value) * t;
                if (std::fabs(new_value - value) > 1e-6f) {
                    value = new_value;
                    changed = true;
                }
            }
            if (!input_.mouse_down || input_.mouse_released) {
                active_slider_id_ = 0;
            }
        }

        const float range = std::max(1e-6f, max_value - min_value);
        const float t = std::clamp((value - min_value) / range, 0.0f, 1.0f);
        const float fill_w = rect.w * t;
        const Rect fill{
            rect.x + 1.0f,
            rect.y + 1.0f,
            std::max(0.0f, fill_w - 2.0f),
            std::max(0.0f, rect.h - 2.0f),
        };
        const Rect thumb{
            rect.x + fill_w - 1.0f,
            rect.y + 1.0f,
            2.0f,
            rect.h - 2.0f,
        };

        add_filled_rect(rect, theme_.secondary, radius);
        if (fill.w > 0.0f && fill.h > 0.0f) {
            const float fill_radius = std::max(
                0.0f, std::min(std::min(radius - 1.0f, fill.h * 0.5f), fill.w * 0.5f));
            add_filled_rect(fill, mix(theme_.primary, theme_.secondary, 0.75f), fill_radius);
        }
        add_outline_rect(rect, hovered ? theme_.primary : theme_.outline, radius);
        add_filled_rect(thumb, theme_.primary, 1.0f);

        add_text(label, Rect{rect.x + 12.0f, rect.y + (rect.h - 13.0f) * 0.5f, rect.w - value_box.w - 26.0f, 13.0f},
                 theme_.text, 13.0f, TextAlign::Left);

        const bool editing = is_text_input_active(id);
        add_filled_rect(value_box, editing ? theme_.input_bg : mix(theme_.input_bg, theme_.secondary, 0.25f),
                        theme_.radius - 2.0f);
        add_outline_rect(value_box, editing ? theme_.focus_ring : theme_.input_border, theme_.radius - 2.0f,
                         editing ? 1.5f : 1.0f);

        std::string value_text = editing ? input_buffer_ : format_float(value, value_decimals);
        if (editing && should_show_caret()) {
            value_text.push_back('|');
        }
        if (value_text.empty()) {
            value_text = "0";
        }
        add_text(value_text,
                 Rect{value_box.x + 6.0f, value_box.y + (value_box.h - 12.0f) * 0.5f, value_box.w - 10.0f, 12.0f},
                 editing ? theme_.text : theme_.muted_text, 12.0f, TextAlign::Right);

        return changed;
    }

    bool input_float(std::string_view label, float& value, float min_value, float max_value, int decimals = 2,
                     float height = 34.0f) {
        if (max_value < min_value) {
            std::swap(max_value, min_value);
        }

        const Rect rect = next_rect(height);
        const Rect label_rect{
            rect.x,
            rect.y + (rect.h - 13.0f) * 0.5f,
            rect.w * 0.46f,
            13.0f,
        };
        const Rect input_rect{
            rect.x + rect.w * 0.50f,
            rect.y + 3.0f,
            rect.w * 0.50f,
            rect.h - 6.0f,
        };
        const std::uint64_t id = id_for(label) ^ 0x95b6a4cb4123be3full;

        add_text(label, label_rect, theme_.text, 13.0f, TextAlign::Left);

        const bool hovered = input_rect.contains(input_.mouse_x, input_.mouse_y);
        if (hovered && input_.mouse_pressed) {
            start_text_input(id, format_float(value, std::max(0, decimals)));
        }

        bool changed = false;
        if (is_text_input_active(id)) {
            consume_numeric_typing(min_value < 0.0f, true);
            clamp_live_numeric_buffer(min_value, max_value, std::max(0, decimals));
            if (input_.key_escape) {
                active_input_id_ = 0;
                input_buffer_.clear();
            } else if (input_.key_enter || (input_.mouse_pressed && !hovered)) {
                changed = commit_text_input(value, min_value, max_value) || changed;
            }
        }

        const bool editing = is_text_input_active(id);
        add_filled_rect(input_rect, theme_.input_bg, theme_.radius - 2.0f);
        add_outline_rect(input_rect, editing ? theme_.focus_ring : theme_.input_border, theme_.radius - 2.0f,
                         editing ? 1.5f : 1.0f);

        std::string value_text = editing ? input_buffer_ : format_float(value, std::max(0, decimals));
        if (editing && should_show_caret()) {
            value_text.push_back('|');
        }
        if (value_text.empty()) {
            value_text = "0";
        }

        add_text(value_text,
                 Rect{input_rect.x + 8.0f, input_rect.y + (input_rect.h - 12.0f) * 0.5f, input_rect.w - 12.0f,
                      12.0f},
                 theme_.text, 12.0f, TextAlign::Right);

        return changed;
    }

    void progress(std::string_view label, float ratio, float height = 10.0f) {
        ratio = std::clamp(ratio, 0.0f, 1.0f);
        const Rect rect = next_rect(height + 18.0f);

        add_text(label, Rect{rect.x, rect.y, rect.w * 0.7f, 14.0f}, theme_.muted_text, 12.0f,
                 TextAlign::Left);

        char pct_text[32];
        std::snprintf(pct_text, sizeof(pct_text), "%.0f%%", ratio * 100.0f);
        add_text(pct_text, Rect{rect.x + rect.w * 0.7f, rect.y, rect.w * 0.3f, 14.0f}, theme_.text, 12.0f,
                 TextAlign::Right);

        const Rect track{rect.x, rect.y + 18.0f, rect.w, std::max(4.0f, height)};
        const Rect fill{
            track.x + 1.0f,
            track.y + 1.0f,
            std::max(0.0f, track.w * ratio - 2.0f),
            std::max(0.0f, track.h - 2.0f),
        };
        const float track_radius = track.h * 0.5f;
        add_filled_rect(track, theme_.track, track_radius);
        if (fill.w > 0.0f && fill.h > 0.0f) {
            const float fill_radius = std::max(
                0.0f, std::min(std::min(track_radius - 1.0f, fill.h * 0.5f), fill.w * 0.5f));
            add_filled_rect(fill, theme_.track_fill, fill_radius);
        }
    }

    bool begin_dropdown(std::string_view label, bool& open, float body_height = 104.0f,
                        float padding = 10.0f) {
        const Rect header = next_rect(34.0f);
        const bool hovered = header.contains(input_.mouse_x, input_.mouse_y);
        if (hovered && input_.mouse_pressed) {
            open = !open;
        }

        Color fill = theme_.panel;
        if (hovered) {
            fill = mix(theme_.panel, theme_.secondary_hover, 0.35f);
        }

        add_filled_rect(header, fill, theme_.radius);
        add_outline_rect(header, theme_.outline, theme_.radius);
        add_text(label, Rect{header.x + 10.0f, header.y + 9.0f, header.w - 30.0f, 13.0f}, theme_.text, 13.0f,
                 TextAlign::Left);
        add_text(open ? "V" : ">", Rect{header.x + 10.0f, header.y + 9.0f, header.w - 20.0f, 13.0f},
                 theme_.muted_text, 13.0f, TextAlign::Right);

        if (!open) {
            return false;
        }

        flush_row();

        body_height = std::max(36.0f, body_height);
        padding = std::clamp(padding, 4.0f, 24.0f);

        const Rect body{content_x_, cursor_y_, content_width_, body_height};
        const std::size_t fill_cmd_index = commands_.size();
        add_filled_rect(body, mix(theme_.panel, theme_.secondary, 0.35f), theme_.radius);
        const std::size_t outline_cmd_index = commands_.size();
        add_outline_rect(body, theme_.outline, theme_.radius);

        scope_stack_.push_back(ScopeState{
            ScopeKind::DropdownBody,
            content_x_,
            content_width_,
            0.0f,
            fill_cmd_index,
            outline_cmd_index,
            body.y,
            body_height,
            padding,
        });
        content_x_ = body.x + padding;
        content_width_ = std::max(10.0f, body.w - 2.0f * padding);
        cursor_y_ = body.y + padding;

        return true;
    }

    void end_dropdown() {
        flush_row();
        if (scope_stack_.empty()) {
            return;
        }
        restore_scope();
    }

    void begin_row(int columns, float gap = 8.0f) {
        flush_row();
        row_.active = true;
        row_.columns = std::max(1, columns);
        row_.index = 0;
        row_.gap = std::max(0.0f, gap);
        row_.y = cursor_y_;
        row_.max_height = 0.0f;
    }

    void end_row() {
        flush_row();
    }

    const std::vector<DrawCommand>& commands() const {
        return commands_;
    }

    const std::vector<char>& text_arena() const {
        return text_arena_;
    }

private:
    void refresh_theme() {
        theme_ = make_theme(theme_mode_, primary_color_);
        theme_.radius = corner_radius_;
    }

    enum class ScopeKind {
        Card,
        DropdownBody,
    };

    struct ScopeState {
        ScopeKind kind{ScopeKind::Card};
        float content_x{0.0f};
        float content_width{0.0f};
        float cursor_y_after{0.0f};
        std::size_t fill_cmd_index{0};
        std::size_t outline_cmd_index{0};
        float top_y{0.0f};
        float min_height{0.0f};
        float padding{0.0f};
    };

    struct RowState {
        bool active{false};
        int columns{1};
        int index{0};
        float gap{8.0f};
        float y{0.0f};
        float max_height{0.0f};
    };

    static std::uint64_t hash_sv(std::string_view text) {
        std::uint64_t value = 1469598103934665603ull;
        for (char ch : text) {
            value ^= static_cast<std::uint8_t>(ch);
            value *= 1099511628211ull;
        }
        return value;
    }

    std::uint64_t id_for(std::string_view label) const {
        const std::uint64_t key = hash_sv(label);
        return panel_id_seed_ ^ (key + 0x9e3779b97f4a7c15ull + (panel_id_seed_ << 6u) +
                                 (panel_id_seed_ >> 2u));
    }

    static bool parse_float_text(const std::string& text, float& out_value) {
        if (text.empty() || text == "-" || text == "." || text == "-.") {
            return false;
        }
        char* end = nullptr;
        const float parsed = std::strtof(text.c_str(), &end);
        if (end == text.c_str() || (end != nullptr && *end != '\0') || !std::isfinite(parsed)) {
            return false;
        }
        out_value = parsed;
        return true;
    }

    static int resolve_decimals(float min_value, float max_value, int requested) {
        if (requested >= 0) {
            return std::clamp(requested, 0, 4);
        }
        const float span = std::fabs(max_value - min_value);
        if (span <= 1.0f) {
            return 2;
        }
        if (span <= 10.0f) {
            return 1;
        }
        return 0;
    }

    static std::string format_float(float value, int decimals) {
        decimals = std::clamp(decimals, 0, 4);
        char buffer[64];
        std::snprintf(buffer, sizeof(buffer), "%.*f", decimals, value);
        return std::string(buffer);
    }

    void start_text_input(std::uint64_t id, const std::string& initial_value) {
        if (active_input_id_ == id) {
            return;
        }
        active_input_id_ = id;
        input_buffer_ = initial_value;
    }

    bool is_text_input_active(std::uint64_t id) const {
        return active_input_id_ == id;
    }

    void consume_numeric_typing(bool allow_negative, bool allow_decimal) {
        for (char ch : input_.text_input) {
            if (ch >= '0' && ch <= '9') {
                input_buffer_.push_back(ch);
            } else if (allow_decimal && ch == '.') {
                if (input_buffer_.find('.') == std::string::npos) {
                    if (input_buffer_.empty()) {
                        input_buffer_.push_back('0');
                    } else if (input_buffer_ == "-") {
                        input_buffer_ += "0";
                    }
                    input_buffer_.push_back('.');
                }
            } else if (allow_negative && ch == '-' && input_buffer_.empty()) {
                input_buffer_.push_back('-');
            }
        }

        if (input_.key_backspace && !input_buffer_.empty()) {
            input_buffer_.pop_back();
        }
    }

    bool commit_text_input(float& value, float min_value, float max_value) {
        float parsed = value;
        const bool valid = parse_float_text(input_buffer_, parsed);
        if (valid) {
            parsed = std::clamp(parsed, min_value, max_value);
        }
        active_input_id_ = 0;
        input_buffer_.clear();
        if (!valid) {
            return false;
        }
        if (std::fabs(parsed - value) <= 1e-6f) {
            return false;
        }
        value = parsed;
        return true;
    }

    void clamp_live_numeric_buffer(float min_value, float max_value, int decimals) {
        if (input_buffer_.empty() || input_buffer_ == "-" || input_buffer_ == "." ||
            input_buffer_ == "-.") {
            return;
        }

        if (input_buffer_.size() > 24u) {
            input_buffer_.resize(24u);
        }

        decimals = std::clamp(decimals, 0, 6);
        const std::size_t dot_pos = input_buffer_.find('.');
        if (dot_pos != std::string::npos && decimals >= 0) {
            const std::size_t max_len = dot_pos + 1u + static_cast<std::size_t>(decimals);
            if (input_buffer_.size() > max_len) {
                input_buffer_.resize(max_len);
            }
        }

        float parsed = 0.0f;
        if (!parse_float_text(input_buffer_, parsed)) {
            return;
        }

        const float clamped = std::clamp(parsed, min_value, max_value);
        if (std::fabs(clamped - parsed) > 1e-6f) {
            input_buffer_ = format_float(clamped, decimals);
        }
    }

    bool should_show_caret() const {
        // 1.2s full cycle, 0.6s on / 0.6s off.
        return std::fmod(input_.time_seconds, 1.2) < 0.6;
    }

    void restore_scope() {
        const ScopeState scope = scope_stack_.back();
        scope_stack_.pop_back();

        if (scope.kind == ScopeKind::Card || scope.kind == ScopeKind::DropdownBody) {
            const float needed_height = std::max(0.0f, (cursor_y_ - scope.top_y) + scope.padding);
            const float final_height = std::max(scope.min_height, needed_height);
            if (scope.fill_cmd_index < commands_.size()) {
                commands_[scope.fill_cmd_index].rect.h = final_height;
            }
            if (scope.outline_cmd_index < commands_.size()) {
                commands_[scope.outline_cmd_index].rect.h = final_height;
            }

            content_x_ = scope.content_x;
            content_width_ = scope.content_width;
            cursor_y_ = scope.top_y + final_height + item_spacing_;
            return;
        }

        content_x_ = scope.content_x;
        content_width_ = scope.content_width;
        cursor_y_ = scope.cursor_y_after;
    }

    void flush_row() {
        if (!row_.active) {
            return;
        }
        cursor_y_ = row_.y + row_.max_height + item_spacing_;
        row_ = RowState{};
    }

    Rect next_rect(float height) {
        height = std::max(2.0f, height);

        if (!row_.active) {
            Rect rect{content_x_, cursor_y_, content_width_, height};
            cursor_y_ += height + item_spacing_;
            return rect;
        }

        const float total_gap = row_.gap * static_cast<float>(row_.columns - 1);
        const float item_width =
            std::max(10.0f, (content_width_ - total_gap) / static_cast<float>(row_.columns));
        Rect rect{
            content_x_ + static_cast<float>(row_.index) * (item_width + row_.gap),
            row_.y,
            item_width,
            height,
        };

        row_.max_height = std::max(row_.max_height, height);
        row_.index += 1;
        if (row_.index >= row_.columns) {
            flush_row();
        }

        return rect;
    }

    void add_filled_rect(const Rect& rect, const Color& color, float radius = 0.0f) {
        DrawCommand cmd;
        cmd.type = CommandType::FilledRect;
        cmd.rect = rect;
        cmd.color = color;
        cmd.radius = std::max(0.0f, radius);
        commands_.push_back(std::move(cmd));
    }

    void add_outline_rect(const Rect& rect, const Color& color, float radius = 0.0f,
                          float thickness = 1.0f) {
        DrawCommand cmd;
        cmd.type = CommandType::RectOutline;
        cmd.rect = rect;
        cmd.color = color;
        cmd.radius = std::max(0.0f, radius);
        cmd.thickness = std::max(1.0f, thickness);
        commands_.push_back(std::move(cmd));
    }

    void add_text(std::string_view text, const Rect& rect, const Color& color, float font_size,
                  TextAlign align) {
        DrawCommand cmd;
        cmd.type = CommandType::Text;
        cmd.rect = rect;
        cmd.color = color;
        cmd.font_size = std::max(8.0f, font_size);
        cmd.align = align;

        const std::uint32_t offset = static_cast<std::uint32_t>(text_arena_.size());
        const std::uint32_t length = static_cast<std::uint32_t>(text.size());
        text_arena_.insert(text_arena_.end(), text.begin(), text.end());
        text_arena_.push_back('\0');
        cmd.text_offset = offset;
        cmd.text_length = length;
        commands_.push_back(std::move(cmd));
    }

    float frame_width_{1280.0f};
    float frame_height_{720.0f};
    InputState input_{};

    ThemeMode theme_mode_{ThemeMode::Dark};
    Color primary_color_{0.25f, 0.55f, 1.0f, 1.0f};
    float corner_radius_{8.0f};
    Theme theme_{make_theme(theme_mode_, primary_color_)};

    std::vector<DrawCommand> commands_{};
    std::vector<ScopeState> scope_stack_{};
    std::vector<char> text_arena_{};

    RowState row_{};
    float content_x_{16.0f};
    float content_width_{800.0f};
    float cursor_y_{16.0f};
    float item_spacing_{10.0f};

    std::uint64_t panel_id_seed_{1469598103934665603ull};
    std::uint64_t active_slider_id_{0};
    std::uint64_t active_input_id_{0};
    std::string input_buffer_{};
    Rect panel_rect_{};
};

#ifdef EUI_ENABLE_GLFW_OPENGL_BACKEND

namespace demo {

struct AppOptions {
    int width{1150};
    int height{820};
    const char* title{"EUI Demo"};
    bool vsync{true};
    bool continuous_render{false};
    double idle_wait_seconds{0.25};
    double max_fps{60.0};
};

struct FrameContext {
    Context& ui;
    GLFWwindow* window{nullptr};
    float dt{0.0f};
    int framebuffer_w{1};
    int framebuffer_h{1};
    bool* repaint_flag{nullptr};

    void request_next_frame() const {
        if (repaint_flag != nullptr) {
            *repaint_flag = true;
        }
    }
};

namespace detail {

using Glyph = std::array<std::uint8_t, 7>;
using Point = std::array<float, 2>;
using TextureId = unsigned int;

struct RuntimeState {
    std::string text_input{};
    bool prev_left_mouse{false};
    bool prev_right_mouse{false};
    bool prev_backspace{false};
    bool prev_enter{false};
    bool prev_escape{false};
    double prev_mouse_x{0.0};
    double prev_mouse_y{0.0};
    bool has_prev_mouse{false};
    int prev_framebuffer_w{0};
    int prev_framebuffer_h{0};

    std::vector<DrawCommand> prev_commands{};
    std::vector<char> prev_text_arena{};
    Color prev_bg{};
    bool has_prev_frame{false};

    TextureId cache_texture{0};
    int cache_w{0};
    int cache_h{0};
    bool has_cache{false};
};

inline void text_input_callback(GLFWwindow* window, unsigned int codepoint) {
    RuntimeState* state = static_cast<RuntimeState*>(glfwGetWindowUserPointer(window));
    if (state == nullptr) {
        return;
    }
    if (codepoint >= 32u && codepoint <= 126u) {
        state->text_input.push_back(static_cast<char>(codepoint));
    }
}

inline bool float_eq(float lhs, float rhs, float eps = 1e-5f) {
    return std::fabs(lhs - rhs) <= eps;
}

inline bool color_eq(const Color& lhs, const Color& rhs, float eps = 1e-4f) {
    return float_eq(lhs.r, rhs.r, eps) && float_eq(lhs.g, rhs.g, eps) &&
           float_eq(lhs.b, rhs.b, eps) && float_eq(lhs.a, rhs.a, eps);
}

inline bool rect_eq(const Rect& lhs, const Rect& rhs, float eps = 0.01f) {
    return float_eq(lhs.x, rhs.x, eps) && float_eq(lhs.y, rhs.y, eps) &&
           float_eq(lhs.w, rhs.w, eps) && float_eq(lhs.h, rhs.h, eps);
}

inline bool rect_intersects(const Rect& lhs, const Rect& rhs) {
    if (lhs.w <= 0.0f || lhs.h <= 0.0f || rhs.w <= 0.0f || rhs.h <= 0.0f) {
        return false;
    }
    return lhs.x < rhs.x + rhs.w && lhs.x + lhs.w > rhs.x && lhs.y < rhs.y + rhs.h &&
           lhs.y + lhs.h > rhs.y;
}

inline Rect rect_union(const Rect& lhs, const Rect& rhs) {
    const float x1 = std::min(lhs.x, rhs.x);
    const float y1 = std::min(lhs.y, rhs.y);
    const float x2 = std::max(lhs.x + lhs.w, rhs.x + rhs.w);
    const float y2 = std::max(lhs.y + lhs.h, rhs.y + rhs.h);
    return Rect{x1, y1, std::max(0.0f, x2 - x1), std::max(0.0f, y2 - y1)};
}

inline bool command_payload_equal(const DrawCommand& lhs, const DrawCommand& rhs,
                                  const std::vector<char>& lhs_arena,
                                  const std::vector<char>& rhs_arena) {
    if (lhs.type != rhs.type || lhs.align != rhs.align || !rect_eq(lhs.rect, rhs.rect) ||
        !color_eq(lhs.color, rhs.color) || !float_eq(lhs.font_size, rhs.font_size) ||
        !float_eq(lhs.radius, rhs.radius) || !float_eq(lhs.thickness, rhs.thickness)) {
        return false;
    }

    if (lhs.type != CommandType::Text) {
        return true;
    }

    if (lhs.text_length != rhs.text_length) {
        return false;
    }
    const std::size_t l_offset = static_cast<std::size_t>(lhs.text_offset);
    const std::size_t r_offset = static_cast<std::size_t>(rhs.text_offset);
    const std::size_t length = static_cast<std::size_t>(lhs.text_length);
    if (l_offset + length > lhs_arena.size() || r_offset + length > rhs_arena.size()) {
        return false;
    }
    for (std::size_t i = 0; i < length; ++i) {
        if (lhs_arena[l_offset + i] != rhs_arena[r_offset + i]) {
            return false;
        }
    }
    return true;
}

inline Rect expanded_and_clamped(const Rect& rect, int width, int height, float expand_px = 2.0f) {
    Rect out{
        rect.x - expand_px,
        rect.y - expand_px,
        rect.w + expand_px * 2.0f,
        rect.h + expand_px * 2.0f,
    };
    out.x = std::clamp(out.x, 0.0f, static_cast<float>(width));
    out.y = std::clamp(out.y, 0.0f, static_cast<float>(height));
    const float right = std::clamp(out.x + out.w, 0.0f, static_cast<float>(width));
    const float bottom = std::clamp(out.y + out.h, 0.0f, static_cast<float>(height));
    out.w = std::max(0.0f, right - out.x);
    out.h = std::max(0.0f, bottom - out.y);
    return out;
}

inline bool compute_dirty_region(const std::vector<DrawCommand>& commands,
                                 const std::vector<char>& text_arena, const RuntimeState& runtime,
                                 const Color& bg, int width, int height, bool force_full, Rect& out_dirty) {
    if (force_full || !runtime.has_prev_frame || !color_eq(bg, runtime.prev_bg)) {
        out_dirty = Rect{0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)};
        return true;
    }

    bool has_dirty = false;
    Rect dirty{};
    const std::size_t max_count = std::max(commands.size(), runtime.prev_commands.size());
    for (std::size_t i = 0; i < max_count; ++i) {
        const bool has_curr = i < commands.size();
        const bool has_prev = i < runtime.prev_commands.size();

        if (has_curr && has_prev &&
            command_payload_equal(commands[i], runtime.prev_commands[i], text_arena, runtime.prev_text_arena)) {
            continue;
        }

        if (has_curr) {
            dirty = has_dirty ? rect_union(dirty, commands[i].rect) : commands[i].rect;
            has_dirty = true;
        }
        if (has_prev) {
            dirty = has_dirty ? rect_union(dirty, runtime.prev_commands[i].rect)
                              : runtime.prev_commands[i].rect;
            has_dirty = true;
        }
    }

    if (!has_dirty) {
        return false;
    }

    out_dirty = expanded_and_clamped(dirty, width, height);
    return out_dirty.w > 0.0f && out_dirty.h > 0.0f;
}

struct IRect {
    int x{0};
    int y{0};
    int w{0};
    int h{0};
};

inline IRect to_gl_rect(const Rect& rect, int framebuffer_w, int framebuffer_h) {
    int x = static_cast<int>(std::floor(rect.x));
    int y_top = static_cast<int>(std::floor(rect.y));
    int w = static_cast<int>(std::ceil(rect.w));
    int h = static_cast<int>(std::ceil(rect.h));

    x = std::clamp(x, 0, framebuffer_w);
    y_top = std::clamp(y_top, 0, framebuffer_h);
    w = std::clamp(w, 0, framebuffer_w - x);
    h = std::clamp(h, 0, framebuffer_h - y_top);
    int y = framebuffer_h - (y_top + h);
    y = std::clamp(y, 0, framebuffer_h);
    return IRect{x, y, w, h};
}

inline void ensure_cache_texture(RuntimeState& runtime, int width, int height) {
    if (runtime.cache_texture == 0u) {
        glGenTextures(1, &runtime.cache_texture);
    }
    if (runtime.cache_w == width && runtime.cache_h == height && runtime.has_cache) {
        return;
    }
    runtime.cache_w = width;
    runtime.cache_h = height;
    runtime.has_cache = false;

    glBindTexture(GL_TEXTURE_2D, runtime.cache_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
}

inline void copy_full_to_cache(RuntimeState& runtime, int width, int height) {
    if (runtime.cache_texture == 0u) {
        return;
    }
    glBindTexture(GL_TEXTURE_2D, runtime.cache_texture);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);
    runtime.has_cache = true;
}

inline void copy_region_to_cache(RuntimeState& runtime, const IRect& gl_rect) {
    if (runtime.cache_texture == 0u || gl_rect.w <= 0 || gl_rect.h <= 0) {
        return;
    }
    glBindTexture(GL_TEXTURE_2D, runtime.cache_texture);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, gl_rect.x, gl_rect.y, gl_rect.x, gl_rect.y, gl_rect.w, gl_rect.h);
    runtime.has_cache = true;
}

inline void draw_cache_texture(const RuntimeState& runtime, int width, int height) {
    if (!runtime.has_cache || runtime.cache_texture == 0u) {
        return;
    }
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, static_cast<double>(width), static_cast<double>(height), 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, runtime.cache_texture);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(static_cast<float>(width), 0.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(static_cast<float>(width), static_cast<float>(height));
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(0.0f, static_cast<float>(height));
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

inline const Glyph& glyph_for(char ch) {
    static const Glyph kUnknown = {0x1E, 0x11, 0x02, 0x04, 0x04, 0x00, 0x04};
    static const Glyph kSpace = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    static const Glyph kDot = {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C};
    static const Glyph kMinus = {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00};
    static const Glyph kGreater = {0x10, 0x08, 0x04, 0x02, 0x04, 0x08, 0x10};
    static const Glyph kPipe = {0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04};
    static const Glyph kPercent = {0x19, 0x19, 0x02, 0x04, 0x08, 0x13, 0x13};
    static const Glyph kSlash = {0x01, 0x02, 0x04, 0x08, 0x10, 0x00, 0x00};

    static const Glyph k0 = {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E};
    static const Glyph k1 = {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E};
    static const Glyph k2 = {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F};
    static const Glyph k3 = {0x1E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x1E};
    static const Glyph k4 = {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02};
    static const Glyph k5 = {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E};
    static const Glyph k6 = {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E};
    static const Glyph k7 = {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08};
    static const Glyph k8 = {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E};
    static const Glyph k9 = {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C};

    static const Glyph kA = {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
    static const Glyph kB = {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E};
    static const Glyph kC = {0x0F, 0x10, 0x10, 0x10, 0x10, 0x10, 0x0F};
    static const Glyph kD = {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E};
    static const Glyph kE = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F};
    static const Glyph kF = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10};
    static const Glyph kG = {0x0F, 0x10, 0x10, 0x17, 0x11, 0x11, 0x0F};
    static const Glyph kH = {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
    static const Glyph kI = {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E};
    static const Glyph kJ = {0x07, 0x02, 0x02, 0x02, 0x12, 0x12, 0x0C};
    static const Glyph kK = {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11};
    static const Glyph kL = {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F};
    static const Glyph kM = {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11};
    static const Glyph kN = {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11};
    static const Glyph kO = {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
    static const Glyph kP = {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10};
    static const Glyph kQ = {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D};
    static const Glyph kR = {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11};
    static const Glyph kS = {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E};
    static const Glyph kT = {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04};
    static const Glyph kU = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
    static const Glyph kV = {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04};
    static const Glyph kW = {0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A};
    static const Glyph kX = {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11};
    static const Glyph kY = {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04};
    static const Glyph kZ = {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F};

    switch (ch) {
        case ' ':
            return kSpace;
        case '.':
            return kDot;
        case '-':
            return kMinus;
        case '>':
            return kGreater;
        case '|':
            return kPipe;
        case '%':
            return kPercent;
        case '/':
            return kSlash;
        case '0':
            return k0;
        case '1':
            return k1;
        case '2':
            return k2;
        case '3':
            return k3;
        case '4':
            return k4;
        case '5':
            return k5;
        case '6':
            return k6;
        case '7':
            return k7;
        case '8':
            return k8;
        case '9':
            return k9;
        case 'A':
            return kA;
        case 'B':
            return kB;
        case 'C':
            return kC;
        case 'D':
            return kD;
        case 'E':
            return kE;
        case 'F':
            return kF;
        case 'G':
            return kG;
        case 'H':
            return kH;
        case 'I':
            return kI;
        case 'J':
            return kJ;
        case 'K':
            return kK;
        case 'L':
            return kL;
        case 'M':
            return kM;
        case 'N':
            return kN;
        case 'O':
            return kO;
        case 'P':
            return kP;
        case 'Q':
            return kQ;
        case 'R':
            return kR;
        case 'S':
            return kS;
        case 'T':
            return kT;
        case 'U':
            return kU;
        case 'V':
            return kV;
        case 'W':
            return kW;
        case 'X':
            return kX;
        case 'Y':
            return kY;
        case 'Z':
            return kZ;
        default:
            return kUnknown;
    }
}

inline void gl_set_color(const Color& color) {
    glColor4f(color.r, color.g, color.b, color.a);
}

inline int build_rounded_points(const Rect& rect, float radius, Point* out_points, int max_points) {
    if (max_points < 4) {
        return 0;
    }

    radius = std::clamp(radius, 0.0f, std::min(rect.w, rect.h) * 0.5f);
    if (radius <= 0.0f) {
        out_points[0] = Point{rect.x, rect.y};
        out_points[1] = Point{rect.x + rect.w, rect.y};
        out_points[2] = Point{rect.x + rect.w, rect.y + rect.h};
        out_points[3] = Point{rect.x, rect.y + rect.h};
        return 4;
    }

    const float left = rect.x;
    const float right = rect.x + rect.w;
    const float top = rect.y;
    const float bottom = rect.y + rect.h;
    const int steps = std::clamp(static_cast<int>(radius * 0.65f), 3, 10);
    const float kPi = 3.1415926f;

    int count = 0;
    auto push_point = [&](float x, float y) {
        if (count < max_points) {
            out_points[count++] = Point{x, y};
        }
    };
    auto add_arc = [&](float cx, float cy, float start_angle, float end_angle, bool include_first) {
        for (int i = 0; i <= steps; ++i) {
            if (i == 0 && !include_first) {
                continue;
            }
            const float t = static_cast<float>(i) / static_cast<float>(steps);
            const float angle = start_angle + (end_angle - start_angle) * t;
            push_point(cx + std::cos(angle) * radius, cy + std::sin(angle) * radius);
        }
    };

    add_arc(left + radius, top + radius, kPi, 1.5f * kPi, true);
    add_arc(right - radius, top + radius, 1.5f * kPi, 2.0f * kPi, false);
    add_arc(right - radius, bottom - radius, 0.0f, 0.5f * kPi, false);
    add_arc(left + radius, bottom - radius, 0.5f * kPi, kPi, false);
    return count;
}

inline void draw_filled_rect(const Rect& rect, float radius) {
    if (radius <= 0.0f) {
        glBegin(GL_QUADS);
        glVertex2f(rect.x, rect.y);
        glVertex2f(rect.x + rect.w, rect.y);
        glVertex2f(rect.x + rect.w, rect.y + rect.h);
        glVertex2f(rect.x, rect.y + rect.h);
        glEnd();
        return;
    }

    std::array<Point, 64> points{};
    const int count = build_rounded_points(rect, radius, points.data(), static_cast<int>(points.size()));
    if (count < 3) {
        return;
    }

    float center_x = 0.0f;
    float center_y = 0.0f;
    for (int i = 0; i < count; ++i) {
        center_x += points[static_cast<std::size_t>(i)][0];
        center_y += points[static_cast<std::size_t>(i)][1];
    }
    center_x /= static_cast<float>(count);
    center_y /= static_cast<float>(count);

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(center_x, center_y);
    for (int i = 0; i < count; ++i) {
        const Point& p = points[static_cast<std::size_t>(i)];
        glVertex2f(p[0], p[1]);
    }
    glVertex2f(points[0][0], points[0][1]);
    glEnd();
}

inline void draw_outline_rect(const Rect& rect, float radius, float thickness) {
    if (radius <= 0.0f) {
        glLineWidth(std::max(1.0f, thickness));
        glBegin(GL_LINE_LOOP);
        glVertex2f(rect.x, rect.y);
        glVertex2f(rect.x + rect.w, rect.y);
        glVertex2f(rect.x + rect.w, rect.y + rect.h);
        glVertex2f(rect.x, rect.y + rect.h);
        glEnd();
        glLineWidth(1.0f);
        return;
    }

    std::array<Point, 64> points{};
    const int count = build_rounded_points(rect, radius, points.data(), static_cast<int>(points.size()));
    if (count < 3) {
        return;
    }

    glLineWidth(std::max(1.0f, thickness));
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < count; ++i) {
        const Point& p = points[static_cast<std::size_t>(i)];
        glVertex2f(p[0], p[1]);
    }
    glEnd();
    glLineWidth(1.0f);
}

inline float text_width(std::size_t glyph_count, float scale) {
    if (glyph_count == 0u) {
        return 0.0f;
    }
    const float advance = 6.0f * scale;
    return advance * static_cast<float>(glyph_count) - scale;
}

inline void draw_text(const DrawCommand& cmd, std::string_view text) {
    if (text.empty()) {
        return;
    }

    const float scale = std::max(1.0f, cmd.font_size / 8.0f);
    const float advance = 6.0f * scale;
    const float width = text_width(text.size(), scale);

    float x = cmd.rect.x;
    if (cmd.align == TextAlign::Center) {
        x += std::max(0.0f, (cmd.rect.w - width) * 0.5f);
    } else if (cmd.align == TextAlign::Right) {
        x += std::max(0.0f, cmd.rect.w - width);
    }
    const float y = cmd.rect.y;

    gl_set_color(cmd.color);
    glBegin(GL_QUADS);
    float pen_x = x;
    for (char raw_ch : text) {
        const char ch = static_cast<char>(std::toupper(static_cast<unsigned char>(raw_ch)));
        if (ch == ' ') {
            pen_x += advance;
            continue;
        }

        const Glyph& glyph = glyph_for(ch);
        for (std::size_t row = 0; row < glyph.size(); ++row) {
            const std::uint8_t bits = glyph[row];
            for (int col = 0; col < 5; ++col) {
                const std::uint8_t mask = static_cast<std::uint8_t>(1u << (4 - col));
                if ((bits & mask) == 0u) {
                    continue;
                }
                const float px = pen_x + static_cast<float>(col) * scale;
                const float py = y + static_cast<float>(row) * scale;
                glVertex2f(px, py);
                glVertex2f(px + scale, py);
                glVertex2f(px + scale, py + scale);
                glVertex2f(px, py + scale);
            }
        }
        pen_x += advance;
    }
    glEnd();
}

class Renderer {
public:
    void render(const std::vector<DrawCommand>& commands, const std::vector<char>& text_arena, int width,
                int height, const Rect* clip_rect = nullptr) const {
        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.0, static_cast<double>(width), static_cast<double>(height), 0.0, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        for (const DrawCommand& cmd : commands) {
            if (clip_rect != nullptr && !rect_intersects(cmd.rect, *clip_rect)) {
                continue;
            }
            switch (cmd.type) {
                case CommandType::FilledRect:
                    gl_set_color(cmd.color);
                    draw_filled_rect(cmd.rect, cmd.radius);
                    break;
                case CommandType::RectOutline:
                    gl_set_color(cmd.color);
                    draw_outline_rect(cmd.rect, cmd.radius, cmd.thickness);
                    break;
                case CommandType::Text: {
                    std::string_view text{};
                    const std::size_t offset = static_cast<std::size_t>(cmd.text_offset);
                    const std::size_t length = static_cast<std::size_t>(cmd.text_length);
                    if (offset + length <= text_arena.size()) {
                        text = std::string_view(text_arena.data() + offset, length);
                    }
                    draw_text(cmd, text);
                    break;
                }
            }
        }
    }
};

}  // namespace detail

template <typename BuildUiFn>
int run(BuildUiFn&& build_ui, const AppOptions& options = {}) {
    if (glfwInit() == 0) {
        std::cerr << "Failed to initialize GLFW." << std::endl;
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    GLFWwindow* window = glfwCreateWindow(options.width, options.height, options.title, nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window." << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(options.vsync ? 1 : 0);

    detail::RuntimeState runtime{};
    glfwSetWindowUserPointer(window, &runtime);
    glfwSetCharCallback(window, detail::text_input_callback);

    detail::Renderer renderer;
    Context ui;
    double previous_time = glfwGetTime();
    double next_frame_deadline = previous_time;
    bool redraw_needed = true;

    while (glfwWindowShouldClose(window) == 0) {
        if (options.continuous_render || redraw_needed) {
            if (options.max_fps > 0.0) {
                const double now_wait = glfwGetTime();
                const double wait_s = next_frame_deadline - now_wait;
                if (wait_s > 0.0005) {
                    glfwWaitEventsTimeout(wait_s);
                } else {
                    glfwPollEvents();
                }
            } else {
                glfwPollEvents();
            }
        } else {
            glfwWaitEventsTimeout(std::max(0.001, options.idle_wait_seconds));
        }

        int framebuffer_w = 1;
        int framebuffer_h = 1;
        glfwGetFramebufferSize(window, &framebuffer_w, &framebuffer_h);

        int window_w = 1;
        int window_h = 1;
        glfwGetWindowSize(window, &window_w, &window_h);

        double mouse_x = 0.0;
        double mouse_y = 0.0;
        glfwGetCursorPos(window, &mouse_x, &mouse_y);

        const float scale_x = window_w > 0 ? static_cast<float>(framebuffer_w) / static_cast<float>(window_w)
                                           : 1.0f;
        const float scale_y = window_h > 0 ? static_cast<float>(framebuffer_h) / static_cast<float>(window_h)
                                           : 1.0f;

        const bool left_mouse_down = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        const bool right_mouse_down = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

        const bool backspace_down = glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS;
        const bool enter_down = (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) ||
                                (glfwGetKey(window, GLFW_KEY_KP_ENTER) == GLFW_PRESS);
        const bool escape_down = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;

        InputState input{};
        input.mouse_x = static_cast<float>(mouse_x) * scale_x;
        input.mouse_y = static_cast<float>(mouse_y) * scale_y;
        input.mouse_down = left_mouse_down;
        input.mouse_pressed = left_mouse_down && !runtime.prev_left_mouse;
        input.mouse_released = !left_mouse_down && runtime.prev_left_mouse;
        input.mouse_right_down = right_mouse_down;
        input.mouse_right_pressed = right_mouse_down && !runtime.prev_right_mouse;
        input.mouse_right_released = !right_mouse_down && runtime.prev_right_mouse;
        input.key_backspace = backspace_down && !runtime.prev_backspace;
        input.key_enter = enter_down && !runtime.prev_enter;
        input.key_escape = escape_down && !runtime.prev_escape;
        input.text_input = runtime.text_input;

        const bool mouse_moved = !runtime.has_prev_mouse ||
                                 std::fabs(mouse_x - runtime.prev_mouse_x) > 0.5 ||
                                 std::fabs(mouse_y - runtime.prev_mouse_y) > 0.5;
        const bool framebuffer_changed =
            framebuffer_w != runtime.prev_framebuffer_w || framebuffer_h != runtime.prev_framebuffer_h;

        runtime.text_input.clear();
        runtime.prev_left_mouse = left_mouse_down;
        runtime.prev_right_mouse = right_mouse_down;
        runtime.prev_backspace = backspace_down;
        runtime.prev_enter = enter_down;
        runtime.prev_escape = escape_down;
        runtime.prev_mouse_x = mouse_x;
        runtime.prev_mouse_y = mouse_y;
        runtime.has_prev_mouse = true;
        runtime.prev_framebuffer_w = framebuffer_w;
        runtime.prev_framebuffer_h = framebuffer_h;

        const double now = glfwGetTime();
        const float dt = static_cast<float>(now - previous_time);
        previous_time = now;
        input.time_seconds = now;

        const bool input_event = input.mouse_pressed || input.mouse_released || input.mouse_right_pressed ||
                                 input.mouse_right_released || input.key_backspace || input.key_enter ||
                                 input.key_escape || !input.text_input.empty();
        const bool render_this_frame =
            options.continuous_render || redraw_needed || framebuffer_changed || mouse_moved || input_event;
        if (!render_this_frame) {
            continue;
        }

        ui.begin_frame(static_cast<float>(framebuffer_w), static_cast<float>(framebuffer_h), input);
        bool request_next_frame = false;
        FrameContext frame_ctx{ui, window, dt, framebuffer_w, framebuffer_h, &request_next_frame};
        build_ui(frame_ctx);
        const auto& commands = ui.end_frame();
        redraw_needed = request_next_frame;

        const Color bg = ui.theme().background;
        const auto& text_arena = ui.text_arena();
        detail::ensure_cache_texture(runtime, framebuffer_w, framebuffer_h);

        const bool force_full_redraw =
            framebuffer_changed || !runtime.has_prev_frame || !runtime.has_cache;
        Rect dirty{};
        const bool has_dirty =
            detail::compute_dirty_region(commands, text_arena, runtime, bg, framebuffer_w, framebuffer_h,
                                         force_full_redraw, dirty);

        if (!force_full_redraw && !has_dirty) {
            runtime.prev_commands = commands;
            runtime.prev_text_arena = text_arena;
            runtime.prev_bg = bg;
            runtime.has_prev_frame = true;
            if (options.max_fps > 0.0) {
                const double frame_interval = 1.0 / options.max_fps;
                next_frame_deadline = glfwGetTime() + frame_interval;
            }
            continue;
        }

        if (force_full_redraw) {
            glDisable(GL_SCISSOR_TEST);
            glClearColor(bg.r, bg.g, bg.b, bg.a);
            glClear(GL_COLOR_BUFFER_BIT);
            renderer.render(commands, text_arena, framebuffer_w, framebuffer_h, nullptr);
            detail::copy_full_to_cache(runtime, framebuffer_w, framebuffer_h);
        } else {
            detail::draw_cache_texture(runtime, framebuffer_w, framebuffer_h);
            if (has_dirty) {
                const detail::IRect gl_dirty = detail::to_gl_rect(dirty, framebuffer_w, framebuffer_h);
                if (gl_dirty.w > 0 && gl_dirty.h > 0) {
                    glEnable(GL_SCISSOR_TEST);
                    glScissor(gl_dirty.x, gl_dirty.y, gl_dirty.w, gl_dirty.h);
                    glClearColor(bg.r, bg.g, bg.b, bg.a);
                    glClear(GL_COLOR_BUFFER_BIT);
                    renderer.render(commands, text_arena, framebuffer_w, framebuffer_h, &dirty);
                    glDisable(GL_SCISSOR_TEST);
                    detail::copy_region_to_cache(runtime, gl_dirty);
                }
            }
        }

        runtime.prev_commands = commands;
        runtime.prev_text_arena = text_arena;
        runtime.prev_bg = bg;
        runtime.has_prev_frame = true;
        glfwSwapBuffers(window);

        if (options.max_fps > 0.0) {
            const double frame_interval = 1.0 / options.max_fps;
            next_frame_deadline = glfwGetTime() + frame_interval;
        }
    }

    if (runtime.cache_texture != 0u) {
        glDeleteTextures(1, &runtime.cache_texture);
        runtime.cache_texture = 0u;
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}

}  // namespace demo

#endif  // EUI_ENABLE_GLFW_OPENGL_BACKEND

}  // namespace eui

#endif  // EUI_H_
