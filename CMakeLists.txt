set(projectName "LogView")
set(version \"0.0.5\")

cmake_minimum_required(VERSION 3.16)
project (${projectName})
add_definitions(-DAPP_VERSION=${version})
add_definitions(-DGITHUB_REPO=\"radx64/LogView\")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS . src src/ui)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/AboutDialog.cpp
    src/UpdateChecker.cpp
    src/UpdateDialog.cpp
    src/Bookmark.cpp
    src/BookmarksModel.cpp
    src/BookmarkDialogWindow.cpp
    src/Marking.cpp
    src/MarkingsModel.cpp
    src/MarkingDialogWindow.cpp
    src/ChunkedTextView.cpp
    src/GrepDialogWindow.cpp
    src/GrepNode.cpp
    src/LineSource.cpp
    src/FileLineSource.cpp
    src/FilteredLineSource.cpp
    src/LogViewer.cpp
    src/MainWindow.cpp
    src/Logfile.cpp
    src/ProjectUiManager.cpp
    src/FileViewer.cpp
    src/ThemeManager.cpp
    src/Settings.cpp
    src/SettingsDialog.cpp
    src/main.cpp
    src/ProjectModel.cpp
    src/loader/Project.cpp
    src/loader/LoaderLogFile.cpp
    src/serializer/SerializerBookmark.cpp
    src/serializer/SerializerBookmarksModel.cpp
    src/serializer/SerializerMarking.cpp
    src/serializer/SerializerMarkingsModel.cpp
    src/serializer/SerializerGrepNode.cpp
    src/serializer/SerializerLogfile.cpp
    src/serializer/SerializerProjectModel.cpp
)

set(FORMS
    src/ui/BookmarkDialogWindow.ui
    src/ui/GrepDialogWindow.ui
    src/ui/MainWindow.ui
)

set(RESOURCES icons.qrc)

set(RC_FILE src/ui/app.rc)

include_directories(. src)

if(WIN32)
  set(CMAKE_WIN32_EXECUTABLE ON)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt6Widgets_EXECUTABLE_COMPILE_FLAGS}")

include_directories(${Qt6Widgets_INCLUDE_DIRS} ${QtQml_INCLUDE_DIRS})
add_definitions(${Qt6Widgets_DEFINITIONS} ${QtQml_DEFINITIONS} ${${Qt6Quick_DEFINITIONS}})

add_executable(${projectName} ${FORMS} ${RESOURCES} ${RC_FILE} ${SOURCES})
target_link_libraries(${projectName} PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network)
