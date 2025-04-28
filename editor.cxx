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

WNDPROC old_windowproc = nullptr;

#endif

configs::Vec2Int window_geometry;
bool is_maximized = false;

ImGuiID dockspace_id { 0 };
bool is_first_frame { false };

ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

}

void error_callback(int error, const char* description)
{
    luabot_logErr("GLFW Error {}: {}", error, description);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    data::window_geometry = { width, height };
    glViewport(0, 0, width, height);
}

void window_maximize_callback(GLFWwindow* window, int maximized)
{
    data::is_maximized = maximized == GLFW_TRUE;
}

#ifdef _WIN32

LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    if(ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
        return true;

    return CallWindowProc(data::old_windowproc, hwnd, msg, wparam, lparam);
}

void setup_win32_handling(GLFWwindow* window)
{
    HWND hwnd = glfwGetWin32Window(window);

    if(hwnd != nullptr) {
        WNDPROC prev_wndproc = reinterpret_cast<WNDPROC>(GetWindowLongPtr(hwnd, GWLP_WNDPROC));

        data::old_windowproc = prev_wndproc;

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

    data::window_geometry = geometry;

#ifdef _WIN32
    setup_win32_handling(window);
#endif

    return window;
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

void render_bot_page()
{
    ImGui::Begin("Bot Workbench");

    ImGui::End();
}

void render_text_editor()
{
    ImGui::Begin("Editor");

    ImGui::End();
}

void render_output_console()
{
    ImGui::Begin("Output");

    ImGui::End();
}

void render_gui(GLFWwindow* window)
{
    render_bot_page();
    render_text_editor();
    render_output_console();
}

}

void editor::set_window_background_color(float r, float g, float b, float alpha)
{
    internal::data::clear_color = ImVec4(r, g, b, alpha);
}

void editor::open_gui()
{
    if(!glfwInit()) {
        luabot_logFatal("Unable to initialize GLFW!");
        return;
    }

    auto window = internal::make_window("Lua!Power Bot Editor", { 640, 480 });
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    internal::initialize_imgui(window);

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if(glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        auto viewport = ImGui::GetMainViewport();
        ImGui::DockSpaceOverViewport(0, viewport, ImGuiDockNodeFlags_PassthruCentralNode);

        internal::render_gui(window);

        ImGui::Render();

        glfwGetFramebufferSize(window, &internal::data::window_geometry.x, &internal::data::window_geometry.y);

        glViewport(0, 0, internal::data::window_geometry.x, internal::data::window_geometry.y);

        auto color = internal::data::clear_color;
        glClearColor(color.x * color.w, color.y * color.w, color.z * color.w, color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

