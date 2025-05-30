#include "modals.hxx"

#include "thirdparty/imgui-docking/imgui.h"

modals::YesNoModal::YesNoModal(std::string_view title, std::string_view question, bool blocking) : Modal(title, blocking), _question(question)
{ }

modals::ModalEvent modals::YesNoModal::render() const
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

    ImVec2 text_size = ImGui::CalcTextSize(_question.c_str(), nullptr, true, max_width - 60.0f);
    float window_width = ImGui::GetStyle().ItemSpacing.x * 2 + text_size.x + 40.0f;

    window_width = std::max(min_width, std::min(window_width, max_width));

    ImGui::SetNextWindowSize(ImVec2(window_width, 0), ImGuiCond_Always);

    ImGui::Dummy(ImVec2(0, 10));

    ImGui::Indent(20.0f);
    auto label_color = ImGui::ColorConvertU32ToFloat4(IM_COL32(3, 154, 255, 255));
    ImGui::TextColored(label_color, "[Question]");
    ImGui::Unindent(20.0f);

    ImGui::Dummy(ImVec2(0, 5));

    ImGui::Indent(20.0f);
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + window_width - 60.0f);
    ImGui::TextWrapped("%s", _question.c_str());
    ImGui::PopTextWrapPos();
    ImGui::Unindent(20.0f);

    ImGui::Dummy(ImVec2(0, 15));

    constexpr float button_width = 80.0f;
    constexpr float button_spacing = 10.0f;
    constexpr float total_buttons_width = button_width * 2 + button_spacing;
    const float window_center = ImGui::GetWindowSize().x * 0.5f;
    const float buttons_start_x = window_center - total_buttons_width * 0.5f;

    ImGui::SetCursorPosX(buttons_start_x);

    if(ImGui::Button("Yes", ImVec2(button_width, 0))) {
        invoke_handler(ModalEvent::Yes);
        result = ModalEvent::Yes;
    }

    ImGui::SameLine();
    ImGui::SetCursorPosX(buttons_start_x + button_width + button_spacing);

    if(ImGui::Button("No", ImVec2(button_width, 0))) {
        invoke_handler(ModalEvent::No);
        result = ModalEvent::No;
    }

    ImGui::Dummy(ImVec2(0, 10));

    ImGui::EndPopup();

    return result;
}
