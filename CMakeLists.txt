cmake_minimum_required(VERSION 3.21)
project(AIPlayer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets)
find_package(PkgConfig REQUIRED)
pkg_check_modules(MPV REQUIRED IMPORTED_TARGET mpv)

include(FetchContent)

FetchContent_Declare(
    whispercpp
    GIT_REPOSITORY https://github.com/ggerganov/whisper.cpp.git
    GIT_TAG        master
)

# whisper.cpp 默认会尝试用 ccache；本机 ccache 动态库依赖异常会导致构建中断
set(GGML_CCACHE OFF CACHE BOOL "Disable ccache for ggml" FORCE)

FetchContent_MakeAvailable(whispercpp)

qt_standard_project_setup()

qt_add_executable(aiplayer
    src/main.cpp
    src/mainwindow.cpp
    src/mpvwidget.cpp
)

# whisper.cpp 头文件路径
target_include_directories(aiplayer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${whispercpp_SOURCE_DIR}
)

target_link_libraries(aiplayer PRIVATE
    Qt6::Widgets
    Qt6::OpenGLWidgets
    PkgConfig::MPV
    whisper
)

if(APPLE)
    set_target_properties(aiplayer PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "AIPlayer"
    )
endif()

install(TARGETS aiplayer
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
