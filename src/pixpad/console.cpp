#include "imgui.h"
#include "app_config.h"

void gui_draw_console()
{
	ImGui::SetNextWindowSize(ImVec2(int(AppConfig::window_width * 0.4), int(AppConfig::window_height * 0.8)), ImGuiCond_Always);
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_ShowBorders;
	if (!ImGui::Begin("console", 0, window_flags))
	{
		ImGui::End();
		return;
	}
	ImGui::Text("imgui console. (%s)", IMGUI_VERSION);
	ImGui::End();
}
