#include <string>
#include <ranges>

#include "code_editor.hxx"

#include "zip2memvfs.hxx"

#include "thirdparty/imtextedit/TextEditor.h"

namespace editor::code::data
{

files::IFileSystem working_filesystem;
std::map<std::string, vfspp::IFilePtr, std::less<>> opened_files;

std::map<std::string, std::string> edited_files;
vfspp::IFilePtr working_file;

TextEditor editor;

}

void editor::code::init()
{
    auto editor_lang = TextEditor::LanguageDefinition::Lua();

    // these functions are public API calls so they are more likely are keywords rather than a just names
    editor_lang.mKeywords.insert("on_start");
    editor_lang.mKeywords.insert("on_message");
    editor_lang.mKeywords.insert("on_callback");

    data::editor.SetLanguageDefinition(editor_lang);
}

void editor::code::assign_filesystem(files::IFileSystem filesystem)
{
    data::working_filesystem = std::move(filesystem);
    refresh();
}

void editor::code::open_file(std::string_view file_name)
{
    if(auto opened_file = data::opened_files.find(file_name); opened_file != data::opened_files.end()) {
        
    }
}

void editor::code::refresh()
{
    for(auto& file: data::opened_files | std::views::values) {
        file->Close();
    }


}

void editor::code::write_cached()
{
}

void editor::code::render()
{
    ImGui::Begin("Editor");

    data::editor.Render(data::working_file->GetFileInfo().Name().c_str());

    ImGui::End();
}
