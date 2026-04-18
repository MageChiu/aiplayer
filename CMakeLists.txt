cmake_minimum_required(VERSION 3.21)
project(AIPlayer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Dependencies)

aiplayer_resolve_dependencies()

qt_standard_project_setup()

qt_add_executable(aiplayer
    src/main.cpp
    src/logcenter.cpp
    src/logwindow.cpp
    src/mainwindow.cpp
    src/localtranslationengine.cpp
    src/mpvwidget.cpp
    src/settingsdialog.cpp
    src/helpdialog.cpp
    resources/resources.qrc
)

# whisper.cpp 头文件路径
target_include_directories(aiplayer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

aiplayer_link_dependencies(aiplayer)

add_executable(aiplayer_translate_helper
    src/translationhelper.cpp
)
set_target_properties(aiplayer_translate_helper PROPERTIES
    OUTPUT_NAME "aiplayer-translate-helper"
)
target_include_directories(aiplayer_translate_helper PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
aiplayer_link_llama_helper(aiplayer_translate_helper)

if(APPLE)
    find_program(BREW_EXECUTABLE brew HINTS /opt/homebrew/bin /usr/local/bin)
    if(NOT BREW_EXECUTABLE)
        message(FATAL_ERROR "Homebrew not found. Please install brew and required dependencies.")
    endif()

    execute_process(
        COMMAND ${BREW_EXECUTABLE} --prefix mpv
        OUTPUT_VARIABLE MPV_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(NOT MPV_PREFIX)
        message(FATAL_ERROR "Failed to resolve Homebrew mpv prefix")
    endif()

    find_program(MACDEPLOYQT_EXECUTABLE macdeployqt
        HINTS
            $ENV{Qt6_DIR}/../../../bin
            $ENV{CMAKE_PREFIX_PATH}/bin
            /opt/homebrew/bin
            /usr/local/bin
    )
    if(NOT MACDEPLOYQT_EXECUTABLE)
        message(FATAL_ERROR "macdeployqt not found")
    endif()

    set_target_properties(aiplayer PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "AIPlayer"
        BUILD_RPATH "@executable_path/../Frameworks;@loader_path/../Frameworks"
        INSTALL_RPATH "@executable_path/../Frameworks;@loader_path/../Frameworks"
    )
    set_target_properties(aiplayer_translate_helper PROPERTIES
        BUILD_RPATH "@executable_path/../Frameworks;@loader_path/../Frameworks"
        INSTALL_RPATH "@executable_path/../Frameworks;@loader_path/../Frameworks"
    )

    add_custom_command(TARGET aiplayer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:aiplayer_translate_helper>" "$<TARGET_FILE_DIR:aiplayer>/"
        COMMAND "${MACDEPLOYQT_EXECUTABLE}" "$<TARGET_BUNDLE_DIR:aiplayer>" -always-overwrite -verbose=1
        COMMAND /bin/sh "${CMAKE_CURRENT_SOURCE_DIR}/scripts/fix_mpv_bundle.sh" "$<TARGET_BUNDLE_DIR:aiplayer>"
        COMMAND /usr/bin/codesign --force --deep --sign - --timestamp=none "$<TARGET_BUNDLE_DIR:aiplayer>"
        VERBATIM
    )
elseif(WIN32)
    set_target_properties(aiplayer PROPERTIES WIN32_EXECUTABLE TRUE)
    add_custom_command(TARGET aiplayer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:aiplayer_translate_helper>" "$<TARGET_FILE_DIR:aiplayer>"
        VERBATIM
    )
endif()

install(TARGETS aiplayer
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

install(TARGETS aiplayer_translate_helper
    RUNTIME DESTINATION bin
)
