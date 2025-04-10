#pragma once

#include <tgbot/tgbot.h>

namespace lua {
struct LuaScript;
}

namespace tg {

class BotRuntimeContext {
public:
    

private:
    void toKeys(std::vector<lua::LuaScript*> scripts);

    std::unordered_map<std::string, lua::LuaScript*> _commands;

    std::mutex _mutex;
    std::thread _botThread;

    std::vector<uint64_t> _maintainers;
};

}
