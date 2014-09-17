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

signals:

public slots:
	void onWindowChanged(QQuickWindow *win);
	void onSceneGraphInitialized();
	void onSceneGraphInvalidated();
	void onSync();
	void onRender();
	void onFrameEnd();

protected:
	void glslError(GLuint shader);
	GLuint glslLoadSource(GLenum shader_type, const char *src, size_t length);
	GLuint glslBuildShader(GLuint *shaders, size_t count);
	GLuint glslLoadFile(GLenum shader_type, const QString &path);
private:
	QTimer *m_updateTimer;
};

#endif // QGLVIEW_H
