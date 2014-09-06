#include <QQuickWindow>
#include <QSGSimpleRectNode>
#include <QOpenGLContext>
#include "qglview.h"

QGLView::QGLView(QQuickItem *parent) :
	QQuickItem(parent)
{
	setFlag(QQuickItem::ItemHasContents, true);
	connect(this, SIGNAL(windowChanged(QQuickWindow*)), this, SLOT(onWindowChanged(QQuickWindow*)));
	// start frame update
	m_updateTimer = new QTimer(this);
	connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(update()));
	m_updateTimer->setInterval(30);
	m_updateTimer->start();
}

QGLView::~QGLView()
{
	if(m_updateTimer) {
		delete m_updateTimer;
		m_updateTimer = 0;
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

		qDebug("Enter window");
	}
	else
	{
		// no window, stop rendering
		qDebug("Leave window");
	}
}

void QGLView::onSceneGraphInitialized()
{
	this->initializeOpenGLFunctions();
//	if(!this->initializeOpenGLFunctions())
//		qDebug("[ERROR] Can't init OpenGL functions");
//	else
//		qDebug("OpenGL functions init OK");

	QQuickWindow *win = window();
	if(!win)
		return;
	QOpenGLContext *glctx = win->openglContext();
	if(glctx)
	{
		QSurfaceFormat fmt = glctx->format();
		QString profile;
		if(fmt.profile() == QSurfaceFormat::CoreProfile)
			profile = "CoreProfile";
		else
			profile = "CompatibilityProfile";
		qDebug("OpenGL %d.%d %s", fmt.majorVersion(), fmt.minorVersion(), qPrintable(profile));
	}
	else
	{
		qDebug("OpenGL context is not ready");
	}
	qDebug("onSceneGraphInitialized");
}

void QGLView::onSceneGraphInvalidated()
{
	qDebug("onSceneGraphInvalidated");
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

