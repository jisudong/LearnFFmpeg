QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    recordthread.cpp

HEADERS += \
    mainwindow.h \
    recordthread.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


macx {
    FFMPEG_HOEM = /usr/local/ffmpeg
    QMAKE_INFO_PLIST = mac/Info.plist
}

INCLUDEPATH += $${FFMPEG_HOEM}/include

LIBS += -L$${FFMPEG_HOEM}/lib \
        -lavdevice \
        -lavformat \
        -lavutil
