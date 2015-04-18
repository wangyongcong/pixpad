#include "stdafx.h"

#include <ctime>
#include <OpenEXR/ImathFun.h>
#include <OpenEXR/ImathColor.h>

#include "app_pixpad.h"
#include "log.h"
#include "math/vecmath.h"
#include "math/vector.h"
#include "math/matrix.h"
#include "raster/raster.h"
#include "raster/gen_mesh.h"

namespace wyc
{
	void xapp_pixpad::on_start()
	{
		m_tex = 0;
		m_vbo = 0;
		m_ibo = 0;
		m_prog = 0;
		m_rnd.init(clock());
		size_t w, h;
		get_viewport_size(w, h);
		glClearColor(0, 0, 0, 0);
		glGenTextures(1, &m_tex);
		if (m_tex == 0)
		{
			error("failed to alloc texture");
			return;
		}
		glBindTexture(GL_TEXTURE_2D, m_tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		
		GLuint buffs[2];
		glGenBuffers(2, buffs);
		m_vbo = buffs[0];
		m_ibo = buffs[1];
		if (!m_vbo || !m_ibo)
		{
			error("Failed to alloc buffers");
			return;
		}
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		float verts[] = {
			0, 0,
			1, 0,
			1, 1,
			0, 1,
		};
		glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
		GLshort indices[] {
			0, 1, 3,
			1, 2, 3,
		};
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		GLuint shaders[2] = { 0, 0 };
		shaders[0] = glsl_load_file(GL_VERTEX_SHADER, "texquad_vs.glsl");
		shaders[1] = glsl_load_file(GL_FRAGMENT_SHADER, "texquad_fs.glsl");
		if (shaders[0] == 0 || shaders[1] == 0)
		{
			error("failed to load shader");
			return;
		}
		m_prog = glsl_build_shader(shaders, 2);
		glDeleteShader(shaders[0]);
		glDeleteShader(shaders[1]);
		if (!m_prog)
		{
			error("failed to build shader program");
			return;
		}
		assert(glGetError() == GL_NO_ERROR);

		on_paint();
	}

	void xapp_pixpad::on_close()
	{
		info("shutting down...");
		if (m_tex) {
			glDeleteTextures(1, &m_tex);
			m_tex = 0;
		}
		if (m_vbo) {
			GLuint buffs[2] = { m_vbo, m_ibo };
			glDeleteBuffers(2, buffs);
			m_vbo = m_ibo = 0;
		}
		if (m_prog) {
			glDeleteProgram(m_prog);
			m_prog = 0;
		}
		m_surf.release();
	}

	void xapp_pixpad::on_render()
	{
		if (!m_redraw)
			return;
		m_redraw = false;
		if (!m_prog)
			return;
		//glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(m_prog);
		GLint loc = glGetUniformLocation(m_prog, "proj");
		if (loc != -1) {
			mat4f proj;
			set_orthograph(proj, 0, 0, 0, 1, 1, 1);
			glUniformMatrix4fv(loc, 1, GL_TRUE, proj.data());
		}
		assert(glGetError() == GL_NO_ERROR);
		loc = glGetUniformLocation(m_prog, "basemap");
		if (loc != -1)
			glUniform1i(loc, 0);
		assert(glGetError() == GL_NO_ERROR);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
		glBindTexture(GL_TEXTURE_2D, m_tex);
		// upload texture data
		size_t w, h;
		get_viewport_size(w, h);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, m_surf.get_buffer());
		// draw primitives
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
		// restore state
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glUseProgram(0);

		gl_get_context()->swap_buffers();
	}

	void xapp_pixpad::on_key_down(int keycode)
	{
		if (keycode == VK_ESCAPE) {
			::PostQuitMessage(0);
			return;
		}
		if (keycode == VK_SPACE) {
			on_paint();
			return;
		}
	}

	void xapp_pixpad::on_paint()
	{
		m_redraw = true;
		typedef Imath::C3c color_t;
		size_t vw, vh;
		get_viewport_size(vw, vh);
		color_t c = { 0, 0, 0 };
		m_surf.storage(vw, vh, sizeof(c));
		m_surf.clear(c);

		size_t lx, ly, rx, ry;
		lx = vw >> 2;
		rx = vw - lx;
		ly = vh >> 2;
		ry = vh - ly;

		// draw viewport frame
		c = { 255, 255, 0 };
		m_surf.set_line(ly, c, lx, rx);
		m_surf.set_line(ry - 1, c, lx, rx);
		for (size_t i = ly; i < ry; ++i)
		{
			m_surf.set(lx, i, c);
			m_surf.set(rx - 1, i, c);
		}
		draw_cube(lx, ly, rx, ry);
	}
	
