#include "parse_args.hxx"

int main(int argc, char** argv)
{
    auto args = cmd::parse_arguments(argc, argv);

    if(cmd::empty(args)) {
        
    }

    return 0;
}
