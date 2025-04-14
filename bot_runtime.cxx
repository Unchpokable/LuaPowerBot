#include "bot_runtime.hxx"

#include <ranges>

#include "lua_load.hxx"

constexpr static std::chrono::seconds ActivityTimeout { 60 };

std::unique_ptr<tg::BotRuntime> tg::BotRuntime::create(const std::string& apiKey, const std::string& commandsPath)
{
    auto context = std::make_unique<BotRuntime>(apiKey);

    return context;
}

tg::BotRuntime::BotRuntime(const std::string& apiKey)
{
    _bot = std::make_unique<TgBot::Bot>(apiKey);
}


void tg::BotRuntime::verifySessions()
{
    auto clock_now = std::chrono::high_resolution_clock::now();

    for(auto sessionPtr : _activeSessions | std::views::values) {
        if(sessionPtr->lastActivity() - clock_now > ActivityTimeout) {
            sessionPtr->forceClose();
        }
    }
}
