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



    ImGui::EndPopup();

    return result;
}
