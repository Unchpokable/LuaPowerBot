#include "bot_runtime.hxx"

#include <ranges>

#include "lua_load.hxx"

#include "logdef.hxx"

constexpr static std::chrono::seconds ActivityTimeout { 60 };

std::unique_ptr<tg::BotRuntime> tg::BotRuntime::create(const std::string& apiKey, const std::string& commandsPath)
{
    auto context = std::make_unique<BotRuntime>(apiKey);
    auto commandsBytecodeMap = lua::load_bytecode_map(commandsPath);

    if(commandsBytecodeMap) {
        context->_bytecode = commandsBytecodeMap.value();
    } else {
        luabot_logFatal("Bytecode map loading failed: {}", commandsBytecodeMap.error().message());
    }

    return context;
}

tg::BotRuntime::BotRuntime(const std::string& apiKey)
{
    _bot = std::make_unique<TgBot::Bot>(apiKey);
}

void tg::BotRuntime::initNewSession(std::uint64_t chatId)
{
}

void tg::BotRuntime::verifySessions()
{
    auto clock_now = std::chrono::high_resolution_clock::now();

    for(auto &sessionPtr : _activeSessions | std::views::values) {
        if(sessionPtr->lastActivity() - clock_now > ActivityTimeout) {
            sessionPtr->forceClose();
        }
    }
}
