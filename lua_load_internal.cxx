#include "lua_load.hxx"

using namespace lua::internal;

Expected<lua::LuaScript*, std::exception> lua::internal::load_script(const std::string& file)
{
    sol::state script_state;
    script_state.open_libraries(sol::lib::base);
    auto result = script_state.safe_script_file(file);

    if(!result.valid()) {
        return std::exception(result.get<sol::error>().what());
    }

    if(!validate_basic_script_signature(script_state)) {
        return std::exception("on-load check failed: no Startup() or Execute() function found");
    }

    auto lua_script = new LuaScript(file, std::move(script_state));

    return lua_script;
}

bool lua::internal::validate_basic_script_signature(const sol::state_view& state)
{
    static const std::string signature_required[] = {"Startup", "Execute"};

    for(auto &function : signature_required) {
        sol::object object = state[function];

        if(!object.valid() || object.get_type() != sol::type::function) {
            return false;
        }
    }

    return true;
}

