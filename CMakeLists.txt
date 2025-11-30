cmake_minimum_required(VERSION 3.31)
project(secrum LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        Network
        Sql
        REQUIRED)

file(GLOB_RECURSE SOURCES
        src/*.cpp
        src/*.h
        src/*.ui
)

add_executable(secrum ${SOURCES})
target_link_libraries(secrum
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
        Qt6::Sql
)

target_include_directories(secrum PRIVATE src)

# Platform specifics
if (WIN32)
    target_compile_definitions(secrum PRIVATE PLATFORM_WINDOWS)
elseif (APPLE)
    target_compile_definitions(secrum PRIVATE PLATFORM_MACOS)
elseif (UNIX)
    target_compile_definitions(secrum PRIVATE PLATFORM_LINUX)
endif ()