#include "lua_api_types.hxx"

void lua::api::types::register_types(sol::state_view state)
{
    state.new_usertype<InlineKeyboardButton>("InlineKeyboardButton");
    state.new_usertype<InlineKeyboard>("InlineKeyboard");
    state.new_usertype<ReplyKeyboardButton>("ReplyKeyboardButton");
    state.new_usertype<ReplyKeyboard>("ReplyKeyboard");
}
