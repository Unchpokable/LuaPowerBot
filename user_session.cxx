#include <ranges>

#include "user_session.hxx"

#include "lua_api_types.hxx"

tg::UserSession::UserSession(const std::shared_ptr<TgBot::Bot>& bot) : _bot(bot) {}

void tg::UserSession::manage(const TgBot::Message::Ptr& message)
{
    _lastActivity = std::chrono::high_resolution_clock::now();

    return;
}

void tg::UserSession::manageCallback(const TgBot::CallbackQuery::Ptr& callbackQuery)
{
    _lastActivity = std::chrono::high_resolution_clock::now();

    return;
}

void tg::UserSession::update()
{
    for(std::ptrdiff_t i = 0; i < _coroutines.size(); i++) {
        auto coroutine = _coroutines.front();
        auto result = coroutine();
        _coroutines.pop();

        if(!result.valid()) {
            continue; // todo: log that coroutine is failed
        }

        sol::object obj = result;
        if(obj.is<lua::api::types::routines::CoroutineStep>()) {
            auto status = obj.as<lua::api::types::routines::CoroutineStep>();
            if(status == lua::api::types::routines::CoroutineStep::Step) {
                _coroutines.push(coroutine);
            }
        } else {
            continue; // todo: log that coroutine returned something wrong
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
    for (const auto& entry : _commandBox) {
        
    }
}
