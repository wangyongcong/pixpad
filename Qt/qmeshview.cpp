#include <QQuickWindow>
#include "qmeshview.h"

QMeshView::QMeshView(QQuickItem *parent) :
	QGLView(parent)
{
	for(int i=0; i<3; ++i)
		m_verts[i].zero();
}

void QMeshView::onSync()
{
}

void QMeshView::onRender()
{
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//	glPushAttrib(GL_ALL_ATTRIB_BITS);
//	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS );
	glUseProgram(0);
	QQuickWindow *win = window();
	QSize sz = win->size() * win->devicePixelRatio();
	glViewport(0, 0, sz.width(), sz.height());
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
//	glPopClientAttrib();
}

void QMeshView::onFrameEnd()
{

}

void QMeshView::onVertsChanged(int idx, int x, int y)
{
	if(idx >= 3)
		return;
	QQuickWindow *win = window();
	if(!win)
		return;
	QSize sz = win->size() * win->devicePixelRatio();
	int w = sz.width();
	int h = sz.height();
	m_verts[idx].set(float(x)/w, 1.0f - float(y)/h);
	qDebug("verts[%d]: (%.2f, %.2f)", idx, m_verts[idx].x, m_verts[idx].y);
}

