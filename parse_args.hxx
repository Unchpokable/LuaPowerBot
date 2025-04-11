#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace cmd {

struct CmdArgs
{
    std::unordered_set<std::string> args;
    std::unordered_map<std::string, std::string> namedArgs;
};


bool empty(const CmdArgs& args);
CmdArgs parse_arguments(int argc, char** argv);

}