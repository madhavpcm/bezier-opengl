include(openglwindow.pri)
CONFIG += console

SOURCES += \
    main.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/gui/openglwindow
INSTALLS += target

INCLUDEPATH += $$PWD/include

RESOURCES += \
    opengl.qrc
