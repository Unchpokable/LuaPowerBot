#include "lua_api_functions.hxx"

lua::api::types::routines::Coroutine lua::api::functions::make_coroutine(const sol::function& func, types::routines::CoroutinePolicy policy)
{
    return { sol::coroutine(func.lua_state(), func), policy };
}

void lua::api::functions::register_functions(sol::state_view state)
{
    state.set_function("MakeCoroutine", &make_coroutine);
}
