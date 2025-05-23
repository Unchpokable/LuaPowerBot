#include "modals.hxx"

#include "thirdparty/imgui-docking/imgui.h"

modals::FileDialogModal::FileDialogModal(std::string_view title, std::string_view initial_path, std::string_view filters, bool blocking)
    : Modal(title, blocking), _initial_path(initial_path), _filter(filters) {}

modals::ModalEvent modals::FileDialogModal::render() const
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
