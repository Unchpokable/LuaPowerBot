#include "modals.hxx"

#include "thirdparty/imgui-docking/imgui.h"

modals::InfoModal::InfoModal(InfoType type, std::string_view title, std::string_view message, bool blocking)
    : Modal(std::string(title), blocking), _message(message), _type(type)
{ }

modals::ModalEvent modals::InfoModal::render() const
{
    ModalEvent result = Continues;

    if(_blocking) {
        ImGui::BeginPopupModal(_imgui_popup_id.c_str());
    } else {
        ImGui::BeginPopup(_imgui_popup_id.c_str());
    }

    ImGui::BeginChild((_imgui_popup_id + "_info").c_str(), ImVec2(250, 150));

    switch(_type) {
        case Info:
        {
            auto label_color = ImGui::ColorConvertU32ToFloat4(IM_COL32(3, 154, 255, 255));
            ImGui::TextColored(label_color, "[Info]");
            break;
        }
        case Warning:
        {
            auto label_color = ImGui::ColorConvertU32ToFloat4(IM_COL32(255, 234, 3, 255));
            ImGui::TextColored(label_color, "[Warn]");
            break;
        }
        case Error:
        {
            auto label_color = ImGui::ColorConvertU32ToFloat4(IM_COL32(212, 51, 51, 255));
            ImGui::TextColored(label_color, "[Error]");
            break;
        }
    }

    ImGui::SameLine();
    ImGui::Text(_message.c_str());

    if(ImGui::Button("OK")) {
        auto handler = _handlers.find(ModalEvent::Ok);
        handler->second();

        result = ModalEvent::Ok;
    }

    ImGui::EndPopup();

    return result;
}
