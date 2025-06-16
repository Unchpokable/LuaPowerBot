#pragma once
#include "zip2memvfs.hxx"

namespace editor::code
{

void init();

void assign_filesystem(files::IFileSystem filesystem);
void open_file(std::string_view file_name);

void refresh();
void write_cached();

void render();

}
