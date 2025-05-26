#pragma once

#include <format>
#include <functional>
#include <iostream>
#include <source_location>

namespace logging {

using LogSink = std::function<void(std::string_view)>;

extern LogSink sink;

enum class Level {
    Info,
    Warn,
    Error,
    Fatal
};

constexpr const char* level_to_string(Level lvl) {
    switch(lvl)
    {
    case Level::Info:
        return "Info";
    case Level::Warn:
        return "Warn";
    case Level::Error:
        return "Error";
    case Level::Fatal:
        return "Fatal";
    default:
        return "UNKNOWN";
    }
}

inline void setLogOutput(LogSink newSink) {
    sink = std::move(newSink);
}

template<typename ...Args>
void log(Level lvl, std::source_location loc, std::string_view fmt, Args&&... args) {
    auto msg = std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...));
    auto full = std::format("[{}] {}:{} :: {}\n", level_to_string(lvl), loc.file_name(), loc.line(), msg);

    sink(full);
}

}
