TEMPLATE = app

QT += qml quick

SOURCES += main.cpp \
	qglview.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
	qglview.h

unix|win32: LIBS += -L$$PWD/../lib/ -llibpixpad_Debug

INCLUDEPATH += $$PWD/../
DEPENDPATH += $$PWD/../

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../lib/libpixpad_Debug.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/../lib/liblibpixpad_Debug.a
