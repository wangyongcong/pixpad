#include <unordered_map>
#include <string>
#include "imgui.h"
#include "app_config.h"
#include "console_log.h"

typedef void(*PFN_EXECUTE_COMMAND)(const std::string &);

class CGuiConsole
{
public:
	static inline CGuiConsole& singleton()
	{
		static CGuiConsole s_console;
		return s_console;
	}

	void draw()
	{
		ImGui::SetNextWindowSize(ImVec2(m_width, m_height), ImGuiCond_Always);
		if (!ImGui::Begin("console", 0, m_flags))
		{
			ImGui::End();
			return;
		}

		const auto &style = ImGui::GetStyle();
		float progress_bar_height = 4;
		ImGui::SetCursorPosX(style.WindowPadding.x * 0.5f);
		ImGui::ProgressBar(0, ImVec2(ImGui::GetWindowWidth() - style.WindowPadding.x, progress_bar_height), "");
		ImGui::Separator();

		// BEGIN log
		ImGui::BeginChild("console_log", ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing() - progress_bar_height - style.ItemSpacing.y), 
			false, ImGuiWindowFlags_HorizontalScrollbar);
		CConsoleLogger *logger = LOGGER_GET(CConsoleLogger);
		for (auto &str : *logger) {
			draw_log(str);
		}
		ImGui::EndChild(); // END log

		// BEGIN input
		ImGui::Separator();
		ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
		if (ImGui::InputText("", m_input_beg, m_input_max, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory,
			[] (ImGuiTextEditCallbackData *ctx) -> int {
			((CGuiConsole*)ctx)->on_input_end();
			return 0;
		}, (void*)this))
		{
			if (m_input_beg[0]) {
				logger->output(m_input_buf);
				process_input();
			}
			m_input_beg[0] = 0;
		}
		ImGui::PopItemWidth();

		 // Demonstrate keeping auto focus on the input box
		if (ImGui::IsItemHovered() || (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)))
			ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
		// END input

		ImGui::End(); // END console
	}

	void on_input_end()
	{
	}

	void process_input()
	{
		std::string cmd_name = strtok(m_input_beg, " ");
		if (cmd_name == "help")
		{
			for (auto &it : m_commands)
				log_info("- %s\t\t%s", it.first.c_str(), it.second.second.c_str());
			log_info("- help\t\tshow help");
			return;
		}
		auto iter = m_commands.find(cmd_name);
		if (iter == m_commands.end()) {
			log_error("Unkonwn command");
			return;
		}
		iter->second.first(m_input_beg + cmd_name.size() + 1);
	}

	void draw_log(const std::string &str)
	{
		static const std::pair<std::string, ImVec4> s_tag_color[] = {
			std::make_pair(std::string("# "), ImVec4(0.0f, 1.0f, 0.0f, 1.0f)),
			std::make_pair(std::string("[ERROR]") , ImVec4(1.0f, 0.4f, 0.4f, 1.0f)),
			std::make_pair(std::string("[WARNING]"), ImVec4(1.0f, 1.0f, 0.0f, 1.0f)),
		};
		for (auto &tag : s_tag_color) {
			if (!str.compare(0, tag.first.size(), tag.first)) {
				ImGui::PushStyleColor(ImGuiCol_Text, tag.second);
				ImGui::TextUnformatted(str.c_str());
				ImGui::PopStyleColor();
				return;
			}
		}
		ImGui::TextUnformatted(str.c_str());
	}

	bool add_command(const char *cmd_name, PFN_EXECUTE_COMMAND func, const char *desc)
	{
		if (!func || !cmd_name || *cmd_name == 0)
			return false;
		auto &ret = m_commands.emplace(std::make_pair(cmd_name, std::make_pair(func, desc)));
		if (!ret.second)
			return false;
		return true;
	}

private:
	int m_width, m_height;
	ImGuiWindowFlags m_flags;
	static constexpr size_t INPUT_BUFF_SIZE = 256;
	char m_input_buf[INPUT_BUFF_SIZE];
	char *m_input_beg;
	size_t m_input_max;
	std::unordered_map<std::string, std::pair<PFN_EXECUTE_COMMAND, std::string>> m_commands;

	CGuiConsole()
	{
		m_width = int(AppConfig::window_width * 0.382f), m_height = int(AppConfig::window_height);
		m_flags = ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_ShowBorders;
		const char *prompt = "# ";
		size_t prompt_len = strlen(prompt);
		strncpy(m_input_buf, prompt, INPUT_BUFF_SIZE);
		m_input_beg = m_input_buf + prompt_len;
		m_input_max = INPUT_BUFF_SIZE - prompt_len;
	}
};


bool console_command(const char *cmd_name, PFN_EXECUTE_COMMAND func, const char *desc)
{
	auto &console = CGuiConsole::singleton();
	return console.add_command(cmd_name, func, desc);
}

void show_console()
{
	auto &console = CGuiConsole::singleton();
	console.draw();
}
