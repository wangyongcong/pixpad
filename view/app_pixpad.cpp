#include "stdafx.h"
#include "app_pixpad.h"
#include "log.h"
#include "vecmath.h"
#include "raster.h"
#include <OpenEXR/ImathColor.h>

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
			mat4f_t proj;
			set_orthograph(proj, 0, 0, 0, 1, 1, 1);
			glUniformMatrix4fv(loc, 1, GL_TRUE, proj.getValue());
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
		typedef IMATH_NAMESPACE::C3c color_t;
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
		float half_vw, half_vh;
		half_vw = (rx - lx - 0.5f) * 0.5f;
		half_vh = (ry - ly - 0.5f) * 0.5f;

		// draw viewport frame
		c = { 255, 255, 0 };
		m_surf.set_line(ly, c, lx, rx);
		m_surf.set_line(ry - 1, c, lx, rx);
		for (size_t i = ly; i < ry; ++i)
		{
			m_surf.set(lx, i, c);
			m_surf.set(rx - 1, i, c);
		}

		std::vector<vec4f_t> planes;
		// left plane
		planes.push_back(vec4f_t(1, 0, 0, 1));
		// top plane
		planes.push_back(vec4f_t(0, -1, 0, 1));
		// right plane
		planes.push_back(vec4f_t(-1, 0, 0, 1));
		// bottom plane
		planes.push_back(vec4f_t(0, 1, 0, 1));

		std::vector<vec3f_t> vertices;
		for (int i = 0; i < 3; ++i)
		{
			float x = (float)m_rnd.nextf();
			float y = (float)m_rnd.nextf();
			// clamp to [-2, 2]
			x = (x - 0.5f) * 4;
			y = (y - 0.5f) * 4;
			vertices.push_back(vec3f_t(x, y, 0));
			info("v%d: (%f, %f)", i, x, y);
		}
		
		wyc::clip_polygon(planes, vertices);

		// transfrom to screen space
		for (auto &v : vertices)
		{
			float tmp;
			tmp = IMATH_NAMESPACE::clamp(v.x, -1.0f, 1.0f);
			v.x = lx + half_vw * (tmp + 1);
			tmp = IMATH_NAMESPACE::clamp(v.y, -1.0f, 1.0f);
			v.y = ly + half_vh * (tmp + 1);
			assert(v.x >= lx && v.x < rx);
			assert(v.y >= ly && v.y < ry);
		}
		
		if (vertices.size()>2)
		{
			xplotter<color_t> plot(m_surf, color_t(0, 255, 0));
			auto v0 = vertices.back();
			for (auto &v1 : vertices)
			{
				line_sampler(v0.x, v0.y, v1.x, v1.y, plot);
				v0 = v1;
			}
			v0 = vertices.front();
			for (size_t i = 2, end = vertices.size() - 1; i < end; ++i)
			{
				auto &v1 = vertices[i];
				line_sampler(v0.x, v0.y, v1.x, v1.y, plot);
			}
		}
		else
		{
			warn("vertices left: %d", vertices.size());
		}

	}

} // namespace wyc
