#pragma once

#include <chrono>
#include <string>
#include <unordered_map>

#include <tgbot/tgbot.h>

#include "lua_load.hxx"

namespace tg {

class UserSession
{
public:
    UserSession(const std::shared_ptr<TgBot::Bot>& bot);

    void manage(const TgBot::Message::Ptr& message);
    void manageCallback(TgBot::CallbackQuery::Ptr callbackQuery);

    std::chrono::steady_clock lastActivity() const;

private:
    std::unordered_map<std::string, lua::LuaScript*> _rawCommands;
    lua::LuaScript* _activeCommand { nullptr };

    std::shared_ptr<TgBot::Bot> _bot;
    std::chrono::steady_clock _lastActivity;
};

}
