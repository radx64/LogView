set(projectName "LogView")
set(version 0)

cmake_minimum_required(VERSION 3.9.0)
project (${projectName})
add_definitions(-DAPP_VERSION="1")
add_definitions(-DQT_STATIC)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS src)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

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

include_directories(${Qt5Widgets_INCLUDES})
add_definitions(${Qt5Widgets_DEFINITIONS})
include_directories(${Qt5Gui_INCLUDES})
add_definitions(${Qt5Gui_DEFINITIONS})
include_directories(${Qt5Core_INCLUDES})
add_definitions(${Qt5Core_DEFINITIONS})

if(WIN32)
  set(CMAKE_WIN32_EXECUTABLE ON)
endif()

qt5_wrap_ui(ui_wrap ${FORMS})

add_executable(LogView ${ui_wrap} ${RESOURCES} ${RC_FILE} ${SOURCES})
target_link_libraries(LogView PUBLIC Qt5::Core Qt5::Gui Qt5::Widgets)

# auto copy dlls if on windows
if(WIN32)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Core> $<TARGET_FILE_DIR:${PROJECT_NAME}>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Gui> $<TARGET_FILE_DIR:${PROJECT_NAME}>
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Widgets> $<TARGET_FILE_DIR:${PROJECT_NAME}> )
endif(WIN32)
