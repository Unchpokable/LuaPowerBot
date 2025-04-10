#pragma once

#include <tgbot/tgbot.h>
#include <queue>

namespace tg {

struct BotConfig {
    
};

class BotRuntimeContext {

public:

private:
    std::mutex _mutex;
    std::thread _botThread;
};

}