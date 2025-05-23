#include "modals.hxx"

#include "thirdparty/imgui-docking/imgui.h"

modals::YesNoModal::YesNoModal(std::string_view title, std::string_view question, bool blocking) : Modal(title, blocking), _question(question)
{ }

modals::ModalEvent modals::YesNoModal::render() const
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
