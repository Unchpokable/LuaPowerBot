#pragma once

#include <sol/sol.hpp>

namespace lua::api {

void register_api(sol::state_view state);

}