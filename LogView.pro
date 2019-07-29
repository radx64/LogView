#-------------------------------------------------
#
# Project created by QtCreator 2019-07-09T17:45:00
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LogView
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++17

SOURCES += \
        src/Bookmark.cpp \
        src/BookmarksModel.cpp \
        src/LineNumberArea.cpp \
        src/LineNumberingBasedOnModelPolicy.cpp \
        src/MainWindow.cpp \
        src/main.cpp \
        src/TabCompositeViewer.cpp \
        src/TextRenderer.cpp \
        src/ViewerWidget.cpp

HEADERS += \
        src/Bookmark.hpp \
        src/BookmarksModel.hpp \
        src/GrepNode.hpp \
        src/ILineNumberingPolicy.hpp \
        src/LineNumberingBasedOnModelPolicy.hpp \
        src/Logfile.hpp \
        src/LineNumberArea.hpp \
        src/MainWindow.hpp \
        src/ProjectModel.hpp \
        src/TabCompositeViewer.hpp \
        src/TextRenderer.hpp \
        src/ViewerWidget.hpp

FORMS += \
    src/MainWindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    icons.qrc
