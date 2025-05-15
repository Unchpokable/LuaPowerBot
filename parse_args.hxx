#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>

namespace cmd {

using ArgsOption = std::variant<std::string, std::pair<std::string, std::string>>;

struct CmdArgs;

namespace env {

extern CmdArgs args;

bool empty();
std::optional<ArgsOption> get(std::string_view arg);
bool contains(std::string_view arg);

}

struct CmdArgs
{
    std::unordered_set<std::string> args;
    std::unordered_map<std::string, std::string> namedArgs;
};

bool empty(const CmdArgs& args);
std::optional<ArgsOption> get(const CmdArgs& args, std::string_view arg);
bool contains(const CmdArgs& args, std::string_view arg);

CmdArgs parse_arguments(int argc, char** argv);

}