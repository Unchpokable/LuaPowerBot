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

    void manage_message(const TgBot::Message::Ptr& message);
    void manage_callback(const TgBot::CallbackQuery::Ptr& callbackQuery);

    void update();

    TimePoint last_activity() const;

    void force_close();

private:
    void map_commands();

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

    void enqueue_task(NoReturningTask task);

    void manage_message(const TgBot::Message::Ptr& message);
    void manage_message(const TgBot::CallbackQuery::Ptr& callbackQuery);

    UserSession::TimePoint last_activity() const;
    void force_close();

    void update();

private:
    void thread_func();

    bool _running { true };

    std::thread _thread;
    std::mutex _mutex;

    std::condition_variable _condition;
    std::queue<NoReturningTask> _tasks;

    UserSession _session;
};

}
