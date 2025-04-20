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

    UserSession(const std::shared_ptr<TgBot::Bot>& bot, const BytecodeMap& commands);

    void manageMessage(const TgBot::Message::Ptr& message);
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

class UserSessionThread
{
public:
    using NoReturningTask = std::function<void()>;

    UserSessionThread(const std::shared_ptr<TgBot::Bot>& bot, const BytecodeMap& commands);
    ~UserSessionThread();

    void enqueueTask(NoReturningTask task);

    void manageMessage(const TgBot::Message::Ptr& message);
    void manageCallback(const TgBot::CallbackQuery::Ptr& callbackQuery);

    UserSession::TimePoint lastActivity() const;
    void forceClose();

    void update();

private:
    void threadFunc();

    bool _running { true };

    std::thread _thread;
    std::mutex _mutex;

    std::condition_variable _condition;
    std::queue<NoReturningTask> _tasks;

    UserSession _session;
};

}
