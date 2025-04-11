#pragma once

#include <string>
#include <vector>

#include <sol/sol.hpp>

namespace lua::api::types {

struct InlineKeyboardButton {
    std::string text;
    std::string callbackData;
};

struct InlineKeyboard {
    using ButtonRow = std::vector<InlineKeyboardButton>;

    std::vector<ButtonRow> buttons;
};

struct ReplyKeyboardButton {
    std::string text;
};

struct ReplyKeyboard {
    using ButtonRow = std::vector<ReplyKeyboardButton>;

    std::vector<ButtonRow> buttons;
};

void register_types(sol::state_view state);

}

