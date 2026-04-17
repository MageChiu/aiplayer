include_guard(GLOBAL)

include(FetchContent)

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets Network)

function(_aiplayer_create_manual_mpv_target)
    if(TARGET AIPlayer::MPV)
        return()
    endif()

    set(oneValueArgs INCLUDE_DIR LIBRARY)
    cmake_parse_arguments(MPV "" "${oneValueArgs}" "" ${ARGN})

    if(NOT MPV_INCLUDE_DIR OR NOT MPV_LIBRARY)
        message(FATAL_ERROR "Manual MPV target requires INCLUDE_DIR and LIBRARY")
    endif()

    add_library(AIPlayer_MPV UNKNOWN IMPORTED)
    set_target_properties(AIPlayer_MPV PROPERTIES
        IMPORTED_LOCATION "${MPV_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${MPV_INCLUDE_DIR}"
    )
    add_library(AIPlayer::MPV ALIAS AIPlayer_MPV)
endfunction()

function(_aiplayer_create_interface_target target_name)
    if(NOT TARGET ${target_name})
        add_library(${target_name} INTERFACE IMPORTED)
    endif()
endfunction()

function(_aiplayer_resolve_mpv_non_windows)
    if(TARGET AIPlayer::MPV)
        return()
    endif()

    if(DEFINED MPV_INCLUDE_DIR AND DEFINED MPV_LIBRARY AND EXISTS "${MPV_INCLUDE_DIR}" AND EXISTS "${MPV_LIBRARY}")
        _aiplayer_create_manual_mpv_target(INCLUDE_DIR "${MPV_INCLUDE_DIR}" LIBRARY "${MPV_LIBRARY}")
        return()
    endif()

    find_path(AIPLAYER_MPV_INCLUDE_DIR NAMES mpv/client.h)
    find_library(AIPLAYER_MPV_LIBRARY NAMES mpv libmpv mpv-2 libmpv-2)
    if(AIPLAYER_MPV_INCLUDE_DIR AND AIPLAYER_MPV_LIBRARY)
        _aiplayer_create_manual_mpv_target(
            INCLUDE_DIR "${AIPLAYER_MPV_INCLUDE_DIR}"
            LIBRARY "${AIPLAYER_MPV_LIBRARY}"
        )
        return()
    endif()

    if(APPLE)
        find_program(BREW_EXECUTABLE brew HINTS /opt/homebrew/bin /usr/local/bin)
        if(BREW_EXECUTABLE)
            execute_process(
                COMMAND ${BREW_EXECUTABLE} --prefix mpv
                OUTPUT_VARIABLE AIPLAYER_MPV_PREFIX
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET
            )
            if(AIPLAYER_MPV_PREFIX)
                find_path(AIPLAYER_MPV_INCLUDE_DIR NAMES mpv/client.h
                    HINTS "${AIPLAYER_MPV_PREFIX}/include"
                )
                find_library(AIPLAYER_MPV_LIBRARY NAMES mpv libmpv mpv-2 libmpv-2
                    HINTS "${AIPLAYER_MPV_PREFIX}/lib"
                )
                if(AIPLAYER_MPV_INCLUDE_DIR AND AIPLAYER_MPV_LIBRARY)
                    _aiplayer_create_manual_mpv_target(
                        INCLUDE_DIR "${AIPLAYER_MPV_INCLUDE_DIR}"
                        LIBRARY "${AIPLAYER_MPV_LIBRARY}"
                    )
                    return()
                endif()
            endif()
        endif()
    endif()

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(MPV REQUIRED IMPORTED_TARGET mpv)
    add_library(AIPlayer::MPV ALIAS PkgConfig::MPV)
endfunction()

