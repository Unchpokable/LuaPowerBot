#include "editor.hxx"

#include "thirdparty/tracy/tracy/Tracy.hpp"

#include "GLFW/glfw3.h"

#include "thirdparty/imgui-docking/imgui.h"
#include "thirdparty/imgui-docking/misc/cpp/imgui_stdlib.h"
#include "thirdparty/imgui-docking/backends/imgui_impl_glfw.h"
#include "thirdparty/imgui-docking/backends/imgui_impl_opengl3.h"

#include "configs.hxx"
#include "logdef.hxx"

#include "bot_workbench.hxx"
#include "code_editor.hxx"

#include "modals.hxx"

#include "ui_state.hxx"

#pragma comment(lib, "opengl32.lib")

namespace editor::internal {

namespace data {

using frame_duration = std::chrono::duration<double, std::nano>;
using time_point = std::chrono::steady_clock::time_point;

configs::Vec2Int window_geometry;

ImGuiID dockspace_id { 0 };
bool is_first_frame { false };

ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

// window framerate limiting by event-or-timeout things
frame_duration unlocked_time_remaining;
frame_duration frametime_limit;
std::condition_variable sleep_condition;
std::condition_variable wakeup_condition;
std::mutex sleep_mutex;
std::mutex wakeup_mutex;
std::jthread framerate_limiter_thread;
std::atomic_bool should_sleep;
std::atomic_bool should_wakeup;

std::atomic<float> target_fps;
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
    state::is_maximized = maximized == GLFW_TRUE;
}

void framerate_limiter_thread(const std::stop_token& stop_token)
{
    constexpr data::frame_duration second = data::frame_duration(1.0e9f);
    data::frame_duration passed_time {};
    data::time_point start_time {}, end_time {};

    while(!stop_token.stop_requested()) {
        auto iteration_time = end_time - start_time;
        start_time = std::chrono::steady_clock::now();

        float target_fps = data::target_fps.load(std::memory_order_acquire);

        passed_time += iteration_time;

        data::frame_duration requested_frame_time = {};

        {
            std::scoped_lock lock(data::sleep_mutex);

            if(data::unlocked_time_remaining > std::chrono::nanoseconds(0)) {
                data::unlocked_time_remaining -= iteration_time;
            } else {
                target_fps = 5;
            }

            // magic division by 1.3 - quite less frametime per frame to let ImGui, GLFW and others do they work
            requested_frame_time = (second / target_fps) / 1.3;

            if(passed_time >= requested_frame_time) {
                data::should_sleep = true;
                data::sleep_condition.notify_all();

                passed_time = {};
            }
        }

        {
            std::unique_lock lock(data::wakeup_mutex);
            data::wakeup_condition.wait_for(lock, requested_frame_time, [&] {
                return data::should_wakeup || stop_token.stop_requested();
            });
            data::should_wakeup.store(false, std::memory_order_release);
        }

        end_time = std::chrono::steady_clock::now();
    }
}

void force_new_frame()
{
    std::scoped_lock lock(data::wakeup_mutex);
    data::should_wakeup.store(true, std::memory_order_release);
    data::wakeup_condition.notify_all();
}

void unlock_frame_rate()
{
    {
        std::scoped_lock lock(data::wakeup_mutex);
        data::unlocked_time_remaining = std::chrono::seconds(1);
    }
    force_new_frame();
}

bool any_modifier_key_down()
{
    return ImGui::IsKeyPressed(ImGuiKey_LeftCtrl) || ImGui::IsKeyPressed(ImGuiKey_RightCtrl) ||
        ImGui::IsKeyPressed(ImGuiKey_LeftShift) || ImGui::IsKeyPressed(ImGuiKey_RightShift) ||
        ImGui::IsKeyPressed(ImGuiKey_LeftSuper) || ImGui::IsKeyPressed(ImGuiKey_RightSuper) ||
        ImGui::IsKeyPressed(ImGuiKey_LeftAlt) || ImGui::IsKeyPressed(ImGuiKey_RightAlt);
}

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

    return window;
}

void initialize_imgui(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    
    ImGui::CreateContext();

    auto &io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL3_Init("#version 130");
}

void init_modules()
{
    workbench::init();
}

void render_bot_page()
{
    workbench::render();
}

void render_text_editor()
{
    code::render();
}

void render_output_console()
{
    ImGui::Begin("Output");

    ImGui::End();
}

void render_gui(GLFWwindow* window)
{
    ZoneScoped;
    state::handle_keyboard();

    if(modals::has_any_modal()) {
        modals::render_from_top();
    }

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

    auto geometry = configs::get<configs::Vec2Int>("Geometry");
    auto framerate = *configs::get<int>("Framerate");
    auto framerate_limit_bound = *configs::get<int>("Framerate_limiting_bound");

    internal::data::target_fps.store(static_cast<float>(framerate), std::memory_order_release);

    auto window = internal::make_window("Lua!Power Bot Editor", { geometry->x, geometry->y });
    glfwMakeContextCurrent(window);

    glfwSetWindowFocusCallback(window, [](GLFWwindow* window, int focused) {
        ImGui_ImplGlfw_WindowFocusCallback(window, focused);
        internal::unlock_frame_rate();
    });

    glfwSetCursorEnterCallback(window, [](GLFWwindow* window, int entered) {
        ImGui_ImplGlfw_CursorEnterCallback(window, entered);
        internal::unlock_frame_rate();
    });

    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
        ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
        internal::unlock_frame_rate();
    });

    glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
        ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
        internal::unlock_frame_rate();
    });

    glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
        ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
        internal::unlock_frame_rate();
    });

    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
        internal::unlock_frame_rate();
    });

    glfwSetCharCallback(window, [](GLFWwindow* window, unsigned int codepoint) {
        ImGui_ImplGlfw_CharCallback(window, codepoint);
        internal::unlock_frame_rate();
    });

    glfwSetMonitorCallback([](GLFWmonitor* monitor, int event) {
        ImGui_ImplGlfw_MonitorCallback(monitor, event);
        internal::unlock_frame_rate();
    });

    //glfwSwapInterval(1);

    internal::initialize_imgui(window);
    internal::init_modules();

    internal::data::framerate_limiter_thread = std::jthread(internal::framerate_limiter_thread);

    while(!glfwWindowShouldClose(window)) {
        if(glfwGetWindowAttrib(window, GLFW_ICONIFIED) || !glfwGetWindowAttrib(window, GLFW_VISIBLE)) {
            glfwWaitEvents();
        }

        {
            while(true) {
                glfwPollEvents();

                if(internal::data::target_fps >= framerate_limit_bound) {
                    break;
                }

                {
                    std::unique_lock lock(internal::data::sleep_mutex);
                    internal::data::sleep_condition.wait(lock);

                    if(internal::data::should_sleep.exchange(false)) {
                        break;
                    }
                }
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //ImGui::Text("%f", ImGui::GetIO().Framerate);

        auto viewport = ImGui::GetMainViewport();
        ImGui::DockSpaceOverViewport(0, viewport, ImGuiDockNodeFlags_PassthruCentralNode);

        internal::render_gui(window);

        ImGui::Render();

        glfwGetFramebufferSize(window, &internal::data::window_geometry.x, &internal::data::window_geometry.y);

        glViewport(0, 0, internal::data::window_geometry.x, internal::data::window_geometry.y);

        auto color = internal::data::clear_color;
        glClearColor(color.x, color.y, color.z, color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        FrameMark;

        if(ImGui::IsAnyMouseDown()) {
            internal::unlock_frame_rate();
        }

        if(internal::any_modifier_key_down()) {
            internal::unlock_frame_rate();
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

