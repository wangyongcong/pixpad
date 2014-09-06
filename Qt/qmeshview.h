#ifndef QMESHVIEW_H
#define QMESHVIEW_H

#include "mathex/vecmath.h"
#include "qglview.h"

class QMeshView : public QGLView
{
	Q_OBJECT
public:
	explicit QMeshView(QQuickItem *parent = 0);

signals:

public slots:
	void onSync();
	void onRender();
	void onFrameEnd();
	void onVertsChanged(int idx, int x, int y);

private:
	wyc::xvec2f_t m_verts[3];

};

#endif // QMESHVIEW_H
