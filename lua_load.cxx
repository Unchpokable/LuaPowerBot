#include "lua_load.hxx"

namespace fs = std::filesystem;

lua::CommandBox::CommandBox(sol::state&& state, std::string prefix) : _state(std::move(state)), _prefix(std::move(prefix)) { }

sol::global_table& lua::CommandBox::commands()
{
    return _state[_prefix].tbl;
}

Expected<BytecodeMap, errors::Error> lua::load_bytecode_map(const std::string& folder) {
    auto path = fs::path(folder);

    if(!exists(path)) {
        return errors::Error("Folder does not exists!");
    }

    if(!is_directory(path)) {
        return errors::Error("Given path should be a root directory!");
    }

    BytecodeMap result;

    for(const auto& entry: fs::directory_iterator(folder)) {
        if(entry.is_regular_file() && fs::path(entry).extension() == ".lua") {
            sol::state temp_state;
            temp_state.open_libraries(sol::lib::base);
            sol::load_result lua = temp_state.load_file(entry.path().string());
            if(!lua.valid()) {
                sol::error err = lua;
                return errors::Error("Unable to load script: " + std::string(err.what()));
            }

            auto func = lua.get<sol::function>();
            auto bytecode = temp_state["string"]["dump"](func);

            if(!bytecode.valid()) {
                sol::error err = bytecode;
                return errors::Error("Unable to compile script: " + std::string(err.what()));
            }

            result.insert_or_assign(entry.path().stem().string(), bytecode.get<std::string>());
        }
    }

    return result;
}

Expected<lua::CommandBox*, errors::Error> lua::make_state_from_cached_bytecode(const BytecodeMap& bytecode_map) {
    sol::state state;

    state.open_libraries(sol::lib::base);

    sol::table commands = state.create_table();

    for(const auto& [name, bytecode] : bytecode_map) {
        sol::load_result initializer_load_result = state.load(bytecode);
        if(!initializer_load_result.valid()) {
            sol::error err = initializer_load_result;
            return errors::Error("Unable to load bytecode from command [" + name + "]: " + err.what());
        }

        sol::function initializer = initializer_load_result.get<sol::function>();
        auto initializer_result = initializer();
        if(!initializer_result.valid()) {
            sol::error err = initializer_load_result;
            return errors::Error("Unable to call initializer for command [" + name + "]: " + err.what());
        }

        sol::table instance = initializer_result.get<sol::table>();
        commands[name] = instance;
    }

    state["commands"] = commands;
    return new CommandBox(std::move(state), "commands");
}

Expected<lua::CommandBox*, errors::Error> lua::load_scripts(const std::string& folder)
{
    auto path = fs::path(folder);

    if(!exists(path)) {
        return errors::Error("Folder does not exists!");
    }

    if(!is_directory(path)) {
        return errors::Error("Given path should be a root directory!");
    }

    sol::state state;
    state.open_libraries(sol::lib::base);

    sol::table commands = state.create_table();

    for(const auto& entry : fs::directory_iterator(folder)) {
        if(entry.is_regular_file() && fs::path(entry).extension() == "lua") {
            std::string command_name = entry.path().stem().string();

            sol::load_result result = state.load_file(entry.path().string());

            if(!result.valid()) {
                sol::error err = result;
                return errors::Error(err.what());
            }

            sol::function initializer = result.get<sol::function>();
            sol::table instance = initializer();
            commands[command_name] = instance;
        }

    }

    state["commands"] = commands;
    return new CommandBox(std::move(state), "commands");
}


