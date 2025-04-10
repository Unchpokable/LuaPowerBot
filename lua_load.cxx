#include "lua_load.hxx"

namespace fs = std::filesystem;

lua::LuaScript::LuaScript(LuaScript&& other) noexcept : originFile(std::move(other.originFile)), luaState(std::move(other.luaState)) { }

lua::LuaScript& lua::LuaScript::operator=(LuaScript&& other) noexcept {
    originFile = std::move(other.originFile);
    luaState = std::move(other.luaState);

    return *this;
}

std::string lua::LuaScript::name() const {
    return fs::path(originFile).stem().string();
}

Expected<std::vector<lua::LuaScript*>, std::exception> lua::load_scripts(const std::string& folder)
{
    auto path = fs::path(folder);

    if(!exists(path)) {
        return std::exception("Folder does not exists!");
    }

    if(!is_directory(path)) {
        return std::exception("Given path should be a root directory!");
    }

    std::vector<LuaScript*> scripts;

    for (const auto& entry : fs::directory_iterator(path)) {
        if(entry.is_regular_file() && fs::path(entry).extension() == "lua") {
            auto script = internal::load_script(entry.path().string());
            if(script) {
                scripts.push_back(script.value());
            } else {
                std::string err_msg = "Unable to load script: " + fs::path(entry).string();
                return std::exception(err_msg.c_str());
            }
        }
    }

    return scripts;
}


