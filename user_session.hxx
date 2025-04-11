#pragma once
#include <string>
#include <unordered_map>

#include <tgbot/tgbot.h>

#include "lua_load.hxx"

namespace tg {

class UserSession
{
public:
    void manage(TgBot::Message::Ptr message);

private:
    std::unordered_map<std::string, lua::LuaScript*> _commands;
};

}
