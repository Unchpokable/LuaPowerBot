#include "modals.hxx"

#include "thirdparty/imgui-docking/imgui.h"
#include "thirdparty/imgui-docking/misc/cpp/imgui_stdlib.h"

modals::InputModal::InputModal(std::string_view title, std::string_view prompt, bool blocking) : Modal(title, blocking)
{
    _prompt = prompt;
}

modals::ModalEvent modals::InputModal::render() const
{
    if(!ImGui::IsPopupOpen(_imgui_popup_id.c_str())) {
        ImGui::OpenPopup(_imgui_popup_id.c_str());
    }

    ModalEvent result = Continues;

    if(_blocking) {
        ImGui::BeginPopupModal(_imgui_popup_id.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    } else {
        ImGui::BeginPopup(_imgui_popup_id.c_str());
    }

    constexpr float min_width = 300.0f;
    constexpr float max_width = 500.0f;

    ImVec2 text_size = ImGui::CalcTextSize(_prompt.c_str(), nullptr, true, max_width - 60.0f);
    float window_width = ImGui::GetStyle().ItemSpacing.x * 2 + text_size.x + 40.0f;

    window_width = std::max(min_width, std::min(window_width, max_width));

    ImGui::SetNextWindowSize(ImVec2(window_width, 0), ImGuiCond_Always);

    ImGui::Dummy(ImVec2(0, 10));

    ImGui::Indent(20.0f);
    auto label_color = ImGui::ColorConvertU32ToFloat4(IM_COL32(3, 154, 255, 255));
    ImGui::TextColored(label_color, "[Input]");
    ImGui::Unindent(20.0f);

    ImGui::Dummy(ImVec2(0, 5));

    ImGui::Indent(20.0f);
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + window_width - 60.0f);
    ImGui::TextWrapped("%s", _prompt.c_str());
    ImGui::PopTextWrapPos();
    ImGui::Unindent(20.0f);

    ImGui::Dummy(ImVec2(0, 10));

    ImGui::Indent(20.0f);
    ImGui::SetNextItemWidth(window_width - 60.0f);

    if(ImGui::IsWindowAppearing()) {
        ImGui::SetKeyboardFocusHere();
    }

    bool enter_pressed = ImGui::InputText("##input", &_user_input, ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::Unindent(20.0f);

    ImGui::Dummy(ImVec2(0, 15));

    constexpr float button_width = 80.0f;
    float window_center = ImGui::GetWindowSize().x * 0.5f;
    ImGui::SetCursorPosX(window_center - button_width * 0.5f);

    if(ImGui::Button("OK", ImVec2(button_width, 0)) || enter_pressed) {
        std::string input_copy = _user_input;
        invoke_handler(ModalEvent::Ok, std::any(input_copy));
        result = ModalEvent::Ok;
    }

    ImGui::Dummy(ImVec2(0, 10));

    ImGui::EndPopup();

    return result;
}


