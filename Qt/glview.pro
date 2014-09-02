TEMPLATE = app

QT += qml quick

SOURCES += main.cpp \
	qglview.cpp \
    qmeshview.cpp

RESOURCES += \
    qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
	qglview.h \
    qmeshview.h


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/ -llibpixpad
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/ -llibpixpadd
else:unix: LIBS += -L$$PWD/../lib/ -llibpixpad

INCLUDEPATH += $$PWD/../
DEPENDPATH += $$PWD/../

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../lib/liblibpixpad.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../lib/liblibpixpadd.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../lib/libpixpad.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../lib/libpixpadd.lib
else:unix: PRE_TARGETDEPS += $$PWD/../lib/liblibpixpad.a
