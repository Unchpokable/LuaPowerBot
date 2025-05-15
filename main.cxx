#include "configs.hxx"
#include "parse_args.hxx"

#include "editor.hxx"

std::string get_executable_path();

#ifdef _WIN32

#include "Windows.h"

std::string get_executable_path()
{
    char buffer[MAX_PATH]; // MAX_PATH
    GetModuleFileNameA(nullptr, buffer, MAX_PATH);

    std::string::size_type pos = std::string(buffer).find_last_of("\\/");
    return std::string(buffer).substr(0, pos + 1);
}

#endif

int main(int argc, char** argv)
{
    std::ignore = cmd::parse_arguments(argc, argv);

    configs::load_from_file(get_executable_path() + "config.json");

    if(cmd::env::empty() || cmd::env::get("gui")) {
        editor::open_gui();
    }

    // todo: run in CLI mode

    return 0;
}
