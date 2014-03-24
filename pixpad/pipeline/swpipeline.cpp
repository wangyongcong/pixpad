#include <cstdlib>

#include "swpipeline.h"

namespace wyc
{
	class xsw_frame_buffer : public xframe_buffer
	{
	public:
		xsw_frame_buffer()
		{
		}
		virtual ~xsw_frame_buffer()
		{

		}
		virtual bool create(unsigned image_format, unsigned width, unsigned height, unsigned depth_format = 0, unsigned stencil_format = 0)
		{
			if ( !m_color_buffer.storage(width, height, pixel_size(XG_PIXEL_FORMAT(image_format))) )
				return false;
			return true;
		}
		virtual void beg_render()
		{
			
		}
		virtual void end_render()
		{

		}
		virtual xrender_buffer* get_buffer(BUFFER_TYPE t)
		{
			if (t == COLOR_BUFFER)
				return &m_color_buffer;
			return NULL;
		}
	private:
		xrender_buffer m_color_buffer;
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
		assert(m_fbo == NULL);
		m_fbo = new xsw_frame_buffer();
		if (!m_fbo->create(NATIVE_PIXEL_FORMAT, width, height))
			return false;
		xrender_buffer* color_buffer = m_fbo->get_buffer(xframe_buffer::COLOR_BUFFER);
		assert(color_buffer);
		m_raster.attach_color_buffer(NATIVE_PIXEL_FORMAT, *color_buffer);
		m_raster.clear_screen();
		m_raster.enable_anti(true);
		int cx = width / 2, cy = height / 2;
		int rx = width / 3, ry = height / 3;
		m_raster.ellipse(cx, cy, rx, ry);

		if (m_texobj == 0) {
			glGenTextures(1, &m_texobj);
			if (!m_texobj) 
				return false;
		}
		glEnable(GL_TEXTURE_RECTANGLE);
		glBindTexture(GL_TEXTURE_RECTANGLE, m_texobj);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_raster.color_buffer());

		return true;
	}
	void xsw_pipeline::beg_frame()
	{
	}
	void xsw_pipeline::end_frame()
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glBindTexture(GL_TEXTURE_RECTANGLE, m_texobj);
		float w = (float)m_raster.width(), h = (float)m_raster.height();
		float z = -10;
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex3f(0, 0, z);
		glTexCoord2f(w, 0);
		glVertex3f(w, 0, z);
		glTexCoord2f(w, h);
		glVertex3f(w, h, z);
		glTexCoord2f(0, h);
		glVertex3f(0, h, z);
		glEnd();
	}
	void xsw_pipeline::draw(/*vertex buffer*/)
	{

	}


}; // end of namespace wyc