#include <string>

#include "bot_runtime.hxx"
#include "zip2memvfs.hxx"

#include "bot_workbench.hxx"

#include <ranges>

#include "logdef.hxx"

#include "thirdparty/imgui-docking/imgui.h"
#include "thirdparty/imgui-docking/misc/cpp/imgui_stdlib.h"

#include "code_editor.hxx"
#include "editor.hxx"
#include "editor_utils.hxx"
#include "modals.hxx"
#include "ui_state.hxx"

namespace editor::workbench::data {

constexpr const char* command_template = R"(return function()
    return {
        -- place your data here
        -- do not use hardcoded data - it can be lost due to bytecode serialization
        
        -- do not remove or rename these functions! This is a public API functions that will be called by a runtime when
        -- on_start will be executed once when new chat session is created
        on_start = function() 
            -- implement your startup routine here
        end;

        -- on_message will be called every time when bot receives a message
        on_message = function(chat_id, message) 
            -- implement your action on message here
        end;

        -- on_callback will be called every time when bot receives a callback query (InlineKeyboard handling, etc.)
        on_callback = function(chat_id, callback_query) 
            -- implement your action to callback here
        end;
    }
end)";

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

// visual things
std::string_view selected_file;
std::string opened_project_name;

}

namespace editor::workbench::internal {

void bot_runtime_thread(const std::stop_token &token) {
    while(!token.stop_requested()) {
        data::bot_runtime->poll_and_dispatch();

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

void on_create_file(const std::any& handler_arg)
{
    auto string = std::any_cast<std::string>(handler_arg);
    auto full_name = std::format("scripts/{}.lua", string);
    if(!data::project_files || !data::project_files->IsInitialized()) {
        modals::inform("Unable to add command", "To add a command, create or open a project", false);
        return;
    }

    data::project_files->CreateFile(vfspp::FileInfo(full_name));

    auto result = files::write_text(data::project_files, full_name, data::command_template);

    if(result != errors::OK) {
        std::string message = std::format("Unable to write a file: {}, error code: {}", full_name, static_cast<std::uint16_t>(result));
        modals::inform("File writing error!", message, false);
    }

    refresh_scripts();
}

void handle_keyboard()
{
    if(ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_N)) {
        modals::ask_input("New command", "Provide a command name:", true)
            ->on(modals::ModalEvent::Ok, on_create_file);
    }
}

}

void editor::workbench::open_project_file(const std::string& file)
{
    auto result = files::open_zip(file);

    if(!result) {
        luabot_logErr("Unable to open file {}, whats happened: {}", file, result.error().message());
        return;
    }

    modals::inform("Project opened!", std::format("successfully opened: {}", file), false);

    data::project_files = std::move(result.value());

    data::opened_project_name = fs::path(file).filename().string();

    refresh_scripts();
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

    for(const auto& file : file_list | std::views::values) {
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

    for(auto &files : data::files_by_folders | std::views::values) {
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
    ImGui::Begin("Bot workbench");

    internal::handle_keyboard();

    static std::string project_path = "No project opened";
    if(data::project_files && data::project_files->IsInitialized()) {
        project_path = "Opened: " + data::opened_project_name;
    }

    ImGui::TextWrapped("%s", project_path.c_str());
    ImGui::Separator();

    if(ImGui::Button("Open new project")) {
        modals::ask_open("Open new project", {}, "*.zip", true)
            ->on(modals::ModalEvent::Ok, [](const std::any& path) {
                open_project_file(std::any_cast<std::string>(path));
            });
    }

    ImGui::BeginChild("Files", ImVec2(0, ImGui::GetContentRegionAvail().y - 100), true);

    if(data::files_by_folders.contains("/")) {
        if(ImGui::TreeNodeEx("/ (Root)", ImGuiTreeNodeFlags_DefaultOpen)) {
            for(const auto& file: data::files_by_folders["/"]) {
                auto is_selected = ImGui::Selectable(file.c_str(), file == data::selected_file);

                if(is_selected) {
                    data::selected_file = file;

                    if(ImGui::IsMouseDoubleClicked(0)) {
                        code::open_file(file);
                    }
                }
            }

            ImGui::TreePop();
        }
    }

    for(const auto& dir : data::folders) {
        if(dir == "/") continue;

        if(ImGui::TreeNodeEx(dir.c_str(), 0)) {
            if(data::files_by_folders.contains(dir)) {
                for(const auto& file : data::files_by_folders[dir]) {
                    if(file.empty()) { // SOMEWHY AND SOMEHOW WE GOT A FILE WITHOUT NAME
                        continue;
                    }

                    bool is_selected = ImGui::Selectable(file.c_str(), file == data::selected_file);

                    if(is_selected) {
                        data::selected_file = file;
                        if(ImGui::IsMouseDoubleClicked(0)) {
                            std::string full_path = dir + file;
                            code::open_file(full_path);
                        }
                    }
                }
            }
            ImGui::TreePop();
        }
    }

    ImGui::EndChild();

    ImGui::Separator();

    ImGui::InputText("API Key", &data::api_key, ImGuiInputTextFlags_Password);

    ImGui::Text("Bot status: ");
    ImGui::SameLine();

    bool is_bot_running = (data::bot_runtime != nullptr);
    ImVec4 status_color = is_bot_running ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    std::string status_text = is_bot_running ? "ONLINE" : "OFFLINE";

    ImGui::TextColored(status_color, "%s", status_text.c_str());
    ImGui::SameLine();

    float circle_radius = ImGui::GetTextLineHeight() * 0.5f;
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddCircleFilled(
        ImVec2(p.x + circle_radius, p.y + circle_radius), 
        circle_radius, 
        ImGui::ColorConvertFloat4ToU32(status_color));
    ImGui::Dummy(ImVec2(circle_radius * 2 + 4, circle_radius * 2));

    if(!is_bot_running) {
        if(ImGui::Button("Start Bot")) {
            if(!data::api_key.empty()) {
                start_bot();
            } else {
                //utils::show_notification("Error", "API key is required to start the bot", true);
            }
        }
    } else {
        if(ImGui::Button("Stop Bot")) {
            stop_bot();
        }
    }

    if(ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S)) {
        save();
    }

    ImGui::End();
}
