#include "bot_runtime.hxx"

#include "lua_load.hxx"

std::unique_ptr<tg::BotRuntimeContext> tg::BotRuntimeContext::create(const std::string& apiKey, const std::string& commandsPath)
{
    auto context = std::make_unique<BotRuntimeContext>(apiKey);

    return context;
}

tg::BotRuntimeContext::BotRuntimeContext(const std::string& apiKey)
{
    _bot = std::make_unique<TgBot::Bot>(apiKey);
}
