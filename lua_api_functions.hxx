#pragma once

#include <sol/sol.hpp>

#include "lua_api_types.hxx"

namespace lua::api::functions {

types::routines::Coroutine make_coroutine(const sol::function& func, types::routines::CoroutinePolicy policy);

void register_functions(sol::state_view state);

}
