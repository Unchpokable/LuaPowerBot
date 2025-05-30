#include "modals.hxx"

#include <format>

namespace modals::data {

id_type counter;
std::deque<id_type> active_modals;
std::unordered_map<id_type, std::unique_ptr<Modal>> modals_registry;

}

modals::id_type modals::add_modal(Modal *modal) {
    auto id = data::counter++;
    data::active_modals.push_back(id);
    data::modals_registry.insert_or_assign(id, std::unique_ptr<Modal>(modal));

    modal->set_popup_id(std::format("modal_popup##{}", id));

    return id;
}

modals::Modal* modals::ask_open(std::string_view title, std::string_view initial_path, std::string_view filters, bool blocking) {
    auto modal = new FileDialogModal(title, initial_path, filters, blocking);
    add_modal(modal);

    return modal;
}

modals::Modal* modals::ask_yes_no(std::string_view title, std::string_view question, bool blocking) {
    auto modal = new YesNoModal(title, question, blocking);
    add_modal(modal);

    return modal;
}

modals::Modal* modals::warn(std::string_view title, std::string_view message, bool blocking) {
    auto modal = new InfoModal(modals::Warning, title, message, blocking);
    add_modal(modal);

    return modal;
}

modals::Modal* modals::inform(std::string_view title, std::string_view message, bool blocking) {
    auto modal = new InfoModal(modals::Info, title, message, blocking);
    add_modal(modal);

    return modal;
}

modals::Modal* modals::error(std::string_view title, std::string_view message, bool blocking) {
    auto modal = new InfoModal(modals::Error, title, message, blocking);
    add_modal(modal);

    return modal;
}

bool modals::has_any_modal()
{
    return !data::active_modals.empty();
}

void modals::render_from_top() {
    auto top = data::active_modals.front();

    auto modal = data::modals_registry[top].get();

    auto result = modal->render();

    if(result == ModalEvent::Continues) {
        return;
    }

    data::modals_registry.erase(top); 
    data::active_modals.pop_front();
}

Expected<modals::ModalEvent, errors::Error> modals::forced_render(id_type modal_id) {
    auto modal = data::modals_registry.find(modal_id);

    if(modal == data::modals_registry.end()) {
        return errors::Error(std::format("Modal window with id {} does not exists", modal_id));
    }

    auto result = modal->second->render();

    if(result == ModalEvent::Continues) {
        return result;
    }

    auto modal_queue_id = std::ranges::find(data::active_modals, modal_id);
    if(modal_queue_id != data::active_modals.end()) {
        data::active_modals.erase(modal_queue_id);
    }

    data::modals_registry.erase(modal_id);

    return result;
}
