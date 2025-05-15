#include "parse_args.hxx"

#include <ranges>

cmd::CmdArgs cmd::env::args = CmdArgs();

std::string remove_quotes(const std::string& str)
{
    if(str.size() >= 2 && str.front() == '"' && str.back() == '"') {
        return str.substr(1, str.size() - 2);
    }
    return str;
}

bool cmd::env::empty() {
    return cmd::empty(cmd::env::args);
}

std::optional<cmd::ArgsOption> cmd::env::get(std::string_view arg) {
    return cmd::get(cmd::env::args, arg);
}

bool cmd::env::contains(std::string_view arg) {
    return cmd::contains(cmd::env::args, arg);
}

bool cmd::empty(const CmdArgs& args) {
    return args.args.empty() && args.namedArgs.empty();
}

std::optional<cmd::ArgsOption> cmd::get(const CmdArgs& args, std::string_view arg)
{
    auto single_arg = std::ranges::find(args.args, arg);

    if(single_arg != args.args.end()) {
        return *single_arg;
    }

    auto arg_pair = args.namedArgs.find(std::string(arg));

    if(arg_pair != args.namedArgs.end()) {
        return *arg_pair;
    }

    return std::nullopt;
}

bool cmd::contains(const CmdArgs& args, std::string_view arg)
{
    auto get_result = get(args, arg);

    return get_result.has_value();
}

cmd::CmdArgs cmd::parse_arguments(int argc, char **argv)
{
    CmdArgs args;

    if(argc == 1) { // only exe name given
        return args;
    }
    
    for(std::ptrdiff_t i = 0; i < argc; i++) {
        std::string arg(argv[i]);

        if(!arg.empty() && arg[0] == '-') {
            if(!arg.empty() && arg[0] == '-') {
                int dashCount = (arg.size() >= 2 && arg[1] == '-') ? 2 : 1;
                std::string trimmed = arg.substr(dashCount);
                size_t pos = trimmed.find('=');
                if(pos != std::string::npos) {
                    std::string key = trimmed.substr(0, pos);
                    std::string value = trimmed.substr(pos + 1);
                    value = remove_quotes(value);
                    args.namedArgs[key] = value;
                } else {
                    args.args.insert(trimmed);
                }
            }
        }
    }

    env::args = args;

    return args;
}
