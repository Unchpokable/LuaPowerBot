#include "strings.hxx"

std::vector<std::string> utils::string_split(std::string &source, char sep)
{
    return string_split(source, std::string { sep });
}

std::vector<std::string> utils::string_split(std::string &source, const std::string &sep)
{
    std::vector<std::string> output;
    auto view = std::views::split(source, sep);
    for(auto subrange : view) {
        output.emplace_back(subrange.begin(), subrange.end());
    }
    return output;
}
