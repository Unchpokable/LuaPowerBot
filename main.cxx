#include <filesystem>

#include "configs.hxx"
#include "parse_args.hxx"

#include "editor.hxx"
#include "logdef.hxx"

std::filesystem::path get_current_path(char* argv[]) {
    std::filesystem::path exe_dir = std::filesystem::canonical(argv[0]).parent_path();

    return exe_dir;
}

int main(int argc, char** argv)
{
    std::ignore = cmd::parse_arguments(argc, argv);

    configs::load_from_file(get_current_path(argv) / "config.json");

    if(cmd::env::empty() || cmd::env::get("gui")) {
        editor::open_gui();
    }

    // todo: run in CLI mode

    return 0;
}
