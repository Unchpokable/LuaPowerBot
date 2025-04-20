#pragma once

#include <string>

namespace configs {

struct Vec2Int
{
    int x, y;
};

void set_working_file(const std::string& file);
void save();
void load();

}