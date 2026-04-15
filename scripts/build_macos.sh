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
  echo "  export CMAKE_PREFIX_PATH=\"$(brew --prefix qt@6)\":$CMAKE_PREFIX_PATH" >&2
  exit 1
fi

# 推测 Qt6 CMake 路径
QT_CMAKE_PREFIX=""
if command_exists brew; then
  if brew list qt@6 >/dev/null 2>&1; then
    QT_PREFIX="$(brew --prefix qt@6)"
    QT_CMAKE_PREFIX="${QT_PREFIX}"
  fi
fi

SRC_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${SRC_DIR}/build/macos"
DIST_DIR="${SRC_DIR}/dist/macos"

mkdir -p "${BUILD_DIR}" "${DIST_DIR}"

CMAKE_ARGS=(
  -S "${SRC_DIR}"
  -B "${BUILD_DIR}"
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"
)

if [[ -n "${QT_CMAKE_PREFIX}" ]]; then
  CMAKE_ARGS+=("-DCMAKE_PREFIX_PATH=${QT_CMAKE_PREFIX}")
fi

echo "[macOS] 运行 CMake 配置 (类型: ${BUILD_TYPE})..."
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
