#pragma once

#include <vector>
#include <string>
#include <ranges>
#include <algorithm>

namespace utils {

std::vector<std::string> string_split(std::string& source, char sep);
std::vector<std::string> string_split(std::string& source, const std::string &sep);

}