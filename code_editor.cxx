#include <string>

#include "code_editor.hxx"

#include "zip2memvfs.hxx"

#include "thirdparty/imtextedit/TextEditor.h"

namespace editor::code::data
{

files::ZipFS working_filesystem;
vfspp::ZipFilePtr working_file;
std::unordered_map<std::string, vfspp::ZipFilePtr> opened_files;
std::unordered_map<std::string, std::string> updated_files;
std::string current_file_text;

}

void editor::code::assign_filesystem(files::ZipFS filesystem)
{
    data::working_filesystem = std::move(filesystem);
}

void editor::code::open_file(std::string_view file_name)
{
}

void editor::code::actualize()
{
}

void editor::code::write_cached()
{
}

void editor::code::render()
{
    ImGui::Begin("Editor");

    ImGui::End();
}
