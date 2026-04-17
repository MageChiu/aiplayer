#!/usr/bin/env bash
set -euo pipefail

PROJECT_NAME="aiplayer"
BUILD_TYPE="Release"

if [[ "${1-}" == "--debug" ]]; then
  BUILD_TYPE="Debug"
elif [[ "${1-}" == "--release" || -z "${1-}" ]]; then
  BUILD_TYPE="Release"
else
  echo "用法: $0 [--release|--debug]" >&2
  exit 1
fi

command_exists() {
  command -v "$1" >/dev/null 2>&1
}

SRC_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DEPS_DIR="${SRC_DIR}/.deps"
VCPKG_ROOT="${VCPKG_ROOT:-${DEPS_DIR}/vcpkg}"

detect_vcpkg_triplet() {
  local arch
  arch="$(uname -m)"
  case "${arch}" in
    arm64) echo "arm64-osx" ;;
    x86_64) echo "x64-osx" ;;
    *)
      echo "[macOS] 不支持的架构: ${arch}" >&2
      exit 1
      ;;
  esac
}

VCPKG_TRIPLET="${VCPKG_TARGET_TRIPLET:-$(detect_vcpkg_triplet)}"
VCPKG_INSTALLED_DIR="${SRC_DIR}/vcpkg_installed/${VCPKG_TRIPLET}"

echo "[macOS] 检查依赖..."
MISSING=()

if ! command_exists cmake; then
  MISSING+=("cmake")
fi
if ! command_exists qtpaths6 && ! command_exists qmake6 && ! command_exists qtpaths; then
  MISSING+=("qt6 (qt@6)")
fi
if ! command_exists mpv; then
  MISSING+=("mpv")
fi
if ! command_exists pkg-config; then
  MISSING+=("pkg-config")
fi

if ((${#MISSING[@]} > 0)); then
  echo "[macOS] 检测到缺少以下依赖: ${MISSING[*]}" >&2
  echo "请使用 Homebrew 安装，例如:" >&2
  echo "  brew install cmake pkg-config mpv" >&2
  echo "  brew install qt@6" >&2
  echo "安装完成后请确保 Qt6 的 cmake 包可见，例如:" >&2
  echo "  export CMAKE_PREFIX_PATH=\"$(brew --prefix qt@6):${CMAKE_PREFIX_PATH-}\"" >&2
  exit 1
fi

echo "[macOS] 准备项目内依赖..."
"${SRC_DIR}/scripts/bootstrap_deps.sh"

export PKG_CONFIG_PATH="${VCPKG_INSTALLED_DIR}/lib/pkgconfig:${VCPKG_INSTALLED_DIR}/share/pkgconfig:${PKG_CONFIG_PATH-}"

# 推测 Qt6 CMake 路径
QT_CMAKE_PREFIX=""
MPV_INCLUDE_DIR=""
MPV_LIBRARY=""
if command_exists brew; then
  if brew list qt@6 >/dev/null 2>&1; then
    QT_PREFIX="$(brew --prefix qt@6)"
    QT_CMAKE_PREFIX="${QT_PREFIX}"
  fi
  if brew list mpv >/dev/null 2>&1; then
    MPV_PREFIX="$(brew --prefix mpv)"
    if [[ -f "${MPV_PREFIX}/include/mpv/client.h" ]]; then
      MPV_INCLUDE_DIR="${MPV_PREFIX}/include"
    fi
    if [[ -f "${MPV_PREFIX}/lib/libmpv.dylib" ]]; then
      MPV_LIBRARY="${MPV_PREFIX}/lib/libmpv.dylib"
    elif [[ -f "${MPV_PREFIX}/lib/libmpv.2.dylib" ]]; then
      MPV_LIBRARY="${MPV_PREFIX}/lib/libmpv.2.dylib"
    fi
  fi
fi

BUILD_DIR="${SRC_DIR}/build/macos"
DIST_DIR="${SRC_DIR}/dist/macos"

mkdir -p "${BUILD_DIR}" "${DIST_DIR}"

TOOLCHAIN_FILE="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
CACHE_FILE="${BUILD_DIR}/CMakeCache.txt"
if [[ -f "${CACHE_FILE}" ]]; then
  if ! grep -q "CMAKE_TOOLCHAIN_FILE:FILEPATH=${TOOLCHAIN_FILE}" "${CACHE_FILE}" 2>/dev/null || \
     ! grep -q "VCPKG_TARGET_TRIPLET:STRING=${VCPKG_TRIPLET}" "${CACHE_FILE}" 2>/dev/null; then
    echo "[macOS] 检测到旧的 CMake 缓存与当前依赖配置不一致，正在清理 ${BUILD_DIR}"
    rm -rf "${BUILD_DIR}"
    mkdir -p "${BUILD_DIR}"
  fi
fi

CMAKE_ARGS=(
  -S "${SRC_DIR}"
  -B "${BUILD_DIR}"
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"
  -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_FILE}"
  -DVCPKG_TARGET_TRIPLET="${VCPKG_TRIPLET}"
)

if [[ -n "${MPV_INCLUDE_DIR}" ]]; then
  CMAKE_ARGS+=("-DMPV_INCLUDE_DIR=${MPV_INCLUDE_DIR}")
fi
if [[ -n "${MPV_LIBRARY}" ]]; then
  CMAKE_ARGS+=("-DMPV_LIBRARY=${MPV_LIBRARY}")
fi

if [[ -n "${QT_CMAKE_PREFIX}" ]]; then
  if [[ -n "${CMAKE_PREFIX_PATH-}" ]]; then
    CMAKE_ARGS+=("-DCMAKE_PREFIX_PATH=${QT_CMAKE_PREFIX};${CMAKE_PREFIX_PATH};${VCPKG_INSTALLED_DIR}")
  else
    CMAKE_ARGS+=("-DCMAKE_PREFIX_PATH=${QT_CMAKE_PREFIX};${VCPKG_INSTALLED_DIR}")
  fi
fi

echo "[macOS] 运行 CMake 配置 (类型: ${BUILD_TYPE})..."
echo "[macOS] 使用 vcpkg triplet: ${VCPKG_TRIPLET}"
cmake "${CMAKE_ARGS[@]}"

echo "[macOS] 开始构建..."
cmake --build "${BUILD_DIR}" -j

APP_PATH="${BUILD_DIR}/${PROJECT_NAME}.app"
if [[ ! -d "${APP_PATH}" ]]; then
  # Qt 默认可能会用工程名/target 名，CMakeLists.txt 中 target 为 aiplayer
  APP_PATH="${BUILD_DIR}/AIPlayer.app"
fi

if [[ -d "${APP_PATH}" ]]; then
  DEST_APP="${DIST_DIR}/${PROJECT_NAME}.app"
  rm -rf "${DEST_APP}"
  cp -R "${APP_PATH}" "${DEST_APP}"
  echo "[macOS] 构建完成，应用已复制到: ${DEST_APP}"
  echo "[macOS] dist 目录: ${DIST_DIR}"
else
  echo "[macOS] 警告: 未找到 .app 产物，已完成编译但请检查构建输出目录: ${BUILD_DIR}" >&2
fi
