// ImGui - standalone example application for Glfw + OpenGL 3, using programmable pipeline
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
#include <unordered_map>
#include <imgui.h>
#include "imgui_impl_glfw_gl3.h"
#include <stdio.h>
#include <GL/gl3w.h>    // This example is using gl3w to access OpenGL functions (because it is small). You may use glew/glad/glLoadGen/etc. whatever already works for you.
#include <GLFW/glfw3.h>
#include <string>
#include "app_config.h"
#define WYC_LOG_IMPLEMENTATION
#include "console_log.h"
#define WYC_SHELLCMD_IMPLEMENTATION
#include "shellcmd.h"

// setup global application config
const char *AppConfig::app_name = "pixpad";
int AppConfig::window_width = 1280;
int AppConfig::window_height = 720;

bool console_register_command(wyc::IShellCommand *cmd);
void console_unregister_command(const std::string &cmd_name);
void show_console(void);
void show_image(const char *img_file);

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}

static void window_size_callback(GLFWwindow *window, int width, int height)
{
	AppConfig::window_width = width;
	AppConfig::window_height = height;
}

static bool init_process()
{
	HMODULE module = LoadLibrary("testbed");
	if (module == NULL) {
		log_error("fail to load library: testbed");
		return false;
	}
	typedef void(*PFN_SET_LOGGER)(wyc::ILogger*);
	PFN_SET_LOGGER set_logger = (PFN_SET_LOGGER)GetProcAddress(module, "set_logger");
	if (set_logger) {
		set_logger(LOGGER_GET(CConsoleLogger));
	}
	typedef wyc::IShellCommand* (*PFN_GET_COMMAND)();
	PFN_GET_COMMAND get_command = (PFN_GET_COMMAND)GetProcAddress(module, "get_command");
	if (get_command) {
		console_register_command(get_command());
	}
	else {
		log_error("module [testbed]: no get_command interface");
	}
	return true;
}

class CCommandExit : public wyc::CShellCommand
{
public:
	CCommandExit(GLFWwindow* window)
		: CShellCommand("exit", "Pixpad exit")
		, m_main_window(window)
	{
		m_opt.add_options()
			("help", "show help message");
	}
	virtual bool process(const po::variables_map &args) override
	{
		glfwSetWindowShouldClose(m_main_window, 1);
		return true;
	}
private:
	GLFWwindow* m_main_window;
};

class CCommandReload : public wyc::CShellCommand
{
public:
	CCommandReload()
		: CShellCommand("reload", "reload plugin")
	{
		m_opt.add_options()
			("help", "show help message")
			("name", po::value<std::string>(), "plugin name");
		m_pos_opt.add("name", 1);
	}
	virtual bool process(const po::variables_map &args) override
	{
		if (!args.count("name")) {
			log_error("please input plugin name");
			return false;
		}
		auto &name = args["name"].as<std::string>();
		log_info("reload %s", name.c_str());
		return true;
	}
};

int main(int, char**)
{
	CConsoleLogger::init();
	log_info("Pixpad start");
    // Setup window
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(AppConfig::window_width, AppConfig::window_height, AppConfig::app_name, NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    gl3wInit();
	glfwSetWindowSizeCallback(window, window_size_callback);

    // Setup ImGui binding
    ImGui_ImplGlfwGL3_Init(window, true);

#if defined(WIN32) || defined(WIN64)
	FreeConsole();
	init_process();
	CCommandExit cmd_exit(window);
	console_register_command(&cmd_exit);
	CCommandReload cmd_reload;
	console_register_command(&cmd_reload);
#endif

    // Load Fonts
    // (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
    //ImGuiIO& io = ImGui::GetIO();
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyTiny.ttf", 10.0f);
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());

    bool show_test_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	ImGuiStyle &style = ImGui::GetStyle();
	style.WindowRounding = 4.0f;
	style.Colors[ImGuiCol_Border].w = 0.8f;

	// Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplGlfwGL3_NewFrame();

		show_image("mipmap.png");

		//ImGui::SetNextWindowCollapsed(true, ImGuiCond_Once);
		ImGui::SetNextWindowPos(ImVec2(1, 1), ImGuiCond_Always);
		show_console();

        // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
        //if (show_test_window)
        //{
        //    ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
        //    ImGui::ShowTestWindow(&show_test_window);
        //}

        // Rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplGlfwGL3_Shutdown();
    glfwTerminate();

    return 0;
}
