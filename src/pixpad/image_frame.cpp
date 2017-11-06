#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "app_config.h"
#include "console_log.h"

class CImageFrame
{
public:
	CImageFrame(const char *img_file=nullptr)
		: m_texid(0)
	{
		m_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
	}

	~CImageFrame()
	{
		clear();
	}

	bool set_image(const void *buf, unsigned width, unsigned height, unsigned pitch)
	{
		GLenum gl_err;
		GLuint texid;
		glGenTextures(1, &texid);
		if (!texid) {
			log_error("OpenGL generate texture error: %d", glGetError());
			return false;
		}
		glBindTexture(GL_TEXTURE_2D, texid);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		bool is_stride = pitch != width;
		if (is_stride) 
			glPixelStorei(GL_UNPACK_ROW_LENGTH, pitch);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
		if (is_stride)
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		gl_err = glGetError();
		if (gl_err != GL_NO_ERROR) {
			glDeleteTextures(1, &texid);
			log_error("OpenGL Error: %d", gl_err);
			return false;
		}

		if (m_texid)
			clear();
		m_texid = texid;
		m_imgw = width;
		m_imgh = height;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &m_texw);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &m_texh);
		log_info("image(%d x %d), texture(%d x %d)", m_imgw, m_imgh, m_texw, m_texh);

		const auto &style = ImGui::GetStyle();
		m_frame_size.x = m_imgw + style.WindowPadding.x * 2;
		m_frame_size.y = m_imgh + style.WindowPadding.y * 2;

		return true;
	}

	void clear()
	{
		if (!m_texid)
			return;
		glDeleteTextures(1, &m_texid);
		m_texid = 0;
	}

	void draw()
	{
		if (!m_texid)
			return;
		ImVec2 pos((AppConfig::window_width - m_frame_size.x) / 2, (AppConfig::window_height - m_frame_size.y) / 2);
		ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
		ImGui::SetNextWindowSize(m_frame_size, ImGuiCond_Always);
		ImGui::Begin("image", 0, m_flags);
		ImGui::Image((ImTextureID)m_texid, ImVec2(m_imgw, m_imgh), ImVec2(0, 0), ImVec2(1, 1));
		ImGui::End();
	}


private:
	GLuint m_texid;
	int m_imgw, m_imgh;
	int m_texw, m_texh;
	int m_width, m_height;
	ImVec2 m_frame_size;
	ImGuiWindowFlags m_flags;
};

void show_image(const void *buf=nullptr, unsigned width=0, unsigned height=0, unsigned pitch=0)
{
	static CImageFrame s_image;
	if (buf) {
		s_image.set_image(buf, width, height, pitch);
	}
	else {
		s_image.draw();
	}
}
