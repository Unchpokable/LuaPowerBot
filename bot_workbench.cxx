#include <string>

#include "bot_runtime.hxx"
#include "zip2memvfs.hxx"

#include "bot_workbench.hxx"

#include "logdef.hxx"

#include "thirdparty/imgui-docking/imgui.h"
#include "thirdparty/imgui-docking/misc/cpp/imgui_stdlib.h"

#include "editor.hxx"
#include "editor_utils.hxx"
#include "ui_state.hxx"

namespace editor::workbench::data {

files::ZipFS project_files;

std::unique_ptr<tg::BotRuntime> bot_runtime;

std::string api_key;

}

void editor::workbench::open_project_file(const std::string& file)
{
    auto result = files::open_zip(file);

    if(!result) {
        luabot_logErr("Unable to open file {}", file);
        return;
    }

    data::project_files = std::move(result.value());
}
