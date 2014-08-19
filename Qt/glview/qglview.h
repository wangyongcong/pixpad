#ifndef QGLVIEW_H
#define QGLVIEW_H

#include <QQuickItem>
#include <QTimer>

class QGLView : public QQuickItem
{
	Q_OBJECT
public:
	explicit QGLView(QQuickItem *parent = 0);
	virtual ~QGLView();

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
	float m_bkcolor;
	float m_step;
	QTimer *m_updateTimer;
};

#endif // QGLVIEW_H
