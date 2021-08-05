include(openglwindow.pri)

SOURCES += \
    main.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/gui/openglwindow
INSTALLS += target

DISTFILES += \
    shaders/fragment.frag \
    shaders/vertex.vert
