set(projectName "LogView")
set(version \"0.1.1\")

cmake_minimum_required(VERSION 3.16)
project (${projectName})
add_definitions(-DAPP_VERSION=${version})
add_definitions(-DGITHUB_REPO=\"radx64/LogView\")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Qt 6.6 is the minimum: the code uses Qt::ColorScheme (6.5+) and
# QList::assign (6.6+).
find_package(Qt6 6.6 REQUIRED COMPONENTS Core Gui Widgets Network)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/ui/AboutDialog.cpp
    src/UpdateChecker.cpp
    src/ui/UpdateDialog.cpp
    src/Bookmark.cpp
    src/BookmarksModel.cpp
    src/ui/BookmarkDialogWindow.cpp
    src/AutoMarking.cpp
    src/AutoMarkingsModel.cpp
    src/ui/AutoMarkingsWidget.cpp
    src/AutoBookmark.cpp
    src/AutoBookmarksModel.cpp
    src/ui/AutoBookmarksWidget.cpp
    src/Marking.cpp
    src/MarkingsModel.cpp
    src/ui/MarkingDialogWindow.cpp
    src/ui/ChunkedTextView.cpp
    src/ui/SearchWidget.cpp
    src/ui/GrepDialogWindow.cpp
    src/ui/MergerDialogWindow.cpp
    src/GrepNode.cpp
    src/LineSource.cpp
    src/FileLineSource.cpp
    src/FilteredLineSource.cpp
    src/ui/LogViewer.cpp
    src/ui/MainWindow.cpp
    src/ui/UiMainWindow.cpp
    src/Logfile.cpp
    src/ProjectUiManager.cpp
    src/ui/FileViewer.cpp
    src/ui/ThemeManager.cpp
    src/Settings.cpp
    src/Translator.cpp
    src/ui/SettingsDialog.cpp
    src/main.cpp
    src/ProjectModel.cpp
    src/loader/BackgroundTask.cpp
    src/loader/LoaderLogFile.cpp
    src/serializer/SerializerBookmark.cpp
    src/serializer/SerializerBookmarksModel.cpp
    src/serializer/SerializerMarking.cpp
    src/serializer/SerializerMarkingsModel.cpp
    src/serializer/SerializerGrepNode.cpp
    src/serializer/SerializerLogfile.cpp
    src/serializer/SerializerProjectModel.cpp
)

set(RESOURCES icons.qrc)

set(RC_FILE src/ui/app.rc)

include_directories(. src src/ui)

if(WIN32)
  set(CMAKE_WIN32_EXECUTABLE ON)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt6Widgets_EXECUTABLE_COMPILE_FLAGS}")

include_directories(${Qt6Widgets_INCLUDE_DIRS} ${QtQml_INCLUDE_DIRS})
add_definitions(${Qt6Widgets_DEFINITIONS} ${QtQml_DEFINITIONS} ${${Qt6Quick_DEFINITIONS}})

add_executable(${projectName} ${RESOURCES} ${RC_FILE} ${SOURCES})
target_link_libraries(${projectName} PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network)

# Ship the translation catalogs (lang/*.lang) next to the executable so they can
# be loaded at runtime relative to the binary.
add_custom_command(TARGET ${projectName} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/lang"
        "$<TARGET_FILE_DIR:${projectName}>/lang"
    COMMENT "Copying language catalogs")

# ---------------------------------------------------------------------------
# Static linking for release builds.
#
# Release executables should carry everything they need so they run on a clean
# machine without hunting for runtime DLLs/shared objects. The C++/compiler
# runtime (libgcc, libstdc++ and, on MinGW, winpthread) is always folded in.
# Qt itself can only be linked statically when the located Qt is a static
# build (e.g. an MXE *.static target or a custom static Qt); against a shared
# Qt a full "-static" link would fail, so we detect this and fall back to just
# a static runtime while Qt stays dynamic (and is bundled by the build script).
# ---------------------------------------------------------------------------
option(LOGVIEW_STATIC "Statically link release executables with their runtime/Qt" ON)

if(LOGVIEW_STATIC)
    get_target_property(_qt_core_type Qt6::Core TYPE)

    if(_qt_core_type STREQUAL "STATIC_LIBRARY")
        message(STATUS "LogView: static Qt detected -> fully static release executable")
        if(MINGW)
            # -static also pulls libgcc/libstdc++/winpthread in statically.
            target_link_options(${projectName} PRIVATE
                $<$<CONFIG:Release>:-static>)
        elseif(MSVC)
            # Fold the MSVC C runtime in (/MT). Only safe with a static Qt,
            # which is itself built against the static CRT; mixing /MT with a
            # shared (/MD) Qt would clash, so we never do this for shared Qt.
            set_property(TARGET ${projectName} PROPERTY
                MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_link_options(${projectName} PRIVATE
                $<$<CONFIG:Release>:-static-libgcc>
                $<$<CONFIG:Release>:-static-libstdc++>)
        endif()
        # Pull Qt's default plugins (platform, styles, image formats) into the
        # binary so it runs without an external plugins/ directory.
        if(COMMAND qt_import_plugins)
            qt_import_plugins(${projectName}
                INCLUDE_BY_TYPE platforms
                INCLUDE_BY_TYPE styles
                INCLUDE_BY_TYPE imageformats)
        endif()
    else()
        message(STATUS "LogView: shared Qt detected -> static C++ runtime only (Qt remains dynamic)")
        if(MINGW OR CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_link_options(${projectName} PRIVATE
                $<$<CONFIG:Release>:-static-libgcc>
                $<$<CONFIG:Release>:-static-libstdc++>)
        endif()
    endif()
endif()

# Strip symbols from MinGW release builds. A stripped binary looks more like a
# normal release artifact and helps reduce antivirus false positives that the
# unsigned, symbol-laden cross-compiled .exe tends to trigger.
if(MINGW)
    target_link_options(${projectName} PRIVATE $<$<CONFIG:Release>:-s>)
endif()
