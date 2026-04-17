#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"

"${ROOT_DIR}/scripts/bootstrap_deps.sh"

cat <<EOF
项目内依赖初始化完成。

后续推荐直接使用平台构建脚本：
  macOS:   ./scripts/build_macos.sh --release
  Linux:   ./scripts/build_linux.sh --release
  Windows: ./scripts/build_windows.ps1 --release

如果只想手动执行 CMake，可参考：
  export VCPKG_ROOT="${ROOT_DIR}/.deps/vcpkg"
  cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="\${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
  cmake --build build
EOF
