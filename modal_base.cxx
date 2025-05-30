#include <any>

#include "modals.hxx"

modals::Modal::Modal(std::string_view title, bool blocking) : _title(title), _blocking(blocking)
{ }

void modals::Modal::add_callback(ModalEvent event, ModalCallback callback)
{
    _handlers.insert_or_assign(event, std::move(callback));
}

void modals::Modal::invoke_handler(ModalEvent event, const std::any &arg) const
{
    auto handler = _handlers.find(event);
    if(handler != _handlers.end()) {
        handler->second(arg);
    }
}

void modals::Modal::invoke_handler(ModalEvent event) const {
    invoke_handler(event, std::any {});
}

void modals::Modal::set_popup_id(std::string_view popup_id)
{
    _imgui_popup_id = popup_id;
}

bool modals::Modal::is_blocking() const
{
    return _blocking;
}
