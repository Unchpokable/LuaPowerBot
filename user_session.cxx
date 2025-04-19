#include <ranges>

#include "user_session.hxx"

#include "lua_api_types.hxx"

#include "strings.hxx"

#include "logdef.hxx"

tg::UserSession::UserSession(const std::shared_ptr<TgBot::Bot>& bot, const BytecodeMap& commands) : _bot(bot) {
    auto commandBoxResult = lua::make_state_from_cached_bytecode(commands);

    if(!commandBoxResult) {
        commandBoxResult.error().throwError<std::exception>();
    }

    auto commandBox = commandBoxResult.value();
    _commandBox.reset(commandBox);
}

void tg::UserSession::manage(const TgBot::Message::Ptr& message)
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
        throw std::runtime_error("CallbackQuery provided by script " + commandName + " is not a function!");
    }

    auto result = command(callbackData);

    if(!result.valid()) {
        throw std::runtime_error("CallbackQuery provided by " + commandName + " called with failure: " + result.get<sol::error>().what());
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
