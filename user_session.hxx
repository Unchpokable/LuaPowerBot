#pragma once

#include <chrono>
#include <queue>
#include <string>
#include <unordered_map>

#include <tgbot/tgbot.h>

#include "lua_load.hxx"

namespace tg {

class UserSession
{
public:
    using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

    UserSession(const std::shared_ptr<TgBot::Bot>& bot);

    void manage(const TgBot::Message::Ptr& message);
    void manageCallback(const TgBot::CallbackQuery::Ptr& callbackQuery);

    void update();

    TimePoint lastActivity() const;

    void forceClose();

private:
    void mapCommands();

    std::shared_ptr<TgBot::Bot> _bot;

    std::queue<sol::coroutine> _coroutines;
    std::unique_ptr<lua::CommandBox> _commandBox;
    std::unordered_map<std::string, sol::function> _mappedCommands;

    TimePoint _lastActivity;
};

}
