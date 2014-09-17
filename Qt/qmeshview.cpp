#include <QQuickWindow>
#include "qmeshview.h"

QMeshView::QMeshView(QQuickItem *parent) :
	QGLView(parent)
{
	m_view_w = 1;
	m_view_h = 1;
	for(int i=0; i<3; ++i) {
		m_verts[i].zero();
	}
	m_verts_changed = false;
	m_program = 0;
}

void QMeshView::onSync()
{
	if(!m_verts_changed)
		return;
	QQuickWindow *win = window();
	QSize sz = win->size() * win->devicePixelRatio();
	int w = sz.width();
	int h = sz.height();
	if(w == 0 || h == 0)
		return;
	float inv_w = 1.0f / w;
	float inv_h = 1.0f / h;
	for(int i = 0; i < 3; ++i)
	{
		m_verts[i].set(m_pt[i].x() * inv_w, 1.0f - m_pt[i].y() * inv_h);
	}
}

bool QMeshView::buildProgram()
{
	GLuint vs = glslLoadFile(GL_VERTEX_SHADER, "res/texquad.vs");
	if(!vs) {
		return false;
	}
	GLuint fs = glslLoadFile(GL_FRAGMENT_SHADER, "res/texquad.fs");
	if(!fs) {
		glDeleteShader(vs);
		return false;
	}
	GLuint shaders[2] = {vs, fs};
	m_program = glslBuildShader(shaders, 2);
	glDeleteShader(vs);
	glDeleteShader(fs);
	return m_program != 0;
}

void QMeshView::onSceneGraphInitialized()
{
	QGLView::onSceneGraphInitialized();
	if(!buildProgram())
		qWarning("failed to build program");
	else
		qDebug("program ok");
}

void QMeshView::onRender()
{
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	QQuickWindow *win = window();
	QSize sz = win->size() * win->devicePixelRatio();
	int w = sz.width();
	int h = sz.height();
	glUseProgram(0);
	glViewport(0, 0, w, h);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, 0, 1);
	glColor4f(1, 1, 1, 1);
	glPushAttrib(GL_POLYGON_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_TRIANGLES);
		glVertex2f(m_verts[0].x, m_verts[0].y);
		glVertex2f(m_verts[1].x, m_verts[1].y);
		glVertex2f(m_verts[2].x, m_verts[2].y);
	glEnd();
	glPopAttrib();
}

void QMeshView::onFrameEnd()
{

}

void QMeshView::geometryChanged(const QRectF & newGeometry, const QRectF & oldGeometry)
{
	QGLView::geometryChanged(newGeometry, oldGeometry);
	m_verts_changed = true;
}
