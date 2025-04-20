#include "editor.hxx"

#include "GLFW/glfw3.h"

#include "thirdparty/imgui-docking/imgui.h"
#include "thirdparty/imgui-docking/backends/imgui_impl_glfw.h"
#include "thirdparty/imgui-docking/backends/imgui_impl_opengl3.h"
#include "thirdparty/imgui-docking/backends/imgui_impl_win32.h"

#include "configs.hxx"

namespace editor::internal {

GLFWwindow* make_window(const char* title, configs::Vec2Int geometry)
{
    auto monitor = glfwGetPrimaryMonitor();

    auto window = glfwCreateWindow(geometry.x, geometry.y, title, monitor, nullptr);

    return window;
}

}


void editor::open_gui()
{
    glfwInit();

    auto wnd = internal::make_window("Lua!Power Bot Editor", { 640, 480 });
}

