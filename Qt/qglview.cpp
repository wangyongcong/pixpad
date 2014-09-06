#include <QQuickWindow>
#include <QSGSimpleRectNode>
#include "qglview.h"

QGLView::QGLView(QQuickItem *parent) :
	QQuickItem(parent)
{
	m_updateTimer = 0;
	setFlag(QQuickItem::ItemHasContents, true);
	connect(this, SIGNAL(windowChanged(QQuickWindow*)), this, SLOT(onWindowChanged(QQuickWindow*)));
}

QGLView::~QGLView()
{
	if(m_updateTimer) {
		delete m_updateTimer;
	}
}

void QGLView::onWindowChanged(QQuickWindow *win)
{
	if(win)
	{
		connect(win, SIGNAL(sceneGraphInitialized()), this, SLOT(onSceneGraphInitialized()), Qt::DirectConnection);
		connect(win, SIGNAL(sceneGraphInvalidated()), this, SLOT(onSceneGraphInvalidated()), Qt::DirectConnection);

		connect(win, SIGNAL(beforeSynchronizing()), this, SLOT(onSync()), Qt::DirectConnection);
		connect(win, SIGNAL(beforeRendering()), this, SLOT(onRender()), Qt::DirectConnection);
		connect(win, SIGNAL(frameSwapped()), this, SLOT(onFrameEnd()), Qt::DirectConnection);

		win->setClearBeforeRendering(false);
	}
	else
	{
		// no window, stop rendering
	}
}

void QGLView::onSceneGraphInitialized()
{
	this->initializeOpenGLFunctions();
//	if(!this->initializeOpenGLFunctions())
//		qDebug("[ERROR] Can't init OpenGL functions");
//	else
//		qDebug("OpenGL functions init OK");
	m_updateTimer = new QTimer(this);
	connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(update()));
	m_updateTimer->setInterval(30);
	m_updateTimer->start();
}

void QGLView::onSceneGraphInvalidated()
{
	// clean up
	if(m_updateTimer) {
		delete m_updateTimer;
		m_updateTimer = 0;
	}
}

void QGLView::onSync()
{
}

void QGLView::onRender()
{
}

void QGLView::onFrameEnd()
{
}

//QSGNode* QGLView::updatePaintNode(QSGNode * oldNode, UpdatePaintNodeData *)
//{
//	return oldNode;
//	QSGSimpleRectNode *node = static_cast<QSGSimpleRectNode*>(oldNode);
//	if(!node)
//	{
//		node = new QSGSimpleRectNode;
//		node->setRect(this->boundingRect());
//	}
//	node->markDirty(QSGNode::DirtyGeometry);
//	return node;
//}

