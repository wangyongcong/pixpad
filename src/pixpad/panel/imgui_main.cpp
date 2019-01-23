#include <cstdio>
#include "imgui.h"

// ImGui demo window
extern void show_demo();
// console window
extern void show_console();
//
extern void show_pixel();

// main window entry
void show_main_frame()
{
	show_pixel();
	//	show_demo();
	show_console();
}

void show_pixel()
{
	constexpr int row = 64, col = 64;
	constexpr int cell_size = 12;
	constexpr int point_size = 2;
	constexpr int origin_x = 400, origin_y = 2;
	
	ImDrawList* draw_list = ImGui::GetOverlayDrawList();
	draw_list->PushClipRectFullScreen();
	
	ImVec2 x0, x1;
	x0.x = origin_x;
	x0.y = origin_y;
	x1.x = origin_x + col * cell_size;
	x1.y = origin_y;
	for(int r = 0; r <= row; ++r)
	{
		draw_list->AddLine(x0, x1, 0xFF00FF00);
		x0.y += cell_size;
		x1.y += cell_size;
	}
	
	x0.x = origin_x;
	x0.y = origin_y;
	x1.x = origin_x;
	x1.y = origin_y + row * cell_size;
	for(int c = 0; c <= col; ++c)
	{
		draw_list->AddLine(x0, x1, 0xFF00FF00);
		x0.x += cell_size;
		x1.x += cell_size;
	}
	
	x0.y = origin_y + cell_size * 0.5;
	for(int r = 0; r < row; ++r, x0.y += cell_size)
	{
		x0.x = origin_x + cell_size * 0.5;
		for(int c = 0; c < col; ++c)
		{
			draw_list->AddCircleFilled(x0, point_size, 0xFFFFFFFF, 6);
			x0.x += cell_size;
		}
	}
	
	draw_list->PopClipRect();
}

void show_demo()
{
	static bool show_demo_window = true;
	static bool show_another_window = false;
	static float clear_color[4] = { 0.28f, 0.36f, 0.5f, 1.0f };

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);
	
	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;
		
		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
		
		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &show_another_window);
		
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
		
		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);
		
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}
	
	// 3. Show another simple window.
	if (show_another_window)
	{
		ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			show_another_window = false;
		ImGui::End();
	}
}