function(aiplayer_resolve_dependencies)
    if(NOT TARGET whisper)
        FetchContent_Declare(
            whispercpp
            GIT_REPOSITORY https://github.com/ggerganov/whisper.cpp.git
            GIT_TAG        master
        )

        # whisper.cpp 默认会尝试用 ccache；本机 ccache 动态库依赖异常会导致构建中断
        set(GGML_CCACHE OFF CACHE BOOL "Disable ccache for ggml" FORCE)
        FetchContent_MakeAvailable(whispercpp)
    endif()

    if(WIN32)
        if(DEFINED MPV_INCLUDE_DIR AND DEFINED MPV_LIBRARY AND EXISTS "${MPV_INCLUDE_DIR}" AND EXISTS "${MPV_LIBRARY}")
            _aiplayer_create_manual_mpv_target(INCLUDE_DIR "${MPV_INCLUDE_DIR}" LIBRARY "${MPV_LIBRARY}")
        else()
            find_path(AIPLAYER_MPV_INCLUDE_DIR NAMES mpv/client.h)
            find_library(AIPLAYER_MPV_LIBRARY NAMES mpv libmpv mpv-2 libmpv-2)
            if(AIPLAYER_MPV_INCLUDE_DIR AND AIPLAYER_MPV_LIBRARY)
                _aiplayer_create_manual_mpv_target(
                    INCLUDE_DIR "${AIPLAYER_MPV_INCLUDE_DIR}"
                    LIBRARY "${AIPLAYER_MPV_LIBRARY}"
                )
            else()
                find_package(PkgConfig REQUIRED)
                pkg_check_modules(MPV REQUIRED IMPORTED_TARGET mpv)
                add_library(AIPlayer::MPV ALIAS PkgConfig::MPV)
            endif()
        endif()
    else()
        _aiplayer_resolve_mpv_non_windows()
    endif()

    if(WIN32)
        find_package(FFMPEG REQUIRED)
        _aiplayer_create_interface_target(AIPlayer_FFmpeg)
        target_include_directories(AIPlayer_FFmpeg INTERFACE ${FFMPEG_INCLUDE_DIRS})
        target_link_directories(AIPlayer_FFmpeg INTERFACE ${FFMPEG_LIBRARY_DIRS})
        target_link_libraries(AIPlayer_FFmpeg INTERFACE ${FFMPEG_LIBRARIES})
        add_library(AIPlayer::FFmpeg ALIAS AIPlayer_FFmpeg)
    else()
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(AVFORMAT REQUIRED IMPORTED_TARGET libavformat)
        pkg_check_modules(AVCODEC REQUIRED IMPORTED_TARGET libavcodec)
        pkg_check_modules(AVUTIL REQUIRED IMPORTED_TARGET libavutil)
        pkg_check_modules(SWRESAMPLE REQUIRED IMPORTED_TARGET libswresample)
        _aiplayer_create_interface_target(AIPlayer_FFmpeg)
        target_link_libraries(AIPlayer_FFmpeg INTERFACE
            PkgConfig::AVFORMAT
            PkgConfig::AVCODEC
            PkgConfig::AVUTIL
            PkgConfig::SWRESAMPLE
        )
        add_library(AIPlayer::FFmpeg ALIAS AIPlayer_FFmpeg)
    endif()

    find_package(LibtorrentRasterbar CONFIG QUIET)
    find_package(libtorrent CONFIG QUIET)
    if(TARGET LibtorrentRasterbar::torrent-rasterbar)
        _aiplayer_create_interface_target(AIPlayer_Torrent)
        target_link_libraries(AIPlayer_Torrent INTERFACE LibtorrentRasterbar::torrent-rasterbar)
        add_library(AIPlayer::Torrent ALIAS AIPlayer_Torrent)
    elseif(TARGET libtorrent::libtorrent)
        _aiplayer_create_interface_target(AIPlayer_Torrent)
        target_link_libraries(AIPlayer_Torrent INTERFACE libtorrent::libtorrent)
        add_library(AIPlayer::Torrent ALIAS AIPlayer_Torrent)
    else()
        if(WIN32)
            message(FATAL_ERROR "libtorrent not found. Please bootstrap local dependencies first.")
        endif()
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(LIBTORRENT REQUIRED IMPORTED_TARGET libtorrent-rasterbar)
        _aiplayer_create_interface_target(AIPlayer_Torrent)
        target_link_libraries(AIPlayer_Torrent INTERFACE PkgConfig::LIBTORRENT)
        add_library(AIPlayer::Torrent ALIAS AIPlayer_Torrent)
    endif()
endfunction()

function(aiplayer_link_dependencies target_name)
    target_link_libraries(${target_name} PRIVATE
        Qt6::Widgets
        Qt6::OpenGLWidgets
        Qt6::Network
        AIPlayer::MPV
        AIPlayer::FFmpeg
        AIPlayer::Torrent
        whisper
    )
endfunction()
