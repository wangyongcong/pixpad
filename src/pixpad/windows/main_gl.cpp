// ImGui - standalone example application for Glfw + OpenGL 3, using programmable pipeline
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
#include <unordered_map>
#include <imgui.h>
#include "imgui_impl_glfw_gl3.h"
#include <stdio.h>
#include <GL/gl3w.h>    // This example is using gl3w to access OpenGL functions (because it is small). You may use glew/glad/glLoadGen/etc. whatever already works for you.
#include <GLFW/glfw3.h>
#include <string>
// #define WYC_LOG_IMPLEMENTATION
// #include "console_log.h"
#define WYC_SHELLCMD_IMPLEMENTATION
#include "shellcmd.h"

// setup global application config
const char *g_app_name = "pixpad";
int g_window_width = 1280;
int g_window_height = 720;

bool console_register_command(wyc::IShellCommand *cmd);
void console_unregister_command(const std::string &cmd_name);
bool show_console(void);
void show_image(const void *buf = nullptr, unsigned width = 0, unsigned height = 0, unsigned pitch = 0);

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}

static void window_size_callback(GLFWwindow *window, int width, int height)
{
	g_window_width = width;
	g_window_height = height;
}

typedef int(*PFN_GET_TASK_STATE)();
PFN_GET_TASK_STATE get_task_state;
typedef int(*PFN_GET_TASK_RESULT)(const void**, unsigned&, unsigned&, unsigned&);
PFN_GET_TASK_RESULT get_task_result;
typedef void(*PFN_CLEAR_TASK)();
PFN_CLEAR_TASK clear_task;

static HMODULE load_module(const char *module_name)
{
	HMODULE module = LoadLibrary(module_name);
	if (module == NULL) {
		log_error("fail to load library: testbed");
		return NULL;
	}
	// typedef void(*PFN_SET_LOGGER)(wyc::ILogger*);
	// PFN_SET_LOGGER set_logger = (PFN_SET_LOGGER)GetProcAddress(module, "set_logger");
	// if (set_logger) {
	// 	set_logger(LOGGER_GET(CConsoleLogger));
	// }
	typedef wyc::IShellCommand**(*PFN_GET_COMMAND)(int&);
	PFN_GET_COMMAND get_command = (PFN_GET_COMMAND)GetProcAddress(module, "get_command_list");
	if (get_command) {
		int cnt = 0;
		auto cmd_lst = get_command(cnt);
		for (int i = 0; i < cnt; ++i) {
			console_register_command(cmd_lst[i]);
		}
	}
	else {
		log_error("module [testbed]: no get_command interface");
	}
	get_task_state = (PFN_GET_TASK_STATE)GetProcAddress(module, "get_task_state");
	get_task_result = (PFN_GET_TASK_RESULT)GetProcAddress(module, "get_task_result");
	clear_task = (PFN_CLEAR_TASK)GetProcAddress(module, "clear_task");
	return module;
}

class CCommandExit : public wyc::CShellCommand
{
public:
	CCommandExit(GLFWwindow* window)
		: CShellCommand("exit", "Exit application")
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
		: CShellCommand("reload", "reload module")
	{
		m_opt.add_options()
			("help", "show help message")
			("name", po::value<std::string>(), "module name")
			;
		m_pos_opt.add("name", 1);
	}
	virtual bool process(const po::variables_map &args) override
	{
		if (!args.count("name")) {
			log_error("no module name");
			return false;
		}
		const std::string &module_name = args["name"].as<std::string>();
		HMODULE module = GetModuleHandle(module_name.c_str());
		if (module) {
			log_info("module has been loaded");
			return true;
		}
		module = load_module(module_name.c_str());
		if (module) {
			log_info("reload module success");
		}
		return true;
	}
};

class CCommandUnload : public wyc::CShellCommand
{
public:
	CCommandUnload()
		: CShellCommand("unload", "unload module")
	{
		m_opt.add_options()
			("help", "show help message")
			("name", po::value<std::string>(), "module name")
			;
		m_pos_opt.add("name", 1);
	}
	virtual bool process(const po::variables_map &args) override
	{
		if (!args.count("name")) {
			log_error("no module name");
			return false;
		}
		const std::string &module_name = args["name"].as<std::string>();
		HMODULE module = GetModuleHandle(module_name.c_str());
		if (!module) {
			log_info("module not found");
			return false;
		}
		typedef wyc::IShellCommand* (*PFN_GET_COMMAND)();
		PFN_GET_COMMAND get_command = (PFN_GET_COMMAND)GetProcAddress(module, "get_command");
		if (get_command) {
			auto cmd = get_command();
			console_unregister_command(cmd->name());
		}
		if (!FreeLibrary(module)) {
			log_error("fail to free module");
			return false;
		}
		log_info("unload module success");
		return true;
	}
};

int main(int, char**)
{
	// CConsoleLogger::init();
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
    GLFWwindow* window = glfwCreateWindow(g_window_width, g_window_height, g_app_name, NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    gl3wInit();
	glfwSetWindowSizeCallback(window, window_size_callback);

    // Setup ImGui binding
    ImGui_ImplGlfwGL3_Init(window, true);

#if defined(WIN32) || defined(WIN64)
	FreeConsole();
	load_module("testbed");
	CCommandExit cmd_exit(window);
	console_register_command(&cmd_exit);
	CCommandReload cmd_reload;
	console_register_command(&cmd_reload);
	CCommandUnload cmd_unload;
	console_register_command(&cmd_unload);
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

    bool show_test_window = false;
	bool collapsed_console = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	ImGuiStyle &style = ImGui::GetStyle();
	style.WindowRounding = 4.0f;
	style.Colors[ImGuiCol_Border].w = 0.8f;

	// Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplGlfwGL3_NewFrame();

		ImGuiIO& io = ImGui::GetIO();
		if (io.KeyCtrl && ImGui::IsKeyReleased('`')) 
			ImGui::SetNextWindowCollapsed(!collapsed_console, ImGuiCond_Always);
		ImGui::SetNextWindowPos(ImVec2(1, 1), ImGuiCond_Always);
		collapsed_console = !show_console();

        // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
        if (show_test_window)
        {
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
            ImGui::ShowTestWindow();
        }

		int st = get_task_state();
		if (st == 2)
		{
			const void *buf;
			unsigned w, h, pitch;
			if (0 == get_task_result(&buf, w, h, pitch)) {
				// setup image
				show_image(buf, w, h, pitch);
			}
			clear_task();
		}
		show_image();

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
