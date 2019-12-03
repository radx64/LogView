set(projectName "LogView")
set(version \"0.0.3\")

cmake_minimum_required(VERSION 3.9.0)
project (${projectName})
add_definitions(-DAPP_VERSION=${version})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS . src src/ui)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
    src/FileViewer.cpp
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
    src/ui/GrepDialogWindow.ui
    src/ui/MainWindow.ui
)

set(RESOURCES icons.qrc)

set(RC_FILE src/ui/app.rc)

include_directories(. src)

if(WIN32)
  set(CMAKE_WIN32_EXECUTABLE ON)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

include_directories(${Qt5Widgets_INCLUDE_DIRS} ${QtQml_INCLUDE_DIRS})
add_definitions(${Qt5Widgets_DEFINITIONS} ${QtQml_DEFINITIONS} ${${Qt5Quick_DEFINITIONS}})

add_executable(${projectName} ${FORMS} ${RESOURCES} ${RC_FILE} ${SOURCES})
target_link_libraries(${projectName} PUBLIC Qt5::Core Qt5::Gui Qt5::Widgets)
