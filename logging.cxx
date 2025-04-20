#include "logging.hxx"

logging::LogSink logging::sink = [](std::string_view msg) {
    std::cerr << msg;
};
