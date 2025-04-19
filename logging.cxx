#include "logging.hxx"

logging::LogSink sink = [](std::string_view msg) {
    std::cerr << msg;
};
