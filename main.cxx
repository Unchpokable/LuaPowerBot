#include "configs.hxx"
#include "parse_args.hxx"

#include "editor.hxx"

int main(int argc, char** argv)
{
    auto args = cmd::parse_arguments(argc, argv);

    configs::load_from_file("C:\\Users\\Unchp\\source\\repos\\LuaPowerBot\\x64\\Debug\\config.json");

    if(cmd::empty(args) || cmd::get(args, "gui")) {
        editor::open_gui();
    }

    // todo: run in CLI mode

    return 0;
}
