#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
APP_PATH="${1:-$ROOT_DIR/build/aiplayer.app}"
BIN_PATH="$APP_PATH/Contents/MacOS/aiplayer"
FRAMEWORKS_DIR="$APP_PATH/Contents/Frameworks"
DIAG_DIR="$ROOT_DIR/diag"
BREW_BIN="${HOMEBREW_PREFIX:+$HOMEBREW_PREFIX/bin/}brew"
if [[ ! -x "$BREW_BIN" ]]; then
  BREW_BIN="/opt/homebrew/bin/brew"
fi

MPV_PREFIX="$($BREW_BIN --prefix mpv)"
LIBMPV_SRC="$MPV_PREFIX/lib/libmpv.2.dylib"

mkdir -p "$FRAMEWORKS_DIR" "$DIAG_DIR"
cp -f "$LIBMPV_SRC" "$FRAMEWORKS_DIR/"

install_name_tool -change "$LIBMPV_SRC" "@rpath/libmpv.2.dylib" "$BIN_PATH" || true
install_name_tool -id "@rpath/libmpv.2.dylib" "$FRAMEWORKS_DIR/libmpv.2.dylib"

bundle_dep() {
  local formula="$1"
  local relpath="$2"
  local filename
  filename="$(basename "$relpath")"
  local src
  src="$($BREW_BIN --prefix "$formula")/$relpath"

  if [[ ! -f "$src" ]]; then
    echo "missing dependency: $src" >&2
    exit 1
  fi

  cp -f "$src" "$FRAMEWORKS_DIR/$filename"
  install_name_tool -id "@rpath/$filename" "$FRAMEWORKS_DIR/$filename"
}

bundle_dep libass lib/libass.9.dylib
bundle_dep ffmpeg lib/libavcodec.62.dylib
bundle_dep ffmpeg lib/libavdevice.62.dylib
bundle_dep ffmpeg lib/libavfilter.11.dylib
bundle_dep ffmpeg lib/libavformat.62.dylib
bundle_dep ffmpeg lib/libavutil.60.dylib
bundle_dep ffmpeg lib/libswresample.6.dylib
bundle_dep ffmpeg lib/libswscale.9.dylib
bundle_dep libplacebo lib/libplacebo.360.dylib
bundle_dep mujs lib/libmujs.dylib
bundle_dep little-cms2 lib/liblcms2.2.dylib
bundle_dep libarchive lib/libarchive.13.dylib
bundle_dep libbluray lib/libbluray.3.dylib
bundle_dep luajit lib/libluajit-5.1.2.dylib
bundle_dep rubberband lib/librubberband.3.dylib
bundle_dep uchardet lib/libuchardet.0.dylib
bundle_dep vapoursynth lib/libvapoursynth-script.0.dylib
bundle_dep zimg lib/libzimg.2.dylib
bundle_dep jpeg-turbo lib/libjpeg.8.dylib
bundle_dep vulkan-loader lib/libvulkan.1.dylib

python3 - "$FRAMEWORKS_DIR" <<'PY'
import os
import shutil
import subprocess
import sys

frameworks_dir = sys.argv[1]
queue = [os.path.join(frameworks_dir, name) for name in os.listdir(frameworks_dir) if name.endswith('.dylib')]
seen = set()

while queue:
    path = queue.pop(0)
    output = subprocess.check_output(['otool', '-L', path], text=True)
    for line in output.splitlines()[1:]:
        dep = line.strip().split(' ', 1)[0]
        if not dep.startswith('/opt/homebrew/'):
            continue
        dep_name = os.path.basename(dep)
        candidate = os.path.join(frameworks_dir, dep_name)
        if not os.path.exists(candidate):
            if not os.path.isfile(dep):
                raise SystemExit(f'missing transitive dependency: {dep}')
            shutil.copy2(dep, candidate)
            subprocess.check_call(['install_name_tool', '-id', f'@rpath/{dep_name}', candidate])
            queue.append(candidate)
        subprocess.check_call(['install_name_tool', '-change', dep, f'@rpath/{dep_name}', path])
        seen.add(dep)

for name in os.listdir(frameworks_dir):
    if not (name.endswith('.dylib') or name == 'Python'):
        continue
    path = os.path.join(frameworks_dir, name)
    output = subprocess.check_output(['otool', '-L', path], text=True)
    for line in output.splitlines()[1:]:
        dep = line.strip().split(' ', 1)[0]
        if dep.startswith('/opt/homebrew/'):
            dep_name = os.path.basename(dep)
            candidate = os.path.join(frameworks_dir, dep_name)
            if os.path.exists(candidate):
                subprocess.check_call(['install_name_tool', '-change', dep, f'@rpath/{dep_name}', path])

print(f'bundled {len(seen)} transitive Homebrew dependencies')
PY

codesign --force --deep --sign - --timestamp=none "$APP_PATH"

otool -L "$BIN_PATH" > "$DIAG_DIR/otool_aiplayer.txt"
otool -L "$FRAMEWORKS_DIR/libmpv.2.dylib" > "$DIAG_DIR/otool_libmpv.txt"
