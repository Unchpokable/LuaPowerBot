#pragma once

#include <queue>

#include <tgbot/tgbot.h>

#include "user_session.hxx"

#include "globals.hxx"

namespace tg {

class BotRuntime
{
public:
    static std::unique_ptr<BotRuntime> create(const std::string& apiKey, const std::string& commandsPath);

    BotRuntime(const std::string& apiKey);

private:
    void verifySessions();

    std::unique_ptr<TgBot::Bot> _bot { nullptr };
    std::unordered_map<std::uint64_t, std::unique_ptr<UserSession>> _activeSessions;
    std::queue<std::uint64_t> _enqueuedClients;

    std::vector<uint64_t> _trustedUsers;

    BytecodeMap _bytecode;
};

}
