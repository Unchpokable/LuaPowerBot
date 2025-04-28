#include "parse_args.hxx"

#include "editor.hxx"

int main(int argc, char** argv)
{
    auto args = cmd::parse_arguments(argc, argv);

    if(cmd::empty(args) || cmd::get(args, "gui")) {
        editor::open_gui();
    }

    // todo: run in CLI mode

    return 0;
}
