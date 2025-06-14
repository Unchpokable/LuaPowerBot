#pragma once

namespace editor::state {

using ImGuiKeyChord = int;
using Callback = std::function<void()>;

extern bool is_maximized;
extern bool ignore_keyboard_input;

void bind_key(ImGuiKeyChord key, Callback callback);
void unbind_key(ImGuiKeyChord key);

void handle_keyboard();

}
