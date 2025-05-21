#include "bot_runtime.hxx"

#include <ranges>

#include "lua_load.hxx"

#include "logdef.hxx"
#include "security.hxx"

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

std::unique_ptr<tg::BotRuntime> tg::BotRuntime::createFromProject(const std::string& zip, const std::string& external_api_key)
{
    auto filesystem = files::open_zip(zip);
    if(!filesystem) {
        luabot_logFatal("Unable to open given ZIP file as a virtual file system: {}", zip);
        throw std::runtime_error("Virtual FS exception occured!");
    }

    auto commands = lua::load_bytecode_map(filesystem.value());

    if(!commands) {
        luabot_logErr("Unable to load scripts: {}", commands.error().message());
        return nullptr;
    }

    std::unique_ptr<BotRuntime> bot = { nullptr };

    if(!external_api_key.empty()) {
        bot = std::make_unique<BotRuntime>(external_api_key);
    } else {
        auto key = files::read_bytes(filesystem.value(), "credentials.bin");

        if(!key) {
            luabot_logErr("Unable to create a bot runtime from given project file - key corrupted");
            return nullptr;
        }

        auto decrypted = security::dpapi_decrypt(key.value());

        if(!decrypted) {
            luabot_logErr("Unable to create a bot runtime - no external key provided and saved project key will be saved by other user or on other PC");
            return nullptr;
        }

        auto decrypted_key = decrypted.value();

        bot = std::make_unique<BotRuntime>(std::string(decrypted_key.begin(), decrypted_key.end()));

        return bot;
    }

    return bot;
}

tg::BotRuntime::BotRuntime(const std::string& apiKey)
{
    _bot = std::make_unique<TgBot::Bot>(apiKey);
}

void tg::BotRuntime::pollAndDispatch()
{
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
