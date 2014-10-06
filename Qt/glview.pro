TEMPLATE = app

QT += qml quick

SOURCES += main.cpp \
	qglview.cpp \
	qpixpad.cpp

RESOURCES += \
	qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
	qglview.h \
	qpixpad.h

unix|win32: LIBS += -L$$PWD/../3rd/lib/ -lImath-2_2

INCLUDEPATH += $$PWD/../3rd/include
DEPENDPATH += $$PWD/../3rd/include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/release/ -lpixpad
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/debug/ -lpixpad
else:unix: LIBS += -L$$PWD/../lib/ -lpixpad

INCLUDEPATH += $$PWD/..
DEPENDPATH += $$PWD/..

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../lib/release/libpixpad.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../lib/debug/libpixpad.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../lib/release/pixpad.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../lib/debug/pixpad.lib
else:unix: PRE_TARGETDEPS += $$PWD/../lib/libpixpad.a
