#include <QGuiApplication>
#include <QScreen>
#include <QQuickView>
#include <QQmlEngine>
#include "qpixpad.h"
#include "render/raster.h"

int main(int argc, char *argv[])
{
	QGuiApplication app(argc, argv);

	const int major = 1;
	const int minor = 0;
	qmlRegisterType<QGLView>("QMLOpenGL", major, minor, "GLView");
	qmlRegisterType<QPixpad>("QMLOpenGL", major, minor, "Pixpad");

	QQuickView view;
	view.setSource(QUrl(QStringLiteral("qrc:///main.qml")));
	QScreen *screen = QGuiApplication::primaryScreen();
	QSize sz = screen->size() / 2;
	view.resize(sz);
	view.setMinimumSize(sz);
	view.setMaximumSize(sz);
	view.show();

//	QObject *root = view.rootObject();

	return app.exec();
}
