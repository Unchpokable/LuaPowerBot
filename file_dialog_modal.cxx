#include <algorithm>
#include <filesystem>
#include <ranges>

#include "logdef.hxx"
#include "modals.hxx"

#include "thirdparty/imgui-docking/imgui.h"

namespace fs = std::filesystem;

modals::FileDialogModal::FileDialogModal(std::string_view title, std::string_view initial_path, std::string_view filters, bool blocking)
    : Modal(title, blocking), _initial_path(initial_path), _filter(filters), _current_path(initial_path)
{
    if(_current_path.empty()) {
        _current_path = fs::current_path().string();
    }

    scan_directory();
}

modals::ModalEvent modals::FileDialogModal::render() const
{
    ModalEvent result = Continues;

    if(!ImGui::IsPopupOpen(_imgui_popup_id.c_str())) {
        ImGui::OpenPopup(_imgui_popup_id.c_str());
    }

    constexpr float min_width = 600.0f;
    constexpr float min_height = 400.0f;
    constexpr float max_width = 800.0f;
    constexpr float max_height = 600.0f;

    size_t total_items = _directories.size() + _files.size();
    float item_height = ImGui::GetTextLineHeightWithSpacing();
    float needed_height = total_items * item_height + 150.0f; 

    float window_width = std::max(min_width, std::min(max_width, min_width + 50.0f));
    float window_height = std::max(min_height, std::min(max_height, needed_height));

    ImGui::SetNextWindowSize(ImVec2(window_width, window_height), ImGuiCond_Appearing);

    if(_blocking) {
        ImGui::BeginPopupModal(_imgui_popup_id.c_str(), nullptr, ImGuiWindowFlags_NoCollapse);
    } else {
        ImGui::BeginPopup(_imgui_popup_id.c_str());
    }

    ImGui::Text("Path: %s", _current_path.c_str());
    ImGui::Separator();

    float available_height = ImGui::GetContentRegionAvail().y;
    float bottom_ui_height = ImGui::GetFrameHeightWithSpacing() * 3 + ImGui::GetStyle().ItemSpacing.y * 2;
    float file_list_height = available_height - bottom_ui_height;

    file_list_height = std::max(file_list_height, 100.0f);

    ImGui::BeginChild("FileBrowser", ImVec2(0, file_list_height), true);

    int index = 0;

    for(const auto& dir : _directories) {
        bool is_selected = (_selected_index == index);

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f)); // Yellow for folders
        if(ImGui::Selectable(("[DIR] " + dir).c_str(), is_selected, ImGuiSelectableFlags_AllowDoubleClick)) {
            _selected_index = index;
            _selected_path.clear();

            if(ImGui::IsMouseDoubleClicked(0)) {
                if(dir == "..") {
                    fs::path p(_current_path);
                    if(p.has_parent_path()) {
                        _current_path = p.parent_path().string();
                    }
                } else {
                    _current_path = (fs::path(_current_path) / dir).string();
                }

                scan_directory();

                // re-scan other directory is a good reason to early return and go to the next render cycle
                ImGui::PopStyleColor();
                ImGui::EndChild();
                ImGui::EndPopup();
                return result;
            }
        }
        ImGui::PopStyleColor();
        index++;
    }

    for(const auto& file : _files) {
        bool is_selected = (_selected_index == index);

        if(ImGui::Selectable(file.c_str(), is_selected, ImGuiSelectableFlags_AllowDoubleClick)) {
            _selected_index = index;
            _selected_path = (fs::path(_current_path) / file).string();

            if(ImGui::IsMouseDoubleClicked(0)) {
                if(_handlers.contains(Ok)) {
                    invoke_handler(ModalEvent::Ok, std::any { _selected_path });
                }
                result = Ok;
            }
        }
        index++;
    }

    ImGui::EndChild();

    ImGui::Separator();

    std::string display_path = _selected_path;
    float available_width = ImGui::GetContentRegionAvail().x - 80.0f;

    if(!display_path.empty()) {
        ImVec2 text_size = ImGui::CalcTextSize(display_path.c_str());
        if(text_size.x > available_width) {
            while(display_path.length() > 3 && ImGui::CalcTextSize(("..." + display_path).c_str()).x > available_width) {
                size_t first_slash = display_path.find_first_of("/\\");
                if(first_slash != std::string::npos && first_slash < display_path.length() - 1) {
                    display_path = display_path.substr(first_slash + 1);
                } else {
                    display_path = display_path.substr(1);
                }
            }
            if(display_path.length() > 3) {
                display_path = "..." + display_path;
            }
        }
    }

    ImGui::Text("Selected: %s", display_path.c_str());

    ImGui::Separator();

    constexpr float button_width = 120.0f;
    const float button_spacing = ImGui::GetStyle().ItemSpacing.x;
    const float total_buttons_width = button_width * 2 + button_spacing;
    const float window_width_current = ImGui::GetWindowSize().x;
    const float buttons_start_x = (window_width_current - total_buttons_width) * 0.5f;

    ImGui::SetCursorPosX(buttons_start_x);

    bool can_select = !_selected_path.empty() && fs::exists(_selected_path) && fs::is_regular_file(_selected_path);

    if(!can_select) {
        ImGui::BeginDisabled();
    }

    if(ImGui::Button("OK", ImVec2(button_width, 0))) {
        if(can_select) {
            if(_handlers.contains(Ok)) {
                invoke_handler(ModalEvent::Ok, _selected_path);
            }
            result = Ok;
        }
    }

    if(!can_select) {
        ImGui::EndDisabled();
    }

    ImGui::SameLine();

    if(ImGui::Button("Cancel", ImVec2(button_width, 0))) {
        if(_handlers.contains(Cancel)) {
            invoke_handler(ModalEvent::Cancel);
        }
        result = Cancel;
    }

    ImGui::EndPopup();

    return result;
}

void modals::FileDialogModal::scan_directory() const
{
    _directories.clear();
    _files.clear();

    _selected_index = -1;

    try {
        fs::path path(_current_path);

        if(path.has_parent_path()) {
            _directories.emplace_back("..");
        }

        for(const auto& entry : fs::directory_iterator(path)) {
            if(entry.is_directory()) {
                _directories.push_back(entry.path().filename().string());
            } else if(entry.is_regular_file()) {
                auto filename = entry.path().filename().string();

                if(matches_filter(filename)) {
                    _files.push_back(filename);
                }
            }
        }

        if(!_directories.empty() && _directories[0] == "..") {
            std::ranges::sort(_directories.begin() + 1, _directories.end());
        } else {
            std::ranges::sort(_directories);
        }

        std::ranges::sort(_files);
    } catch(std::exception& ex) {
        luabot_logErr("Error during enumerating folder: {}\n{}", _current_path, ex.what());
    }
}

bool modals::FileDialogModal::matches_filter(std::string_view filename) const
{
    if(_filter.empty() || _filter == "*" || _filter == "*.*") {
        return true;
    }

    if(_filter[0] == '*' && _filter[1] == '.') {
        auto extension = _filter.substr(1);

        return filename.size() >= extension.size() && filename.substr(filename.size() - extension.size()) == extension;
    }

    return false;
}
