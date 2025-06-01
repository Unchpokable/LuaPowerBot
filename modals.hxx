#pragma once

#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <any>

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

using ModalCallback = std::function<void(std::any)>;
using handler = std::pair<ModalEvent, ModalCallback>;

using id_type = int;

class Modal
{
public:
    explicit Modal(std::string_view title, bool blocking);
    virtual ~Modal() = default;

    virtual ModalEvent render() const = 0;

    void add_callback(ModalEvent event, ModalCallback callback);
    void invoke_handler(ModalEvent event, const std::any &arg) const;
    void invoke_handler(ModalEvent event) const;

    void set_popup_id(std::string_view popup_id);

    bool is_blocking() const;

    // fluent builder time!
    template<typename Func>
    Modal& on(ModalEvent event, Func&& handler_func);

protected:
    std::string _imgui_popup_id;
    std::string _title;
    bool _blocking;
    std::unordered_map<ModalEvent, ModalCallback> _handlers;
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

class InputModal : public Modal
{
public:
    InputModal(std::string_view title, std::string_view prompt, bool blocking);
    virtual ~InputModal() override = default;

    virtual ModalEvent render() const override;

private:
    std::string _prompt;
    std::string _user_input;
};

// internal implementation function. If you're using this directly outside this module - you're doing wrong 
id_type add_modal(Modal* modal); 

Modal* ask_open(std::string_view title, std::string_view initial_path, std::string_view filters, bool blocking);

Modal* ask_yes_no(std::string_view title, std::string_view question, bool blocking);

Modal* warn(std::string_view title, std::string_view message, bool blocking);
Modal* inform(std::string_view title, std::string_view message, bool blocking);
Modal* error(std::string_view title, std::string_view message, bool blocking);

bool has_any_modal();

void render_from_top();

Expected<ModalEvent, errors::Error> forced_render(id_type modal_id);

}

template <typename Func>
modals::Modal& modals::Modal::on(ModalEvent event, Func&& handler_func) {
    add_callback(event, std::forward<Func>(handler_func));

    return *this;
}
