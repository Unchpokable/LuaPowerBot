#include "lua_load.hxx"

namespace fs = std::filesystem;

lua::CommandBox::CommandBox(sol::state&& state, std::string prefix) : _state(std::move(state)), _prefix(std::move(prefix)) { }

sol::global_table& lua::CommandBox::commands()
{
    return _state[_prefix].tbl;
}

Expected<lua::CommandBox*, std::exception> lua::load_scripts(const std::string& folder)
{
    auto path = fs::path(folder);

    if(!exists(path)) {
        return std::exception("Folder does not exists!");
    }

    if(!is_directory(path)) {
        return std::exception("Given path should be a root directory!");
    }


    for(const auto& entry : fs::directory_iterator(path)) {
        if(entry.is_regular_file() && fs::path(entry).extension() == "lua") {

        }
    }

    sol::state state;
    state.open_libraries(sol::lib::base);

    sol::table commands = state.create_table();

    for(const auto& entry : fs::directory_iterator(folder)) {
        std::string command_name = entry.path().stem().string();

        sol::load_result result = state.load_file(entry.path().string());

        if(!result.valid()) {
            sol::error err = result;
            return std::exception(err.what());
        }

        sol::function initializer = result.get<sol::function>();
        sol::table instance = initializer();
        commands[command_name] = instance;
    }

    state["commands"] = commands;

    return new CommandBox(std::move(state), "commands");
}


