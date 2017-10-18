#include "imgui.h"

void gui_draw_console()
{
	ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiCond_Always);
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_ShowBorders;
	ImGui::Begin("console", 0, window_flags);
	ImGui::Text("dear imgui says hello. (%s)", IMGUI_VERSION);
	ImGui::End();
}