	void xapp_pixpad::random_triangle(int lx, int ly, int rx, int ry)
	{
		std::vector<vec4f> planes;
		// left plane
		planes.push_back({ 1, 0, 0, 1 });
		// top plane
		planes.push_back({ 0, -1, 0, 1 });
		// right plane
		planes.push_back({ -1, 0, 0, 1 });
		// bottom plane
		planes.push_back({ 0, 1, 0, 1 });

		std::vector<vec3f> vertices;
		for (int i = 0; i < 3; ++i)
		{
			float x = (float)m_rnd.nextf();
			float y = (float)m_rnd.nextf();
			// clamp to [-2, 2]
			x = (x - 0.5f) * 4;
			y = (y - 0.5f) * 4;
			vertices.push_back({ x, y, 0 });
			info("v%d: (%f, %f)", i, x, y);
		}
		wyc::clip_polygon(planes, vertices);

		// transfrom to screen space
		float half_vw, half_vh;
		half_vw = (rx - lx - 0.5f) * 0.5f;
		half_vh = (ry - ly - 0.5f) * 0.5f;
		for (auto &v : vertices)
		{
			float tmp;
			tmp = Imath::clamp(v.x, -1.0f, 1.0f);
			v.x = lx + half_vw * (tmp + 1);
			tmp = Imath::clamp(v.y, -1.0f, 1.0f);
			v.y = ly + half_vh * (tmp + 1);
			assert(v.x >= lx && v.x < rx);
			assert(v.y >= ly && v.y < ry);
		}

		if (vertices.size()>2)
		{
			xplotter<Imath::C3c> plot(m_surf, Imath::C3c(0, 255, 0));
			auto v0 = vertices.back();
			for (auto &v1 : vertices)
			{
				draw_line(v0, v1, plot);
				v0 = v1;
			}
			v0 = vertices.front();
			for (size_t i = 2, end = vertices.size() - 1; i < end; ++i)
			{
				auto &v1 = vertices[i];
				draw_line(v0, v1, plot);
			}
		}
		else
		{
			warn("vertices left: %d", vertices.size());
		}
	}

	void xapp_pixpad::draw_cube(int lx, int ly, int rx, int ry)
	{
		std::vector<vec3f> vertices;
		std::vector<unsigned short> faces;
		wyc::box(0.4f, vertices, faces);

		mat4f proj, m1, m2;
		wyc::set_perspective(proj, 45, 4.f / 3, 1, 100);
		// rotate
		wyc::set_rotate_x(m1, wyc::deg2rad(45.f));
		wyc::set_rotate_y(m2, wyc::deg2rad(30.f));
		m1 *= m2;
		// tranlate
		//m1[0][3] = 0.f;
		//m1[1][3] = 0.f;
		m1[2][3] = -1.4f;
		// finally the MVP matrix
		proj *= m1;

		// viewing vector 
		vec3f camera_pos = { 0, 0, 0 };
		// translate to object space
		if (m2.inverse_of(m1)) 
			camera_pos = m2 * camera_pos;

		float half_vw, half_vh;
		half_vw = (rx - lx - 0.5f) * 0.5f;
		half_vh = (ry - ly - 0.5f) * 0.5f;

		std::vector<vec4f> verts_cache;
		verts_cache.reserve(9);
		vec3f v0, v1, v2, n, view;
		int beg = 0, end = faces.size();
		for (int i = beg + 2; i < end; i += 3)
		{
			// projection
			v0 = vertices[faces[i - 2]];
			v1 = vertices[faces[i - 1]];
			v2 = vertices[faces[i]];
			// backface culling
			n = v1 - v0;
			n = n.cross(v2 - v0);
			n.normalize();
			view = camera_pos - v0;
			view.normalize();
			if ((n ^ view) < 0)
				continue;
			// MVP transform
			verts_cache.push_back(proj * v0);
			verts_cache.push_back(proj * v1);
			verts_cache.push_back(proj * v2);
			// clipping
			wyc::clip_polygon_homo(verts_cache);
			if (verts_cache.empty())
				continue;
			// viewport transform
			for (auto &v : verts_cache)
			{
				v /= v.w;
				float tmp;
				tmp = Imath::clamp(v.x, -1.0f, 1.0f);
				v.x = lx + half_vw * (tmp + 1);
				tmp = Imath::clamp(v.y, -1.0f, 1.0f);
				v.y = ly + half_vh * (tmp + 1);
				assert(v.x >= lx && v.x < rx);
				assert(v.y >= ly && v.y < ry);
			}
			// draw
			draw_triangles(verts_cache);
			// clear cache
			verts_cache.clear();
		}

	}

	void xapp_pixpad::draw_triangles(const std::vector<vec4f> &vertices)
	{
		xplotter<Imath::C3c> plot(m_surf, Imath::C3c(0, 255, 0));
		auto v0 = vertices.back();
		for (auto &v1 : vertices)
		{
			draw_line(v0, v1, plot);
			v0 = v1;
		}
		v0 = vertices.front();
		for (size_t i = 2, end = vertices.size() - 1; i < end; ++i)
		{
			auto &v1 = vertices[i];
			draw_line(v0, v1, plot);
		}
	}


} // namespace wyc
