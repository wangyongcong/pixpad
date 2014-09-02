#include "qmeshview.h"

QMeshView::QMeshView(QQuickItem *parent) :
	QGLView(parent)
{
	for(int i=0; i<3; ++i)
		m_verts[i].zero();
}

void QMeshView::render()
{
	glBegin(GL_TRIANGLES);
		glVertex2d(m_verts[0].x, m_verts[0].y);
		glColor3f(0, 0, 0);
		glVertex2d(m_verts[1].x, m_verts[1].y);
		glColor3f(0, 0, 0);
		glVertex2d(m_verts[2].x, m_verts[2].y);
		glColor3f(0, 0, 0);
	glEnd();
}

void QMeshView::onVertsChanged(int idx, int x, int y)
{
	if(idx >= 3)
		return;
	m_verts[idx].set(x, y);
//	qDebug("verts[%d]: (%.2f, %.2f)", idx, m_verts[idx].x, m_verts[idx].y);
}

