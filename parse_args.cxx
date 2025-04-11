#include "parse_args.hxx"

std::string remove_quotes(const std::string& str)
{
    if(str.size() >= 2 && str.front() == '"' && str.back() == '"') {
        return str.substr(1, str.size() - 2);
    }
    return str;
}

bool cmd::empty(const CmdArgs& args) {
    return args.args.empty() && args.namedArgs.empty();
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

    return args;
}
