#include "modals.hxx"

#include "thirdparty/imgui-docking/imgui.h"

modals::InputModal::InputModal(std::string_view title, std::string_view prompt, bool blocking) : Modal(title, blocking)
{
    _prompt = prompt;
}

modals::ModalEvent modals::InputModal::render() const
{
    ModalEvent result = Continues;

    if(!ImGui::IsPopupOpen(_imgui_popup_id.c_str())) {
        ImGui::OpenPopup(_imgui_popup_id.c_str());
    }

    return result;
}


