#include <ranges>

#include "user_session.hxx"

#include "lua_api_types.hxx"

#include "strings.hxx"

#include "logdef.hxx"

tg::UserSession::UserSession(const std::shared_ptr<TgBot::Bot>& bot, const BytecodeMap& commands) : _bot(bot) {
    auto commandBoxResult = lua::make_state_from_cached_bytecode(commands);

    if(!commandBoxResult) {
        auto err = commandBoxResult.error();
        luabot_logFatal("An error happens during making a Lua state from cached bytecode {}", err.message());
        err.throwError<std::exception>();
    }

    auto commandBox = commandBoxResult.value();
    _commandBox.reset(commandBox);
}

void tg::UserSession::manageMessage(const TgBot::Message::Ptr& message)
{
    _lastActivity = std::chrono::high_resolution_clock::now();

    return;
}

void tg::UserSession::manageCallback(const TgBot::CallbackQuery::Ptr& callbackQuery)
{
    _lastActivity = std::chrono::high_resolution_clock::now();
    auto tokens = utils::string_split(callbackQuery->data, ';');

    if(tokens.size() < 2) {
        luabot_logErr("Invalid callbackQuery data format, expected `function;data`, got {}", callbackQuery->data);
    }

    auto commandName = tokens[0];
    auto callbackData = tokens[1];

    auto command = _commandBox->commands()[commandName]["CallbackQuery"].get<sol::function>();

    if(!command.valid()) {
        luabot_logFatal("CallbackQuery provided by script {} is not a function!", commandName);
        return;
    }

    auto result = command(callbackData);

    if(!result.valid()) {
        luabot_logFatal("CallbackQuery provided by {} called with failure: {}", commandName, result.get<sol::error>().what());
        return;
    }

    // todo: handle if callback query returned coroutine or smth like that;
}

void tg::UserSession::update()
{
    for(std::ptrdiff_t i = 0; i < _coroutines.size(); i++) {
        auto coroutine = _coroutines.front();
        auto result = coroutine();
        _coroutines.pop();

        if(!result.valid()) {
            sol::error err = result;
            luabot_logErr("Coroutine resume failed: {}", err.what());
            continue;
        }

        sol::object obj = result;
        if(obj.is<lua::api::types::routines::CoroutineStep>()) {
            auto status = obj.as<lua::api::types::routines::CoroutineStep>();
            if(status == lua::api::types::routines::CoroutineStep::Step) {
                _coroutines.push(coroutine);
            }
        } else {
            luabot_logErr("Coroutine returned unsupported type");
        }
    }
}

tg::UserSession::TimePoint tg::UserSession::lastActivity() const
{
    return _lastActivity;
}

void tg::UserSession::forceClose()
{
}

void tg::UserSession::mapCommands()
{
    for (const auto& [name, func_object] : _commandBox->commands()) {
        auto str_name = name.as<std::string>();
        
        if(!func_object.is<sol::function>()) {
            luabot_logErr("Lua object named {} is not a function!", str_name);
        }

        auto function = func_object.as<sol::function>();

        _mappedCommands.insert_or_assign(str_name, function);
    }
}

tg::UserSessionThread::UserSessionThread(const std::shared_ptr<TgBot::Bot>& bot, const BytecodeMap& commands)
    : _session(bot, commands)
{
    _thread = std::thread(&UserSessionThread::threadFunc, this);
    luabot_logInfo("Started UserSessionThread");
}

tg::UserSessionThread::~UserSessionThread()
{
    {
        std::unique_lock lock(_mutex);
        _running = false;
        _condition.notify_one();
    }

    if(_thread.joinable()) {
        _thread.join();
    }


}

void tg::UserSessionThread::enqueueTask(NoReturningTask task)
{
    {
        std::unique_lock lock(_mutex);
        _tasks.push(std::move(task));
    }

    _condition.notify_one();
}

void tg::UserSessionThread::manageMessage(const TgBot::Message::Ptr& message)
{
    enqueueTask([this, message]() {
        _session.manageMessage(message);
    });
}

void tg::UserSessionThread::manageCallback(const TgBot::CallbackQuery::Ptr& callbackQuery)
{
    enqueueTask([this, callbackQuery]() {
        _session.manageCallback(callbackQuery);
    });
}

tg::UserSession::TimePoint tg::UserSessionThread::lastActivity() const
{
    return _session.lastActivity();
}

void tg::UserSessionThread::forceClose()
{
    enqueueTask([this]() {
        _session.forceClose();
    });
}

void tg::UserSessionThread::update()
{
    enqueueTask([this]() {
        try {
            _session.update();
        } catch(const std::exception& e) {
            luabot_logErr("Exception in manual UserSession update: {}", e.what());
        }
    });

    _condition.notify_one();
}

void tg::UserSessionThread::threadFunc()
{
    while(true) {
        NoReturningTask task;

        {
            std::unique_lock lock(_mutex);
            _condition.wait(lock, [this] {
                return !_running || !_tasks.empty();
            });

            if(!_running && _tasks.empty()) {
                break;
            }

            if(!_tasks.empty()) {
                task = std::move(_tasks.front());
                _tasks.pop();
            }
        }

        if(task) {
            try {
                task();
            } catch(const std::exception& e) {
                luabot_logErr("Exception in UserSessionThread task: {}", e.what());
            } catch(...) {
                luabot_logErr("Unknown exception in UserSessionThread task");
            }
        }
    }
}
