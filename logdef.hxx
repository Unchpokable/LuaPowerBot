#pragma once

#include "logging.hxx"

#define luabot_logInternal(lvl, fmt, ...) \
    ::logging::log((lvl), std::source_location::current(), (fmt), ##__VA_ARGS__)

#define luabot_logInfo(fmt, ...) \
    luabot_logInternal(::logging::Level::Info, (fmt), ##__VA_ARGS__)

#define luabot_logWarn(fmt, ...) \
    luabot_logInternal(::logging::Level::Warn, (fmt), ##__VA_ARGS__)

#define luabot_logErr(fmt, ...) \
    luabot_logInternal(::logging::Level::Error, (fmt), ##__VA_ARGS__)

#define luabot_logFatal(fmt, ...) \
    luabot_logInternal(::logging::Level::Fatal, (fmt), ##__VA_ARGS__)