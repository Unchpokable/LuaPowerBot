#include "editor.hxx"

#include "GLFW/glfw3.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h" 

#include "thirdparty/imgui-docking/imgui.h"
#include "thirdparty/imgui-docking/backends/imgui_impl_glfw.h"
#include "thirdparty/imgui-docking/backends/imgui_impl_opengl3.h"
#include "thirdparty/imgui-docking/backends/imgui_impl_win32.h"

#include "configs.hxx"
#include "logdef.hxx"

#pragma comment(lib, "opengl32.lib")

#ifdef _WIN32
#include <windows.h>
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif

namespace editor::internal {

namespace data {

#ifdef _WIN32

WNDPROC g_wndproc = nullptr;

#endif

configs::Vec2Int g_wnd_geometry;
bool g_is_maximized = false;

}

void error_callback(int error, const char* description)
{
    luabot_logErr("GLFW Error {}: {}", error, description);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    data::g_wnd_geometry = { width, height };
    glViewport(0, 0, width, height);
}

void window_maximize_callback(GLFWwindow* window, int maximized)
{
    data::g_is_maximized = maximized == GLFW_TRUE;
}

#ifdef _WIN32

LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    if(ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
        return true;

    return CallWindowProc(data::g_wndproc, hwnd, msg, wparam, lparam);
}

void setup_win32_handling(GLFWwindow* window)
{
    HWND hwnd = glfwGetWin32Window(window);

    if(hwnd != nullptr) {
        WNDPROC prev_wndproc = reinterpret_cast<WNDPROC>(GetWindowLongPtr(hwnd, GWLP_WNDPROC));

        data::g_wndproc = prev_wndproc;

        SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(wnd_proc));
    }
}

#endif

GLFWwindow* make_window(const char* title, configs::Vec2Int geometry)
{
    glfwSetErrorCallback(error_callback);

    if(!glfwInit()) {
        luabot_logFatal("Failed to initialize GLFW");
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(
        geometry.x, geometry.y,
        title, nullptr, nullptr
    );

    if(window == nullptr) {
        luabot_logFatal("Failed to create GLFW window");
        glfwTerminate();
        return nullptr;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetWindowMaximizeCallback(window, window_maximize_callback);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); 

    data::g_wnd_geometry = geometry;

#ifdef _WIN32
    setup_win32_handling(window);
#endif

    return window;
}

}

void initialize_imgui(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    auto &io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

void editor::open_gui()
{
    glfwInit();

    auto wnd = internal::make_window("Lua!Power Bot Editor", { 640, 480 });
}

