#include <string>
#include <vector>
#include "imgui.h"
#include "app_config.h"

class CGuiConsole
{
public:
	CGuiConsole()
		: m_tail(true)
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

		m_filter.Draw("Filter");
		ImGui::SameLine();
		ImGui::Checkbox("Tail", &m_tail);
		const auto &style = ImGui::GetStyle();
		ImGui::Separator();

		float progress_bar_height = 4;

		// BEGIN log
		ImGui::BeginChild("console_log", ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing() - progress_bar_height - style.ItemSpacing.y), 
			false, ImGuiWindowFlags_HorizontalScrollbar);
		for(auto &str: m_log_buff)
			ImGui::TextUnformatted(str.c_str());
		ImGui::EndChild(); // END log

		ImGui::SetCursorPosX(0);
		ImGui::ProgressBar(0.99, ImVec2(m_width - style.WindowPadding.x * 0.5, progress_bar_height), "");

		// BEGIN input
		ImGui::Separator();
		if (ImGui::InputText("Input", m_input_buff, INPUT_BUFF_SIZE, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory,
			[] (ImGuiTextEditCallbackData *ctx) -> int {
			((CGuiConsole*)ctx)->on_input_end();
			return 0;
		}, (void*)this))
		{
			if (m_input_buff[0]) {
				m_log_buff.emplace_back(m_input_buff);
				m_input_buff[0] = 0;
			}
		}
		 // Demonstrate keeping auto focus on the input box
		if (ImGui::IsItemHovered() || (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)))
			ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
		// END input

		ImGui::End(); // END console
	}

	void on_input_end()
	{
	}

private:
	int m_width, m_height;
	ImGuiWindowFlags m_flags;
	std::vector<std::string> m_log_buff;
	ImGuiTextFilter m_filter;
	static constexpr size_t INPUT_BUFF_SIZE = 256;
	char m_input_buff[INPUT_BUFF_SIZE];
	bool m_tail;
};

void show_console()
{
	static CGuiConsole s_console;
	s_console.draw();
}
