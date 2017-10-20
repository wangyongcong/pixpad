#include <string>
#include "imgui.h"
#include "app_config.h"
#include "console_log.h"

class CGuiConsole
{
public:
	CGuiConsole()
	{
		m_width = int(AppConfig::window_width * 0.4), m_height = int(AppConfig::window_height * 0.8);
		m_flags = ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_ShowBorders;
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
		ImGui::SetCursorPosX(style.WindowPadding.x * 0.5);
		ImGui::ProgressBar(0, ImVec2(ImGui::GetWindowWidth() - style.WindowPadding.x, progress_bar_height), "");
		ImGui::Separator();

		// BEGIN log
		ImGui::BeginChild("console_log", ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing() - progress_bar_height - style.ItemSpacing.y), 
			false, ImGuiWindowFlags_HorizontalScrollbar);
		CConsoleLogger *logger = dynamic_cast<CConsoleLogger*>(wyc::g_log);
		for (auto &str : *logger) {
			draw_log(str);
		}
		ImGui::EndChild(); // END log

		// BEGIN input
		ImGui::Separator();
		ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
		if (ImGui::InputText("", m_input_buff, INPUT_BUFF_SIZE, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory,
			[] (ImGuiTextEditCallbackData *ctx) -> int {
			((CGuiConsole*)ctx)->on_input_end();
			return 0;
		}, (void*)this))
		{
			if (m_input_buff[0]) {
				wyc::g_log->write(m_input_buff, 0);
				m_input_buff[0] = 0;
			}
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

	void draw_log(const std::string &str)
	{
		static const std::pair<std::string, ImVec4> s_tag_color[] = {
			std::make_pair(std::string("[ERROR]") , ImVec4(1.0f, 0.4f, 0.4f, 1.0f)),
			std::make_pair(std::string("[WARNING]"), ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }),
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

private:
	int m_width, m_height;
	ImGuiWindowFlags m_flags;
	static constexpr size_t INPUT_BUFF_SIZE = 256;
	char m_input_buff[INPUT_BUFF_SIZE];
};

void show_console()
{
	static CGuiConsole s_console;
	s_console.draw();
}
