#include <cstdlib>

#include "swpipeline.h"

namespace wyc
{
	class xsw_frame_buffer : public xframe_buffer
	{
	public:
		xsw_frame_buffer() : m_color_buffer()
		{
		}
		virtual ~xsw_frame_buffer()
		{

		}
		virtual bool create(unsigned image_format, unsigned width, unsigned height, unsigned depth_format = 0, unsigned stencil_format = 0)
		{
			if (!m_color_buffer.create(width, height, PIXEL_FMT_RGBA8888))
				return false;
			return true;
		}
		virtual void beg_render()
		{
			
		}
		virtual void end_render()
		{

		}
		xcolor_buffer& get_color_buffer()
		{
			return m_color_buffer;
		}
	private:
		xcolor_buffer m_color_buffer;
	};

	xsw_pipeline::xsw_pipeline()
	{
		m_fbo = NULL;
		m_texobj = 0;
	}

	xsw_pipeline::~xsw_pipeline()
	{
		if (m_fbo != NULL)
		{
			delete m_fbo;
			m_fbo = 0;
		}
		if (m_texobj)
		{
			glDeleteTextures(1, &m_texobj);
			m_texobj = 0;
		}
	}

	bool xsw_pipeline::create_surface(unsigned format, unsigned width, unsigned height)
	{
		if (m_fbo != NULL) {
			delete m_fbo;
		}
		xsw_frame_buffer *fbo = new xsw_frame_buffer();
		fbo->create(format, width, height);
		m_fbo = fbo;
		m_raster.share_color_buffer(fbo->get_color_buffer(),0,0,width,height);
		m_raster.set_bkcolor(0, 0, 255);
		m_raster.set_color(255, 0, 0);

		if (m_texobj == 0) {
			glGenTextures(1, &m_texobj);
			if (!m_texobj) 
				return false;
		}
		glBindTexture(GL_TEXTURE_RECTANGLE, m_texobj);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		return true;
	}
	void xsw_pipeline::beg_frame()
	{
		m_raster.clear_screen();
		m_raster.rect(100, 100, 100, 100);
	}
	void xsw_pipeline::end_frame()
	{
		glClear(GL_COLOR_BUFFER_BIT);
		unsigned w = m_raster.width(), h = m_raster.height();
		glBindTexture(GL_TEXTURE_RECTANGLE, m_texobj);
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_raster.color_buffer());
		glEnable(GL_TEXTURE_RECTANGLE);
		float z = -10;
		glBegin(GL_QUADS);
		{
			glTexCoord2f(0, 0);
			glVertex3f(0, 0, z);
			glTexCoord2f(w, 0);
			glVertex3f(w, 0, z);
			glTexCoord2f(w, h);
			glVertex3f(w, h, z);
			glTexCoord2f(0, h);
			glVertex3f(0, h, z);
		}
		glEnd();
	}
	void xsw_pipeline::draw(/*vertex buffer*/)
	{

	}


}; // end of namespace wyc