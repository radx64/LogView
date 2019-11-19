#-------------------------------------------------
#
# Project created by QtCreator 2019-07-09T17:45:00
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LogView
TEMPLATE = app
VERSION = 0.0.2

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += \
        QT_DEPRECATED_WARNINGS \
        APP_VERSION=\\\"$$VERSION\\\"
# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++17

SOURCES += \
        src/Bookmark.cpp \
        src/BookmarksModel.cpp \
        src/GrepDialogWindow.cpp \
        src/GrepNode.cpp \
        src/LineNumberArea.cpp \
        src/LogViewer.cpp \
        src/MainWindow.cpp \
        src/Logfile.cpp \
        src/ModelNumberingPolicy.cpp \
        src/ProjectViewer.cpp \
        src/main.cpp \
        src/ProjectManager.cpp \
        src/ProjectModel.cpp \
        src/TextRenderer.cpp \
        src/loader/Project.cpp \
        src/loader/LoaderLogFile.cpp \
        src/serializer/SerializerBookmark.cpp \
        src/serializer/SerializerBookmarksModel.cpp \
        src/serializer/SerializerGrepNode.cpp \
        src/serializer/SerializerLogfile.cpp \
        src/serializer/SerializerProjectModel.cpp \

HEADERS += \
        src/Bookmark.hpp \
        src/BookmarksModel.hpp \
        src/GrepDialogWindow.hpp \
        src/GrepNode.hpp \
        src/ILineNumberingPolicy.hpp \
        src/LogViewer.hpp \
        src/Logfile.hpp \
        src/LineNumberArea.hpp \
        src/MainWindow.hpp \
        src/ModelNumberingPolicy.hpp \
        src/ProjectModel.hpp \
        src/ProjectUiManager.hpp \
        src/ProjectViewer.hpp \
        src/TextRenderer.hpp \
        src/loader/Project.hpp \
        src/loader/LoaderLogFile.hpp \
        src/serializer/SerializerBookmark.hpp \
        src/serializer/SerializerBookmarksModel.hpp \
        src/serializer/SerializerGrepNode.hpp \
        src/serializer/SerializerLogfile.hpp \
        src/serializer/SerializerProjectModel.hpp

FORMS += \
    src/GrepDialogWindow.ui \
    src/MainWindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    icons.qrc

RC_FILE = src/app.rc
