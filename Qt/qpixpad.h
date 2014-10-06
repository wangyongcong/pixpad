#ifndef QPIXPAD_H
#define QPIXPAD_H

#include "render/vecmath.h"
#include "qglview.h"

class QPixpad : public QGLView
{
	Q_OBJECT
	Q_PROPERTY(QPointF vert0 READ vert0 WRITE setVert0)
	Q_PROPERTY(QPointF vert1 READ vert1 WRITE setVert1)
	Q_PROPERTY(QPointF vert2 READ vert2 WRITE setVert2)
public:
	explicit QPixpad(QQuickItem *parent = 0);
	const QPointF& vert0() const
	{
		return m_pt[0];
	}
	void setVert0(const QPointF &p)
	{
		m_pt[0] = p;
		m_verts_changed = true;
//		qDebug("setVert0: (%f, %f)", p.x(), p.y());
	}
	const QPointF& vert1() const
	{
		return m_pt[1];
	}
	void setVert1(const QPointF &p)
	{
		m_pt[1] = p;
		m_verts_changed = true;
	}
	const QPointF& vert2() const
	{
		return m_pt[2];
	}
	void setVert2(const QPointF &p)
	{
		m_pt[2] = p;
		m_verts_changed = true;
	}
signals:

public slots:
	virtual void onSceneGraphInitialized();
	virtual void onSceneGraphInvalidated();
	virtual void onSync();
	virtual void onRender();
	virtual void onFrameEnd();

protected:
	virtual void geometryChanged(const QRectF & newGeometry, const QRectF & oldGeometry);

private:
	float m_view_w, m_view_h;
	QPointF m_pt[3];
	vec2f_t m_verts[3];
	bool m_verts_changed;
	GLuint m_program;
	GLuint m_vbo, m_ibo;
	GLuint m_texture;
	mat4f_t m_mat_proj;
};

#endif // QPIXPAD_H
