#include <string>
#include <vector>
#include "imgui.h"
#include "app_config.h"

class CGuiConsole
{
public:
	CGuiConsole()
	{
		m_width = int(AppConfig::window_width * 0.4), m_height = int(AppConfig::window_height * 0.8);
	}

	void draw()
	{
		ImGui::SetNextWindowSize(ImVec2(m_width, m_height), ImGuiCond_Always);
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_ShowBorders;
		if (!ImGui::Begin("console", 0, window_flags))
		{
			ImGui::End();
			return;
		}
		m_filter.Draw("Filter");
		ImGui::SameLine();
		if (ImGui::Button("Clear")) {
		}
		ImGui::Separator();

		// BEGIN log
		ImGui::BeginChild("console_log", ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);
		for(auto &str: m_log_buff)
			ImGui::TextUnformatted(str.c_str());
		ImGui::EndChild(); // END log
		ImGui::Separator();

		// BEGIN input
		if (ImGui::InputText("Input", m_input_buff, INPUT_BUFF_SIZE, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory, 
			[] (ImGuiTextEditCallbackData *ctx) -> int {
			((CGuiConsole*)ctx)->on_input_end();
			return 0;
		}, (void*)this))
		{
			m_log_buff.emplace_back(m_input_buff);
			m_input_buff[0] = 0;
		} // END input

		ImGui::End(); // END console
	}

	void on_input_end()
	{
	}

private:
	int m_width, m_height;
	std::vector<std::string> m_log_buff;
	ImGuiTextFilter m_filter;
	static constexpr size_t INPUT_BUFF_SIZE = 256;
	char m_input_buff[INPUT_BUFF_SIZE];
};

void draw_console()
{
	static CGuiConsole s_console;
	s_console.draw();
}
