#include <QQuickWindow>
#include <QSGSimpleRectNode>
#include "qglview.h"

QGLView::QGLView(QQuickItem *parent) :
	QQuickItem(parent)
{
	m_bkcolor = 0;
	m_step = 0.03f;
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
		m_updateTimer = new QTimer(this);
		connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(onFrame()));
		m_updateTimer->setInterval(30);
		m_updateTimer->start();
		connect(win, SIGNAL(sceneGraphInitialized()), this, SLOT(onSceneGraphInitialized()), Qt::DirectConnection);
		connect(win, SIGNAL(sceneGraphInvalidated()), this, SLOT(onSceneGraphInvalidated()), Qt::DirectConnection);
		connect(win, SIGNAL(frameSwapped()), this, SLOT(onFrameEnd()));
	}
}

void QGLView::onSceneGraphInitialized()
{

}

void QGLView::onSceneGraphInvalidated()
{
	// clean up
	if(m_updateTimer) {
		delete m_updateTimer;
		m_updateTimer = 0;
	}
}

void QGLView::onFrame()
{
	this->update();
}

void QGLView::onFrameEnd()
{
}

QSGNode* QGLView::updatePaintNode(QSGNode * oldNode, UpdatePaintNodeData *)
{
	QSGSimpleRectNode *node = static_cast<QSGSimpleRectNode*>(oldNode);
	if(!node)
	{
		node = new QSGSimpleRectNode;
		node->setRect(this->boundingRect());
	}
	QColor c;
	c.setRgbF(m_bkcolor, m_bkcolor, m_bkcolor);
	node->setColor(c);
	m_bkcolor += m_step;
	if(m_bkcolor > 1)
	{
		m_step = -m_step;
		m_bkcolor = 1 + m_step;
	}
	else if(m_bkcolor < 0)
	{
		m_step = -m_step;
		m_bkcolor = m_step;
	}
//	qDebug("updatePaintNode: %f", m_bkcolor);
	node->markDirty(QSGNode::DirtyGeometry);
	return node;
}
