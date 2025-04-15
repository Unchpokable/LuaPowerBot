#pragma once

#include "logging.hxx"

#define luabot_logInternal(lvl, fmt, ...) \
    ::logging::log((lvl), std::source_location::current(),(fmt), ##__VA_ARGS__)

#define luabot_logInfo(...) \
    luabot_logInternal(::logging::Level::Info, __VA_ARGS__)

#define luabot_logWarn(...) \
    luabot_logInternal(::logging::Level::Warn, __VA_ARGS__)

#define luabot_logErr(...) \
    luabot_logInternal(::logging::Level::Error, __VA_ARGS__)

#define luabot_logFatal(...) \
    luabot_logInternal(::logging::Level::Fatal, __VA_ARGS__)