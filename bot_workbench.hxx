#pragma once

namespace editor::workbench {

void init();
void shutdown();

void open_file(std::string_view file);

void refresh_scripts();
void save();

void start_bot();
void stop_bot();

void render();

}