#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DEPS_DIR="${ROOT_DIR}/.deps"
VCPKG_ROOT="${VCPKG_ROOT:-${DEPS_DIR}/vcpkg}"
MPV_ROOT="${DEPS_DIR}/mpv"

command_exists() {
  command -v "$1" >/dev/null 2>&1
}

if ! command_exists git; then
  echo "[deps] 缺少 git，无法初始化本地依赖。" >&2
  exit 1
fi

if ! command_exists cmake; then
  echo "[deps] 缺少 cmake，请先安装 CMake。" >&2
  exit 1
fi

prepare_local_mpv() {
  local uname_s="$1"
  mkdir -p "${MPV_ROOT}/include" "${MPV_ROOT}/lib"

  if [[ -f "${MPV_ROOT}/include/mpv/client.h" ]]; then
    if compgen -G "${MPV_ROOT}/lib/libmpv*.dylib" >/dev/null || compgen -G "${MPV_ROOT}/lib/libmpv*.so*" >/dev/null; then
      echo "[deps] 已检测到本地 mpv 依赖: ${MPV_ROOT}"
      return 0
    fi
  fi

  if [[ "${uname_s}" == "Darwin" ]]; then
    if ! command_exists brew || ! brew list mpv >/dev/null 2>&1; then
      echo "[deps] 缺少 mpv。macOS 请先安装: brew install mpv" >&2
      return 1
    fi
    local mpv_prefix
    mpv_prefix="$(brew --prefix mpv)"
    rm -rf "${MPV_ROOT}/include/mpv"
    mkdir -p "${MPV_ROOT}/include"
    cp -R "${mpv_prefix}/include/mpv" "${MPV_ROOT}/include/"
    cp -f "${mpv_prefix}/lib/"libmpv*.dylib "${MPV_ROOT}/lib/" 2>/dev/null || true
    echo "[deps] 已将 Homebrew mpv 规整到 ${MPV_ROOT}"
    return 0
  fi

  if [[ "${uname_s}" == "Linux" ]]; then
    if ! command_exists pkg-config || ! pkg-config --exists mpv; then
      echo "[deps] 缺少 mpv 开发包。Linux 请先安装系统包并确保 pkg-config 可找到 mpv。" >&2
      return 1
    fi
    local include_dir lib_dir
    include_dir="$(pkg-config --cflags-only-I mpv | sed -n 's/^-I//p' | head -n 1)"
    lib_dir="$(pkg-config --libs-only-L mpv | sed -n 's/^-L//p' | head -n 1)"
    if [[ -z "${include_dir}" || -z "${lib_dir}" || ! -f "${include_dir}/mpv/client.h" ]]; then
      echo "[deps] 无法通过 pkg-config 解析 mpv 的 include/lib 路径。" >&2
      return 1
    fi
    rm -rf "${MPV_ROOT}/include/mpv"
    mkdir -p "${MPV_ROOT}/include"
    cp -R "${include_dir}/mpv" "${MPV_ROOT}/include/"
    cp -f "${lib_dir}/"libmpv.so* "${MPV_ROOT}/lib/" 2>/dev/null || true
    echo "[deps] 已将系统 mpv 规整到 ${MPV_ROOT}"
    return 0
  fi

  return 0
}

mkdir -p "${DEPS_DIR}"

if [[ ! -d "${VCPKG_ROOT}/.git" ]]; then
  echo "[deps] 初始化本地 vcpkg 到 ${VCPKG_ROOT}"
  git clone https://github.com/microsoft/vcpkg.git "${VCPKG_ROOT}"
fi

if [[ ! -x "${VCPKG_ROOT}/vcpkg" ]]; then
  echo "[deps] 启动 vcpkg bootstrap"
  "${VCPKG_ROOT}/bootstrap-vcpkg.sh" -disableMetrics
fi

UNAME_S="$(uname -s)"
UNAME_M="$(uname -m)"
case "${UNAME_S}:${UNAME_M}" in
  Darwin:arm64) VCPKG_TRIPLET="arm64-osx" ;;
  Darwin:x86_64) VCPKG_TRIPLET="x64-osx" ;;
  Linux:x86_64) VCPKG_TRIPLET="x64-linux" ;;
  Linux:aarch64) VCPKG_TRIPLET="arm64-linux" ;;
  *)
    echo "[deps] 暂不支持当前平台自动推导 vcpkg triplet: ${UNAME_S}/${UNAME_M}" >&2
    exit 1
    ;;
esac

if [[ -n "${VCPKG_TARGET_TRIPLET-}" ]]; then
  VCPKG_TRIPLET="${VCPKG_TARGET_TRIPLET}"
fi

echo "[deps] 使用 vcpkg triplet: ${VCPKG_TRIPLET}"
"${VCPKG_ROOT}/vcpkg" install --triplet "${VCPKG_TRIPLET}" --x-manifest-root="${ROOT_DIR}"
prepare_local_mpv "${UNAME_S}"

cat <<EOF
[deps] 本地依赖已准备完成
  VCPKG_ROOT=${VCPKG_ROOT}
  VCPKG_TARGET_TRIPLET=${VCPKG_TRIPLET}
  CMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake
  MPV_ROOT=${MPV_ROOT}
EOF
