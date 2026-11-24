# Unit tests (Qt Test). Built into the "LogViewTests" target and registered
# with CTest. Works with the system libarchive on Linux and the bundled static
# libarchive (zlib + liblzma) on Windows.
find_package(Qt6 6.6 REQUIRED COMPONENTS Test)

add_executable(LogViewTests
    ArchiveFormatsTest.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/ArchiveCatalog.cpp
    ${CMAKE_SOURCE_DIR}/src/archive/ArchiveCommon.cpp
    ${CMAKE_SOURCE_DIR}/src/Translator.cpp
)

target_include_directories(LogViewTests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_compile_definitions(LogViewTests PRIVATE
    LOGVIEW_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
target_link_libraries(LogViewTests PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Test ${LOGVIEW_LIBARCHIVE_TARGET})

if(LOGVIEW_LIBARCHIVE_STATIC)
    target_compile_definitions(LogViewTests PRIVATE LIBARCHIVE_STATIC)
endif()

add_test(NAME LogViewTests COMMAND LogViewTests)
