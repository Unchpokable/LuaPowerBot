#pragma once
#include "zip2memvfs.hxx"

namespace editor::code
{

void assign_filesystem(files::IFileSystem filesystem);
void open_file(std::string_view file_name);

void actualize();
void write_cached();

void render();

}
