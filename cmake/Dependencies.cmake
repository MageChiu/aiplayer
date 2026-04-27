include_guard(GLOBAL)

include(FetchContent)
include(ExternalProject)

option(AIPLAYER_ENABLE_TORRENT "Enable magnet/torrent playback via libtorrent" ON)
option(AIPLAYER_PKGCONFIG_STATIC "Use pkg-config static link flags for third-party libraries" OFF)

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

    set(_use_static_pkgconfig ${AIPLAYER_PKGCONFIG_STATIC})
    if(_use_static_pkgconfig AND DEFINED ${prefix}_STATIC_LIBRARY_DIRS)
        target_link_directories(${target_name} INTERFACE ${${prefix}_STATIC_LIBRARY_DIRS})
    else()
        target_link_directories(${target_name} INTERFACE ${${prefix}_LIBRARY_DIRS})
    endif()

    if(_use_static_pkgconfig AND DEFINED ${prefix}_STATIC_LINK_LIBRARIES)
        target_link_libraries(${target_name} INTERFACE ${${prefix}_STATIC_LINK_LIBRARIES})
    else()
        target_link_libraries(${target_name} INTERFACE ${${prefix}_LINK_LIBRARIES})
    endif()

    set(_pkg_options)
    if(_use_static_pkgconfig AND DEFINED ${prefix}_STATIC_LDFLAGS_OTHER)
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

function(_aiplayer_setup_llama_external)
    if(TARGET AIPlayer::Llama)
        return()
    endif()

    set(LLAMA_SOURCE_DIR "${CMAKE_BINARY_DIR}/_deps/llamacpp-src")
    set(LLAMA_SUBBUILD_DIR "${CMAKE_BINARY_DIR}/_deps/llamacpp-subbuild")
    set(LLAMA_BINARY_DIR "${CMAKE_BINARY_DIR}/_deps/llamacpp-build")
    set(LLAMA_INSTALL_DIR "${CMAKE_BINARY_DIR}/_deps/llamacpp-install")

    if(NOT EXISTS "${LLAMA_SOURCE_DIR}/include/llama.h")
        FetchContent_Declare(
            aiplayer_llamacpp_source
            GIT_REPOSITORY https://github.com/ggml-org/llama.cpp.git
            GIT_TAG        master
            GIT_SHALLOW    TRUE
            SOURCE_DIR     "${LLAMA_SOURCE_DIR}"
            SUBBUILD_DIR   "${LLAMA_SUBBUILD_DIR}"
        )
        FetchContent_Populate(aiplayer_llamacpp_source)
    endif()

    set(_llama_cmake_args
        -DCMAKE_INSTALL_PREFIX=${LLAMA_INSTALL_DIR}
        -DBUILD_SHARED_LIBS=ON
        -DLLAMA_BUILD_SERVER=OFF
        -DLLAMA_BUILD_EXAMPLES=OFF
        -DLLAMA_BUILD_TESTS=OFF
        -DLLAMA_BUILD_TOOLS=OFF
        -DLLAMA_BUILD_COMMON=OFF
        -DLLAMA_CURL=OFF
        -DGGML_CCACHE=OFF
    )

    if(CMAKE_BUILD_TYPE)
        list(APPEND _llama_cmake_args -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
    endif()
    if(CMAKE_TOOLCHAIN_FILE)
        list(APPEND _llama_cmake_args -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})
    endif()
    if(CMAKE_OSX_ARCHITECTURES)
        list(APPEND _llama_cmake_args -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES})
    endif()
    if(CMAKE_OSX_DEPLOYMENT_TARGET)
        list(APPEND _llama_cmake_args -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
    endif()

    if(WIN32)
        set(_llama_imported_location "${LLAMA_INSTALL_DIR}/bin/llama.dll")
        set(_llama_imported_implib   "${LLAMA_INSTALL_DIR}/lib/llama.lib")
    elseif(APPLE)
        set(_llama_imported_location "${LLAMA_INSTALL_DIR}/lib/libllama.dylib")
        unset(_llama_imported_implib)
    else()
        set(_llama_imported_location "${LLAMA_INSTALL_DIR}/lib/libllama.so")
        unset(_llama_imported_implib)
    endif()

    ExternalProject_Add(llamacpp_external
        PREFIX "${CMAKE_BINARY_DIR}/_deps/llamacpp"
        SOURCE_DIR "${LLAMA_SOURCE_DIR}"
        BINARY_DIR "${LLAMA_BINARY_DIR}"
        INSTALL_DIR "${LLAMA_INSTALL_DIR}"
        CMAKE_ARGS ${_llama_cmake_args}
        DOWNLOAD_COMMAND ""
        UPDATE_COMMAND ""
        BUILD_BYPRODUCTS "${_llama_imported_location}"
    )

    add_library(AIPlayer_Llama SHARED IMPORTED GLOBAL)
    set_target_properties(AIPlayer_Llama PROPERTIES
        IMPORTED_LOCATION "${_llama_imported_location}"
        INTERFACE_INCLUDE_DIRECTORIES "${LLAMA_SOURCE_DIR}/include;${LLAMA_SOURCE_DIR}/ggml/include"
    )
    if(WIN32)
        set_target_properties(AIPlayer_Llama PROPERTIES
            IMPORTED_IMPLIB "${_llama_imported_implib}"
        )
    endif()
    add_library(AIPlayer::Llama ALIAS AIPlayer_Llama)
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

    _aiplayer_setup_llama_external()

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

    _aiplayer_create_interface_target(AIPlayer_Torrent)
    set(_aiplayer_torrent_enabled OFF)
    if(AIPLAYER_ENABLE_TORRENT)
        find_package(LibtorrentRasterbar CONFIG QUIET)
        find_package(libtorrent CONFIG QUIET)
        if(TARGET LibtorrentRasterbar::torrent-rasterbar)
            target_link_libraries(AIPlayer_Torrent INTERFACE LibtorrentRasterbar::torrent-rasterbar)
            set(_aiplayer_torrent_enabled ON)
        elseif(TARGET libtorrent::libtorrent)
            target_link_libraries(AIPlayer_Torrent INTERFACE libtorrent::libtorrent)
            set(_aiplayer_torrent_enabled ON)
        else()
            find_package(PkgConfig QUIET)
            if(PkgConfig_FOUND)
                pkg_check_modules(LIBTORRENT QUIET libtorrent-rasterbar)
                if(LIBTORRENT_FOUND)
                    _aiplayer_apply_pkgconfig_static(AIPlayer_Torrent LIBTORRENT)
                    set(_aiplayer_torrent_enabled ON)
                endif()
            endif()
        endif()
    endif()

    if(_aiplayer_torrent_enabled)
        target_compile_definitions(AIPlayer_Torrent INTERFACE AIPLAYER_ENABLE_TORRENT=1)
        message(STATUS "Torrent support enabled")
    else()
        target_compile_definitions(AIPlayer_Torrent INTERFACE AIPLAYER_ENABLE_TORRENT=0)
        if(AIPLAYER_ENABLE_TORRENT)
            message(WARNING "libtorrent-rasterbar not found. Magnet/torrent playback will be disabled.")
        else()
            message(STATUS "Torrent support disabled by AIPLAYER_ENABLE_TORRENT=OFF")
        endif()
    endif()
    add_library(AIPlayer::Torrent ALIAS AIPlayer_Torrent)
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

function(aiplayer_link_llama_helper target_name)
    add_dependencies(${target_name} llamacpp_external)
    target_link_libraries(${target_name} PRIVATE
        Qt6::Core
        AIPlayer::Llama
    )
endfunction()
