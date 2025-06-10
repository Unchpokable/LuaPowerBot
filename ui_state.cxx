#include <unordered_map>
#include <functional>
#include <ranges>

#include "ui_state.hxx"

#include "thirdparty/imgui-docking/imgui.h"
#include "tracy/tracy/Tracy.hpp"

bool editor::state::is_maximized = false;
bool editor::state::ignore_keyboard_input = false;

namespace editor::state::data {

std::unordered_map<ImGuiKeyChord, Callback> key_bindings;

}

void editor::state::bind_key(ImGuiKeyChord key, Callback callback)
{
    data::key_bindings.insert_or_assign(key, std::move(callback));
}

void editor::state::unbind_key(ImGuiKeyChord key)
{
    auto iter = data::key_bindings.find(key);
    if(iter != data::key_bindings.end()) {
        data::key_bindings.erase(iter);
    }
}

void editor::state::handle_keyboard()
{
    ZoneScoped;
    if(ignore_keyboard_input) {
        return;
    }

    for(auto key: data::key_bindings | std::views::keys) {
        if(ImGui::IsKeyChordPressed(key)) {
            data::key_bindings[key]();
        }
    }
}
