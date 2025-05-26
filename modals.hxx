#pragma once

#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "error.hxx"
#include "expected.hxx"

namespace modals {

enum InfoType {
    Info,
    Warning,
    Error
};

enum ModalEvent {
    Ok,
    Cancel,
    Yes,
    No,
    Close,
    Continues // means that modal window rendered something but not ended its routine. DO NOT ADD CALLBACKS TO THIS
};

using anon_func_ptr = void(*)();
using handler = std::pair<ModalEvent, anon_func_ptr>;

using id_type = int;

template<typename T>
concept event_handler = std::same_as<T, handler>;

template<typename F>
constexpr auto anonymize_function(F function) -> void(*)()
{
    return reinterpret_cast<void(*)()>(function);
}

template<typename ...Args>
constexpr auto arguments_callback(handler handler) -> void(*)(Args...)
{
    return reinterpret_cast<void(*)(Args...)>(handler.second);
}

template<typename F>
constexpr handler handle(ModalEvent event, F handler_func)
{
    auto anonymized_function = anonymize_function(handler_func);

    return handler { event, anonymized_function };
}

class Modal
{
public:
    explicit Modal(std::string_view title, bool blocking);
    virtual ~Modal() = default;

    virtual ModalEvent render() const = 0;

    void add_callback(handler handler);

    void set_popup_id(std::string_view popup_id);

protected:
    std::string _imgui_popup_id;
    std::string _title;
    bool _blocking;
    std::unordered_map<ModalEvent, anon_func_ptr> _handlers;
};

class FileDialogModal : public Modal
{
public:
    explicit FileDialogModal(std::string_view title, std::string_view initial_path, std::string_view filters, bool blocking);
    virtual ~FileDialogModal() override = default;

    virtual ModalEvent render() const override;

private:
    void scan_directory() const;
    bool matches_filter(std::string_view filename) const;

    std::string _initial_path;
    std::string _filter;
    mutable std::string _selected_path;
    mutable std::string _current_path;

    mutable std::vector<std::string> _directories;
    mutable std::vector<std::string> _files;

    mutable int _selected_index = -1;
};

class YesNoModal : public Modal
{
public:
    explicit YesNoModal(std::string_view title, std::string_view question, bool blocking);
    virtual ~YesNoModal() override = default;

    virtual ModalEvent render() const override;

private:
    std::string _question;
};

class InfoModal : public Modal
{
public:
    InfoModal(InfoType type, std::string_view title, std::string_view message, bool blocking);
    virtual ~InfoModal() override = default;

    virtual ModalEvent render() const override;

private:
    std::string _message;
    InfoType _type;
};

// internal implementation function. If you're using this directly outside this module - you're doing wrong 
id_type add_modal(Modal* modal); 

template<event_handler ...Handlers>
id_type ask_open(std::string_view title, std::string_view initial_path, std::string_view filters, bool blocking, Handlers... handlers);

template<event_handler ...Handlers>
id_type ask_yes_no(std::string_view title, std::string_view question, bool blocking, Handlers... handlers);

id_type warn(std::string_view title, std::string_view message, bool blocking);
id_type inform(std::string_view title, std::string_view message, bool blocking);
id_type error(std::string_view title, std::string_view message, bool blocking);

bool has_any_modal();

///@brief renders a modal window on top of the queue. Pops it from queue if modal dialog returned final status code and lefts it on top of the queue if it returned \c ModalEvent::Continues
ModalEvent render_top();

Expected<ModalEvent, errors::Error> forced_render(id_type modal_id);

}

template<modals::event_handler ...Handlers>
modals::id_type modals::ask_open(std::string_view title, std::string_view initial_path, std::string_view filters, bool blocking, Handlers ...handlers)
{
    auto modal = new FileDialogModal(title, initial_path, filters, blocking);

    (modal->add_callback(handlers), ...);

    return add_modal(modal);
}

template<modals::event_handler ...Handlers>
modals::id_type modals::ask_yes_no(std::string_view title, std::string_view question, bool blocking, Handlers ...handlers)
{
    auto modal = new YesNoModal(title, question, blocking);

    (modal->add_callback(handlers), ...);

    return add_modal(modal);
}
