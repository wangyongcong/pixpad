#include <QGuiApplication>
#include <QQuickView>
#include <QQmlEngine>
#include "qmeshview.h"
#include "render/raster.h"

int main(int argc, char *argv[])
{
	QGuiApplication app(argc, argv);

	const int major = 1;
	const int minor = 0;
	qmlRegisterType<QGLView>("QMLOpenGL", major, minor, "GLView");
	qmlRegisterType<QMeshView>("QMLOpenGL", major, minor, "MeshView");

	QQuickView view;
	view.setSource(QUrl(QStringLiteral("qrc:///main.qml")));

	QObject *root = view.rootObject();
	root->connect(root, SIGNAL(vertsChanged(int, int, int)), root, SLOT(onVertsChanged(int, int, int)));

	view.resize(800, 600);
	view.show();

	return app.exec();
}
