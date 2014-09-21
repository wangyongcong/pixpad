#include <QQuickWindow>
#include <QSGSimpleRectNode>
#include <QOpenGLContext>
#include <QFile>
#include "qglview.h"

QGLView::QGLView(QQuickItem *parent) :
	QQuickItem(parent)
{
	setFlag(QQuickItem::ItemHasContents, true);
	connect(this, SIGNAL(windowChanged(QQuickWindow*)), this, SLOT(onWindowChanged(QQuickWindow*)), Qt::DirectConnection);
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
	qDebug("QGLView::onWindowChanged");
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
//	if(!this->initializeOpenGLFunctions()) {
//		qFatal("Failed to initialize OpenGL functions");
//	}
	QQuickWindow *win = window();
	Q_ASSERT(win);
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

void QGLView::glslError(GLuint shader)
{
	GLint ret;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &ret);
	if (ret<1) {
		qCritical("Shader compile error: Unknown");
		return;
	}
	ret += 1;
	char *info = new char[ret];
	glGetShaderInfoLog(shader, ret, &ret, info);
	qCritical("Shader compile error:\n%s", info);
	delete[] info;
}

GLuint QGLView::glslLoadSource(GLenum shader_type, const char *src, size_t length)
{
	GLuint shader = glCreateShader(shader_type);
	if (shader == 0) {
		return 0;
	}
	const GLchar *src_list[1] = { src };
	const GLint len_list[1] = { length };
	glShaderSource(shader, 1, src_list, len_list);
	glCompileShader(shader);
	GLint ret;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &ret);
	if (ret == GL_FALSE) {
		glslError(shader);
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

GLuint QGLView::glslBuildShader(GLuint *shaders, size_t count)
{
	GLuint program = glCreateProgram();
	for (size_t i = 0; i < count; ++i)
		glAttachShader(program, shaders[i]);
	glLinkProgram(program);
	GLint ret;
	glGetProgramiv(program, GL_LINK_STATUS, &ret);
	if (ret == GL_FALSE) {
		glslError(program);
		glDeleteProgram(program);
		return 0;
	}
	return program;
}

GLuint QGLView::glslLoadFile(GLenum shader_type, const QString &path)
{
	QFile file(path);
	if(!file.exists()) {
		qWarning("File not found: %s", qPrintable(path));
		return 0;
	}
	if(!file.open(QIODevice::ReadOnly))
		return 0;
	QByteArray data = file.readAll();
	file.close();
	GLuint shader = glslLoadSource(shader_type, data.data(), data.size());
	return shader;
}
