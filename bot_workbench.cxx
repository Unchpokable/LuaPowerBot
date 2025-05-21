#include <string>

#include "bot_runtime.hxx"
#include "zip2memvfs.hxx"

#include "bot_workbench.hxx"

#include "logdef.hxx"

#include "thirdparty/imgui-docking/imgui.h"
#include "thirdparty/imgui-docking/misc/cpp/imgui_stdlib.h"

#include "code_editor.hxx"
#include "editor.hxx"
#include "editor_utils.hxx"
#include "ui_state.hxx"

namespace editor::workbench::data {

files::ZipFS project_files;

std::unique_ptr<tg::BotRuntime> bot_runtime;

std::string api_key;

std::vector<std::string> folders;
std::map<std::string, std::vector<std::string>> files_by_folders;

// synchronization and threading
std::mutex mutex;
std::condition_variable condition;

std::jthread bot_runtime_thread;

using Task = std::function<void()>;
std::queue<Task> bot_runtime_tasks_queue;

}

namespace editor::workbench::internal {

void bot_runtime_thread(const std::stop_token &token) {
    while(!token.stop_requested()) {
        data::bot_runtime->pollAndDispatch();

        data::Task task;
        {
            std::unique_lock lock(data::mutex);
            data::condition.wait(lock, [&] {
                return !data::bot_runtime_tasks_queue.empty() || token.stop_requested();
            });

            if(token.stop_requested() && data::bot_runtime_tasks_queue.empty()) {
                break;
            }

            if(!data::bot_runtime_tasks_queue.empty()) {
                task = std::move(data::bot_runtime_tasks_queue.front());
                data::bot_runtime_tasks_queue.pop();
            }
        }

        if(task) {
            task();
        }
    }
}

void bot_runtime_enqueue_task(data::Task task)
{
    {
        std::unique_lock lock(data::mutex);
        data::bot_runtime_tasks_queue.push(std::move(task));
    }

    data::condition.notify_one();
}

}

void editor::workbench::open_project_file(const std::string& file)
{
    auto result = files::open_zip(file);

    if(!result) {
        luabot_logErr("Unable to open file {}, whats happened: {}", file, result.error().message());
        return;
    }

    data::project_files = std::move(result.value());
}

void editor::workbench::refresh_scripts()
{
    data::folders.clear();
    data::files_by_folders.clear();

    data::files_by_folders["/"] = {};

    if(!data::project_files || !data::project_files->IsInitialized()) {
        luabot_logWarn("Project does not exists or opened with errors. Reopen project and try again");
        return;
    }

    code::actualize();

    auto &file_list = data::project_files->FileList();

    for(const auto& [path, file] : file_list) {
        const auto& file_info = file->GetFileInfo();
        std::string abs_path = file_info.AbsolutePath();

        if(file_info.IsDir()) {
            continue;
        }

        std::filesystem::path fs_path(abs_path);
        std::filesystem::path parent_path = fs_path.parent_path();
        std::string parent_str = parent_path.string();

        if(parent_str.empty() || parent_str == "/") {
            data::files_by_folders["/"].push_back(file_info.Name());
            continue;
        }

        if(parent_str.back() != '/') {
            parent_str += '/';
        }

        auto dir_it = std::ranges::find(data::folders, parent_str);
        if(dir_it == data::folders.end()) {
            data::folders.push_back(parent_str);

            data::files_by_folders[parent_str] = {};
        }

        data::files_by_folders[parent_str].push_back(file_info.Name());
    }

    std::ranges::sort(data::folders);

    for(auto& [dir, files] : data::files_by_folders) {
        std::ranges::sort(files);
    }
}

void editor::workbench::save()
{
    code::write_cached();
}

void editor::workbench::start_bot()
{
}

void editor::workbench::stop_bot()
{
}

void editor::workbench::render()
{
}
