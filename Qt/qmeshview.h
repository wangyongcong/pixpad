#ifndef QMESHVIEW_H
#define QMESHVIEW_H

#include "mathex/vecmath.h"
#include "qglview.h"

class QMeshView : public QGLView
{
	Q_OBJECT
	Q_PROPERTY(QPointF vert0 READ vert0 WRITE setVert0)
	Q_PROPERTY(QPointF vert1 READ vert1 WRITE setVert1)
	Q_PROPERTY(QPointF vert2 READ vert2 WRITE setVert2)
public:
	explicit QMeshView(QQuickItem *parent = 0);
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
		m_pt[1 ] = p;
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
	void onSync();
	void onRender();
	void onFrameEnd();

protected:
	virtual void geometryChanged(const QRectF & newGeometry, const QRectF & oldGeometry);

private:
	float m_view_w, m_view_h;
	QPointF m_pt[3];
	wyc::xvec2f_t m_verts[3];
	bool m_verts_changed;
};

#endif // QMESHVIEW_H
