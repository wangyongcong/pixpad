#include "stdafx.h"
#include "resource.h"
#include "util/log.h"

#include "pixpad.h"

namespace wyc
{

	static const std::unordered_map<int, RENDER_MODE> s_menuid_to_renderer = {
		{ IDM_SOFTWARE, SOFTWARE_RENDERER },
		{ IDM_OPENGL, OPENGL_RENDERER },
	};

	xapp_pixpad::xapp_pixpad()
	{
		for (int i = 0; i < RENDERER_COUNT; ++i)
		{
			pipelines[i] = nullptr;
		}
		for (int i = 0; i < MODEL_COUNT; ++i) {
			models[i].first = nullptr;
			models[i].second = nullptr;
		}
		transform.identity();
		rotate.zero();
	}

	xapp_pixpad::~xapp_pixpad()
	{
		wyc::xpipeline *pipeline;
		for (int i = 0; i < RENDERER_COUNT; ++i)
		{
			pipeline = this->pipelines[i];
			if (pipeline)
				delete pipeline;
		}
		for (int i = 0; i < MODEL_COUNT; ++i) {
			if (models[i].first)
				delete models[i].first;
			if (models[i].second)
				delete models[i].second;
		}
	}

	void xapp_pixpad::on_start()
	{
		// Initialize menu items
		HMENU main_menu = GetMenu(m_hwnd_main);
		CheckMenuRadioItem(main_menu, IDM_SOFTWARE, IDM_OPENGL, IDM_OPENGL, MF_BYCOMMAND);
		CheckMenuRadioItem(main_menu, IDM_REGULAR_TRIANGLE, IDM_REGULAR_TRIANGLE, IDM_REGULAR_TRIANGLE, MF_BYCOMMAND);

		// initialize OpenGL
		glClearColor(0, 0, 0, 1.0f);

		// create models
		wyc::xvertex_buffer *vertices = new wyc::xvertex_buffer();
		wyc::xindex_buffer *indices = new wyc::xindex_buffer();
		this->models[this->model_id] = { vertices, indices };
		wyc::gen_regular_triangle<wyc::xvertex_p3c3>(1, *vertices, *indices);
		auto v = vertices->get_as<wyc::xvertex_p3c3>();
		v[0].color.set(1.0, 0, 0);
		v[1].color.set(0, 1.0, 0);
		v[2].color.set(0, 0, 1.0);

		this->material = "vertex_color";
		this->transform.identity();
		this->transform.set_col(3, wyc::xvec3f_t(0, 0, -4));

		// initialize renderer
		this->pipelines[SOFTWARE_RENDERER] = new wyc::xsw_pipeline();
		this->pipelines[OPENGL_RENDERER] = new wyc::xgl_pipeline();
		wyc::xpipeline *pipeline = this->pipelines[this->render_mode];
		if (pipeline) {
			auto model = this->models[this->model_id];
			pipeline->commit(model.first, model.second);
			pipeline->set_transform(this->transform);
			pipeline->set_material(this->material);
		}
	}

	void xapp_pixpad::on_close()
	{
		debug("app close");
	}

	void xapp_pixpad::on_render()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		wyc::xpipeline *pipeline = this->pipelines[this->render_mode];
		if (pipeline) {
			pipeline->set_transform(this->transform);
			pipeline->render();
		}
		wyc::gl_get_context()->swap_buffers();
	}

	bool xapp_pixpad::on_command(int cmd_id, int cmd_event)
	{
		switch (cmd_id)
		{
		case IDM_OPENGL:
		case IDM_SOFTWARE:
			CheckMenuRadioItem(GetMenu(m_hwnd_main), IDM_SOFTWARE, IDM_OPENGL, cmd_id, MF_BYCOMMAND);
			set_render_mode(s_menuid_to_renderer.at(cmd_id));
			break;
		default:
			return false;
		}
		return true;
	}

	void xapp_pixpad::on_resize(unsigned width, unsigned height)
	{
		debug("resize window: %d x %d", width, height);
		assert(width > 0 && height > 0);
		HWND target_wnd = wyc::gl_get_context()->get_window();
		MoveWindow(target_wnd, 0, 0, width, height, FALSE);
		glViewport(0, 0, width, height);
		wyc::xpipeline *pipeline = this->pipelines[this->render_mode];
		if (pipeline) {
			pipeline->set_viewport(width, height);
			pipeline->set_perspective(45, float(width) / height, 1, 1000);
		}
	}

	void xapp_pixpad::on_mouse_button_down(MOUSE_BUTTON button, int x, int y)
	{
		if (button == MOUSE_BUTTON_LEFT) {
			this->is_drag_mode = true;
			this->drag_start_pos = { x, y };
		}
	}

	void xapp_pixpad::on_mouse_button_up(MOUSE_BUTTON button, int x, int y)
	{
		if (button == MOUSE_BUTTON_LEFT) {
			this->is_drag_mode = false;
			this->rotate.x = this->rotate.z;
			this->rotate.y = this->rotate.w;
		}
	}

	void xapp_pixpad::on_mouse_move(int x, int y)
	{
		if (!this->is_drag_mode)
			return;
		int dx = x - this->drag_start_pos.first,
			dy = y - this->drag_start_pos.second;
		float speed = 0.5f;
		float xrot = dy*speed + this->rotate.x, \
			yrot = dx*speed + this->rotate.y;
		xrot = std::max<float>(-89, std::min<float>(xrot, 89));
		if (yrot >= 360)
			yrot -= 360;
		this->rotate.z = xrot;
		this->rotate.w = yrot;
		wyc::xmat3f_t m1, m2;
		wyc::matrix_yrotate3d(m1, DEG_TO_RAD(yrot));
		wyc::matrix_xrotate3d(m2, DEG_TO_RAD(xrot));
		m1.mul(m2);
		wyc::xvec3f_t v;
		for (int i = 0; i < 3; ++i)
		{
			m1.get_row(i, v);
			this->transform.set_col(i, v);
		}
	}

	void xapp_pixpad::on_key_down(int keycode)
	{
		if (keycode == VK_SPACE)
		{
			this->transform.set_row(0, wyc::xvec3f_t(1, 0, 0));
			this->transform.set_row(1, wyc::xvec3f_t(0, 1, 0));
			this->transform.set_row(2, wyc::xvec3f_t(0, 0, 1));
			this->rotate.zero();
		}
	}

	void xapp_pixpad::set_render_mode(RENDER_MODE mode)
	{
		if (mode == this->render_mode)
			return;
		this->render_mode = mode;
		wyc::xpipeline *pipeline = this->pipelines[mode];
		if (!pipeline)
			return;
		RECT rectClient;
		GetClientRect(this->m_hwnd_main, &rectClient);
		int width = rectClient.right - rectClient.left;
		int height = rectClient.bottom - rectClient.top;
		pipeline->set_viewport(width, height);
		pipeline->set_perspective(45, float(width) / height, 1, 1000);

		auto model = this->models[this->model_id];
		pipeline->commit(model.first, model.second);
		pipeline->set_transform(this->transform);
		pipeline->set_material(this->material);
	}

}; // end of namespace wyc

