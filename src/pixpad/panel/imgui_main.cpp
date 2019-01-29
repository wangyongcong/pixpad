#include <cstdio>
#include "imgui.h"
#include "core/spw_tile_buffer.h"

// ImGui demo window
extern void show_demo();
// console window
extern void show_console();
//
extern void show_pixel();

// main window entry
void show_main_frame()
{
//	show_demo();
	show_pixel();
	show_console();
	
//	ImDrawList* draw_list = ImGui::GetOverlayDrawList();
//	draw_list->PushClipRectFullScreen();
//	draw_list->AddQuadFilled(ImVec2(100, 100), ImVec2(200, 100), ImVec2(200, 200), ImVec2(100, 200), 0xFFFFFFFF);
//	draw_list->PopClipRect();
}

class CPixelMap
{
public:
	CPixelMap()
	{
		m_buf.storage(64, 64);
		ImU32 colors[] = {
			0xFFFFFFFF, 0xFFFF0000, 0xFF0000FF,
			0xFFFFFF00, 0xFFFF00FF, 0xFF00FFFF,
			0xFF88FF00, 0xFF8800FF, 0xFF00FF88,
			0xFF0088FF,
		};
		unsigned color_count = sizeof(colors) / sizeof(ImU32);
		for(auto i=0u; i < m_buf.tile_count(); ++i) {
			m_buf.set_tile(i, colors[i % color_count]);
		}
//		m_buf.clear(0xFFFFFFFF);
	}
	
	~CPixelMap() {
	}
	
	void draw() {
		constexpr int row = 64, col = 64;
		constexpr int cell_size = 11.2;
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
		
		ImVec2 va, vb, vc, vd;
		x0.x = origin_x + cell_size * 0.5 - point_size * 0.5;
		va.y = origin_y + cell_size * 0.5 - point_size * 0.5;
		for(int r = 0; r < row; ++r, x0.y += cell_size)
		{
			va.x = x0.x;
			vb.x = va.x + point_size; vb.y = va.y;
			vc.x = vb.x; vc.y = vb.y + point_size;
			vd.x = va.x; vd.y = vc.y;
			for(int c = 0; c < col; ++c)
			{
				draw_list->AddQuadFilled(va, vb, vc, vd, m_buf.get(c, r));
				va.x += cell_size;
				vb.x += cell_size;
				vc.x += cell_size;
				vd.x += cell_size;
			}
			va.y += cell_size;
		}
		
		draw_list->PopClipRect();
	}
	
private:
	wyc::CSpwTileBuffer<ImU32> m_buf;
	
};

void show_pixel()
{
	static CPixelMap s_pixelmap;
	s_pixelmap.draw();
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

