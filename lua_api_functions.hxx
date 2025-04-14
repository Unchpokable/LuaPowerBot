#pragma once

#include <sol/sol.hpp>

#include "lua_api_types.hxx"

namespace lua::api::functions {

sol::coroutine makeCoroutine(const sol::function& func);

}
