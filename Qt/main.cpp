#include <QGuiApplication>
#include <QQuickView>
#include <QOpenGLContext>
#include <QQmlEngine>
#include "qglview.h"
#include "render/raster.h"

int main(int argc, char *argv[])
{
	QGuiApplication app(argc, argv);

	qmlRegisterType<QGLView>("OpenGLUnderQML", 1, 0, "OpenGLView");

	QQuickView view;
	view.setSource(QUrl(QStringLiteral("qrc:///main.qml")));
	view.resize(800, 600);
	view.show();

	QOpenGLContext *glctx = view.openglContext();
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

	return app.exec();
}
