#!/usr/bin/env bash
set -euo pipefail

if ! command -v brew >/dev/null 2>&1; then
  echo "Homebrew 未安装，请先安装 Homebrew: https://brew.sh/" >&2
  exit 1
fi

brew install qt mpv pkgconf cmake

QT_PREFIX="$(brew --prefix qt)"
MPV_PREFIX="$(brew --prefix mpv)"
PKGCONF_PREFIX="$(brew --prefix pkgconf)"

cat <<EOF
依赖安装完成。

建议在当前 shell 中执行：
  export CMAKE_PREFIX_PATH="${QT_PREFIX}:${CMAKE_PREFIX_PATH:-}"
  export PKG_CONFIG_PATH="${MPV_PREFIX}/lib/pkgconfig:${QT_PREFIX}/lib/pkgconfig:${PKGCONF_PREFIX}/lib/pkgconfig:${PKG_CONFIG_PATH:-}"

然后构建：
  cmake -S . -B build -DCMAKE_PREFIX_PATH="${QT_PREFIX}"
  cmake --build build
EOF
