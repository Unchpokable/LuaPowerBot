#pragma once

#include <queue>

#include <tgbot/tgbot.h>

#include "user_session.hxx"

#include "globals.hxx"

namespace lua {
struct LuaScript;
}

namespace tg {

class BotRuntimeContext
{
public:
    static std::unique_ptr<BotRuntimeContext> create(const std::string& apiKey, const std::string& commandsPath);

    BotRuntimeContext(const std::string& apiKey);

private:
    void verifySessions();

    std::unique_ptr<TgBot::Bot> _bot { nullptr };
    std::unordered_map<std::uint64_t, std::unique_ptr<UserSession>> _activeSessions;
    std::queue<std::uint64_t> _enqueuedClients;

    std::thread _thread;
    std::mutex _mutex;

    std::vector<uint64_t> _trustedUsers;
};

}
