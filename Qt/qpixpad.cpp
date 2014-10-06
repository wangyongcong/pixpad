#include <cassert>
#include <QQuickWindow>
#include <QImage>
#include "qpixpad.h"

QPixpad::QPixpad(QQuickItem *parent) :
	QGLView(parent)
{
	m_view_w = 1;
	m_view_h = 1;
	for(int i=0; i<3; ++i) {
		m_verts[i] = {0, 0};
	}
	m_verts_changed = false;
	m_program = 0;
	m_vbo = 0;
	m_ibo = 0;
	m_texture = 0;
	wyc::set_orthograph(m_mat_proj, 0, 0, 0, 1, 1, 1);
}

void QPixpad::onSceneGraphInitialized()
{
	QGLView::onSceneGraphInitialized();
	// create graph resource
	GLuint shaders[2] = {0, 0};
	shaders[0] = glslLoadFile(GL_VERTEX_SHADER, "res/texquad_vs.glsl");
	if(!shaders[0]) {
		qFatal("Failed to load vertex shader");
		return;
	}
	shaders[1] = glslLoadFile(GL_FRAGMENT_SHADER, "res/texquad_fs.glsl");
	if(!shaders[1]) {
		glDeleteShader(shaders[0]);
		qFatal("Failed to load fragment shader");
		return;
	}
	m_program = glslBuildShader(shaders, 2);
	glDeleteShader(shaders[0]);
	glDeleteShader(shaders[1]);
	if(!m_program) {
		qFatal("Failed to build shader program");
		return;
	}
	Q_ASSERT(GL_NO_ERROR == glGetError());
	GLuint buffs[2];
	glGenBuffers(2, buffs);
	m_vbo = buffs[0];
	m_ibo = buffs[1];
	if(!m_vbo || !m_ibo)
	{
		qFatal("Failed to alloc buffers");
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
	Q_ASSERT(GL_NO_ERROR == glGetError());

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	GLshort indices[] {
		0, 1, 3,
		1, 2, 3,
	};
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	Q_ASSERT(GL_NO_ERROR == glGetError());

	glGenTextures(1, &m_texture);
	if(!m_texture)
		return;
	glBindTexture(GL_TEXTURE_2D, m_texture);
	size_t sz = sizeof(GLubyte) * 3 * 32 * 32;
	GLubyte *pix = new GLubyte[sz];
	for(size_t i=0; i<sz; i+=3)
	{
		pix[i] = 0;
		pix[i+1] = 255;
		pix[i+2] = 0;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, pix);
	delete [] pix;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	Q_ASSERT(GL_NO_ERROR == glGetError());
}

void QPixpad::onSceneGraphInvalidated()
{
	// clean up graph resource
	QGLView::onSceneGraphInvalidated();
}

void QPixpad::onSync()
{
	if(!m_verts_changed)
		return;
	QQuickWindow *win = window();
	QSize sz = win->size() * win->devicePixelRatio();
	int w = sz.width();
	int h = sz.height();
	if(w == 0 || h == 0)
		return;
	w = 1.0f / w;
	h = 1.0f / h;
	float x, y;
	for(int i = 0; i < 3; ++i)
	{
		x = float(m_pt[i].x());
		y = float(m_pt[i].y());
		m_verts[i] = {x * w, 1.0f - y * h};
	}
}

void QPixpad::onRender()
{
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//	QQuickWindow *win = window();
//	QSize sz = win->size() * win->devicePixelRatio();
//	int w = sz.width();
//	int h = sz.height();
//	glViewport(0, 0, w, h);
	if(!m_program)
		return;
	glUseProgram(m_program);
	assert (glGetError() == GL_NO_ERROR);
	GLint loc = glGetUniformLocation(m_program, "projection");
	if(loc != -1)
		glUniformMatrix4fv(loc, 1, GL_TRUE, m_mat_proj.getValue());
	assert (glGetError() == GL_NO_ERROR);
	loc = glGetUniformLocation(m_program, "basemap");
	if(loc != -1)
		glUniform1i(loc, 0);
	assert (glGetError() == GL_NO_ERROR);

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBindTexture(GL_TEXTURE_2D, m_texture);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	assert (glGetError() == GL_NO_ERROR);

	glUseProgram(0);
	return;

//	glUseProgram(0);
//	glMatrixMode(GL_MODELVIEW);
//	glLoadIdentity();
//	glMatrixMode(GL_PROJECTION);
//	glLoadMatrixf(m_mat_proj.data());
////	glLoadIdentity();
////	glOrtho(0, 1, 0, 1, 0, 1);
//	glColor4f(1, 1, 1, 1);
//	glPushAttrib(GL_POLYGON_BIT);
//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//	glBegin(GL_TRIANGLES);
//		glVertex2f(m_verts[0].x, m_verts[0].y);
//		glVertex2f(m_verts[1].x, m_verts[1].y);
//		glVertex2f(m_verts[2].x, m_verts[2].y);
//	glEnd();
//	glPopAttrib();
}

void QPixpad::onFrameEnd()
{

}

void QPixpad::geometryChanged(const QRectF & newGeometry, const QRectF & oldGeometry)
{
	QGLView::geometryChanged(newGeometry, oldGeometry);
	m_verts_changed = true;
}
