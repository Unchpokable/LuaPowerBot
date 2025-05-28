#include "modals.hxx"

modals::Modal::Modal(std::string_view title, bool blocking) : _title(title), _blocking(blocking)
{ }

void modals::Modal::add_callback(handler handler)
{
    _handlers.insert_or_assign(handler.first, handler.second);
}

void modals::Modal::set_popup_id(std::string_view popup_id)
{
    _imgui_popup_id = popup_id;
}

bool modals::Modal::is_blocking() const
{
    return _blocking;
}
