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
    static std::unique_ptr<BotRuntime> create_from_project(const std::string& zip, const std::string& external_api_key = {});

    BotRuntime(const std::string& apiKey);

    void poll_and_dispatch();

private:
    void init_new_session(std::uint64_t chatId);

    void verify_sessions();

    std::unique_ptr<TgBot::Bot> _bot { nullptr };
    std::unordered_map<std::uint64_t, std::unique_ptr<UserSession>> _activeSessions;
    std::queue<std::uint64_t> _enqueuedClients;

    std::vector<uint64_t> _trustedUsers;

    BytecodeMap _bytecode;
};

}
