#include "lua_api_types.hxx"

void lua::api::types::register_types(sol::state_view state)
{
    state.new_usertype<ui::InlineKeyboardButton>("InlineKeyboardButton");
    state.new_usertype<ui::InlineKeyboard>("InlineKeyboard");
    state.new_usertype<ui::ReplyKeyboardButton>("ReplyKeyboardButton");
    state.new_usertype<ui::ReplyKeyboard>("ReplyKeyboard");

    state.new_enum("CoroutineStep",
        "Step", routines::CoroutineStep::Step,
        "Done", routines::CoroutineStep::Done);
}
