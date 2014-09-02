#ifndef QGLVIEW_H
#define QGLVIEW_H

#include <QQuickItem>
#include <QTimer>
#include <QOpenGLFunctions>

class QGLView : public QQuickItem, public QOpenGLFunctions
{
	Q_OBJECT
public:
	explicit QGLView(QQuickItem *parent = 0);
	virtual ~QGLView();
	virtual void render();

signals:

public slots:
	void onWindowChanged(QQuickWindow *win);
	void onSceneGraphInitialized();
	void onSceneGraphInvalidated();
	void onFrameEnd();
	void onFrame();

protected:
	virtual QSGNode* updatePaintNode(QSGNode * oldNode, UpdatePaintNodeData * updatePaintNodeData);

private:
	QTimer *m_updateTimer;
};

#endif // QGLVIEW_H
