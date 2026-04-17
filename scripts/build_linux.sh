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
MPV_ROOT="${MPV_ROOT:-${DEPS_DIR}/mpv}"

echo "[Linux] 检查依赖..."
MISSING=()

if ! command_exists cmake; then
  MISSING+=("cmake")
fi
if ! command_exists qmake6 && ! command_exists qmake && ! command_exists qtpaths6 && ! command_exists qtpaths; then
  MISSING+=("Qt6 (开发包)")
fi
if ! command_exists mpv; then
  MISSING+=("mpv")
fi
if ! command_exists pkg-config; then
  MISSING+=("pkg-config")
fi

if ((${#MISSING[@]} > 0)); then
  echo "[Linux] 检测到缺少以下依赖: ${MISSING[*]}" >&2
  echo "Debian/Ubuntu 系可参考:" >&2
  echo "  sudo apt update" >&2
  echo "  sudo apt install cmake pkg-config mpv qt6-base-dev qt6-base-dev-tools" >&2
  echo "Arch/Manjaro 系可参考:" >&2
  echo "  sudo pacman -Syu cmake pkgconf mpv qt6-base" >&2
  exit 1
fi

echo "[Linux] 准备项目内依赖..."
"${SRC_DIR}/scripts/bootstrap_deps.sh"

BUILD_DIR="${SRC_DIR}/build/linux"
DIST_DIR="${SRC_DIR}/dist/linux"

mkdir -p "${BUILD_DIR}" "${DIST_DIR}"

CMAKE_ARGS=(
  -S "${SRC_DIR}"
  -B "${BUILD_DIR}"
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"
  -DCMAKE_TOOLCHAIN_FILE="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
  -DMPV_ROOT="${MPV_ROOT}"
)

echo "[Linux] 运行 CMake 配置 (类型: ${BUILD_TYPE})..."
cmake "${CMAKE_ARGS[@]}"

echo "[Linux] 开始构建..."
cmake --build "${BUILD_DIR}" -j

BIN_PATH="${BUILD_DIR}/${PROJECT_NAME}"
if [[ -x "${BIN_PATH}" ]]; then
  DEST_BIN="${DIST_DIR}/${PROJECT_NAME}"
  cp "${BIN_PATH}" "${DEST_BIN}"
  echo "[Linux] 构建完成，可执行文件已复制到: ${DEST_BIN}"
  echo "[Linux] dist 目录: ${DIST_DIR}"
else
  echo "[Linux] 警告: 未找到可执行文件，已完成编译但请检查构建输出目录: ${BUILD_DIR}" >&2
fi
