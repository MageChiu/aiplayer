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

function(_aiplayer_require_library out_var)
    set(options)
    set(oneValueArgs NAME)
    set(multiValueArgs HINTS)
    cmake_parse_arguments(REQLIB "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    find_library(_found_library NAMES "${REQLIB_NAME}" HINTS ${REQLIB_HINTS} REQUIRED)
    set(${out_var} "${_found_library}" PARENT_SCOPE)
endfunction()

function(_aiplayer_apply_pkgconfig_static target_name prefix)
    target_include_directories(${target_name} INTERFACE ${${prefix}_INCLUDE_DIRS})
    if(DEFINED ${prefix}_STATIC_LIBRARY_DIRS)
        target_link_directories(${target_name} INTERFACE ${${prefix}_STATIC_LIBRARY_DIRS})
    else()
        target_link_directories(${target_name} INTERFACE ${${prefix}_LIBRARY_DIRS})
    endif()
    if(DEFINED ${prefix}_STATIC_LINK_LIBRARIES)
        target_link_libraries(${target_name} INTERFACE ${${prefix}_STATIC_LINK_LIBRARIES})
    else()
        target_link_libraries(${target_name} INTERFACE ${${prefix}_LINK_LIBRARIES})
    endif()

    set(_pkg_options)
    if(DEFINED ${prefix}_STATIC_LDFLAGS_OTHER)
        set(_pkg_options ${${prefix}_STATIC_LDFLAGS_OTHER})
    elseif(DEFINED ${prefix}_LDFLAGS_OTHER)
        set(_pkg_options ${${prefix}_LDFLAGS_OTHER})
    endif()

    if(_pkg_options)
        set(_normalized_link_options)
        set(_expect_framework_name FALSE)
        foreach(_opt IN LISTS _pkg_options)
            if(_expect_framework_name)
                list(APPEND _normalized_link_options "SHELL:-framework ${_opt}")
                set(_expect_framework_name FALSE)
            elseif(_opt STREQUAL "-framework")
                set(_expect_framework_name TRUE)
            else()
                list(APPEND _normalized_link_options "${_opt}")
            endif()
        endforeach()
        target_link_options(${target_name} INTERFACE ${_normalized_link_options})
    endif()
endfunction()

function(_aiplayer_try_mpv_root mpv_root)
    if(NOT mpv_root)
        return()
    endif()

    if(APPLE)
        set(_candidate_names libmpv libmpv.2 mpv mpv-2 libmpv-2)
    elseif(UNIX)
        set(_candidate_names libmpv mpv mpv-2 libmpv-2)
    else()
        set(_candidate_names mpv libmpv mpv-2 libmpv-2)
    endif()

    find_path(_mpv_include_dir NAMES mpv/client.h
        HINTS "${mpv_root}/include"
        NO_DEFAULT_PATH
    )
    find_library(_mpv_library
        NAMES ${_candidate_names}
        HINTS "${mpv_root}/lib"
        NO_DEFAULT_PATH
    )

    if(_mpv_include_dir AND _mpv_library)
        _aiplayer_create_manual_mpv_target(
            INCLUDE_DIR "${_mpv_include_dir}"
            LIBRARY "${_mpv_library}"
        )
    endif()
endfunction()

function(_aiplayer_resolve_mpv_non_windows)
    if(TARGET AIPlayer::MPV)
        return()
    endif()

    set(_default_mpv_root "${CMAKE_SOURCE_DIR}/.deps/mpv")
    if(DEFINED MPV_ROOT AND EXISTS "${MPV_ROOT}")
        _aiplayer_try_mpv_root("${MPV_ROOT}")
    elseif(EXISTS "${_default_mpv_root}")
        _aiplayer_try_mpv_root("${_default_mpv_root}")
    endif()

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
        set(_default_mpv_root "${CMAKE_SOURCE_DIR}/.deps/mpv")
        if(DEFINED MPV_ROOT AND EXISTS "${MPV_ROOT}")
            _aiplayer_try_mpv_root("${MPV_ROOT}")
        elseif(EXISTS "${_default_mpv_root}")
            _aiplayer_try_mpv_root("${_default_mpv_root}")
        endif()

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
        pkg_check_modules(AVFORMAT REQUIRED libavformat)
        pkg_check_modules(AVCODEC REQUIRED libavcodec)
        pkg_check_modules(AVUTIL REQUIRED libavutil)
        pkg_check_modules(SWRESAMPLE REQUIRED libswresample)
        _aiplayer_create_interface_target(AIPlayer_FFmpeg)
        _aiplayer_apply_pkgconfig_static(AIPlayer_FFmpeg AVFORMAT)
        _aiplayer_apply_pkgconfig_static(AIPlayer_FFmpeg AVCODEC)
        _aiplayer_apply_pkgconfig_static(AIPlayer_FFmpeg AVUTIL)
        _aiplayer_apply_pkgconfig_static(AIPlayer_FFmpeg SWRESAMPLE)
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
        pkg_check_modules(LIBTORRENT REQUIRED libtorrent-rasterbar)
        _aiplayer_create_interface_target(AIPlayer_Torrent)
        _aiplayer_apply_pkgconfig_static(AIPlayer_Torrent LIBTORRENT)
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
