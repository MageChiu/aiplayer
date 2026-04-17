#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DEPS_DIR="${ROOT_DIR}/.deps"
VCPKG_ROOT="${VCPKG_ROOT:-${DEPS_DIR}/vcpkg}"

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

cat <<EOF
[deps] 本地依赖已准备完成
  VCPKG_ROOT=${VCPKG_ROOT}
  VCPKG_TARGET_TRIPLET=${VCPKG_TRIPLET}
  CMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake
EOF
