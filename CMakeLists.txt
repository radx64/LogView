set(projectName "LogView")
set(version 0.0.3)

cmake_minimum_required(VERSION 3.1.0)
project (${projectName})
add_definitions(VERSION=${version})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Gui REQUIRED)
find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++17")
set(SOURCES
    src/Bookmark.cpp
    src/BookmarksModel.cpp
    src/GrepDialogWindow.cpp
    src/GrepNode.cpp
    src/LineNumberArea.cpp
    src/LogViewer.cpp
    src/MainWindow.cpp
    src/Logfile.cpp
    src/ModelNumberingPolicy.cpp
    src/ProjectUiManager.cpp
    src/ProjectViewer.cpp
    src/main.cpp
    src/ProjectModel.cpp
    src/TextRenderer.cpp
    src/loader/Project.cpp
    src/loader/LoaderLogFile.cpp
    src/serializer/SerializerBookmark.cpp
    src/serializer/SerializerBookmarksModel.cpp
    src/serializer/SerializerGrepNode.cpp
    src/serializer/SerializerLogfile.cpp
    src/serializer/SerializerProjectModel.cpp
)

set(FORMS
    src/GrepDialogWindow.ui
    src/MainWindow.ui
)

set(RESOURCES icons.qrc)

set(RC_FILE src/app.rc)

include_directories(src)

add_executable(LogView ${SOURCES} ${FROMS} ${RESOURCES} ${RC_FILE})
target_link_libraries(LogView Qt5::Gui Qt5::Core Qt5::Widgets)
#target_link_libraries(LogView)
