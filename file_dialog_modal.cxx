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

    if(_blocking) {
        ImGui::BeginPopupModal(_imgui_popup_id.c_str(), nullptr, ImGuiWindowFlags_NoCollapse);
    } else {
        ImGui::BeginPopup(_imgui_popup_id.c_str());
    }

    ImGui::Text("Path: %s", _current_path.c_str());

    ImGui::Separator();

    ImGui::BeginChild("FileBrowser", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() * 2), true);

    int index = 0;

    for(const auto& dir : _directories) {
        bool is_selected = (_selected_index == index);

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f)); // Yellow for folders
        if(ImGui::Selectable(("[DIR] " + dir).c_str(), is_selected, ImGuiSelectableFlags_AllowDoubleClick)) {
            _selected_index = index;

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
                    auto handler = arguments_callback<std::string_view>({ Ok, _handlers.at(Ok) });
                    handler(_selected_path);
                }
                result = Ok;
            }
        }
        index++;
    }

    ImGui::Text("Selected: %s", _selected_path.c_str());

    ImGui::Separator();

    if(ImGui::Button("OK", ImVec2(120, 0))) {
        if(!_selected_path.empty() && fs::exists(_selected_path) && fs::is_regular_file(_selected_path)) {
            if(_handlers.contains(Ok)) {
                auto handler = arguments_callback<std::string_view>({ Ok, _handlers.at(Ok) });
                handler(_selected_path);
            }
            result = Ok;
        }
    }

    ImGui::SameLine();

    if(ImGui::Button("Cancel", ImVec2(120, 0))) {
        if(_handlers.contains(Cancel)) {
            auto handler = _handlers.at(Cancel);
            handler();
        }
        result = Cancel;
    }

    ImGui::EndChild();

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
