#include "lua_api_functions.hxx"

sol::coroutine lua::api::functions::makeCoroutine(const sol::function& func)
{
    sol::coroutine coroutine = sol::coroutine(func.lua_state(), func);
    return coroutine;
}
