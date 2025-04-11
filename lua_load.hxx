#pragma once

#include <filesystem>

#include <sol/sol.hpp>

#include "expected.hxx"

namespace lua {

struct LuaScript
{
    LuaScript(std::string file, sol::state&& state) : originFile(std::move(file)), luaState(std::move(state)) {}

    LuaScript(const LuaScript&) = delete;
    LuaScript& operator=(const LuaScript&) = delete;

    LuaScript(LuaScript&& other) noexcept;
    LuaScript& operator=(LuaScript&& other) noexcept;

    std::string name() const;

    std::string originFile;
    sol::state luaState;
};

Expected<std::vector<LuaScript*>, std::exception> load_scripts(const std::string& folder);

}


namespace lua::internal {

Expected<LuaScript*, std::exception> load_script(const std::string& file);
bool validate_basic_script_signature(const sol::state_view& state);

}