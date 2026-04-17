#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
APP_PATH="${1:-$ROOT_DIR/build/aiplayer.app}"
BIN_PATH="$APP_PATH/Contents/MacOS/aiplayer"
FRAMEWORKS_DIR="$APP_PATH/Contents/Frameworks"
DIAG_DIR="$ROOT_DIR/diag"
DEPS_DIR="$ROOT_DIR/.deps"
MPV_ROOT="${MPV_ROOT:-$DEPS_DIR/mpv}"
VCPKG_ROOT="${VCPKG_ROOT:-$DEPS_DIR/vcpkg}"
VCPKG_INSTALLED_ROOT="${ROOT_DIR}/vcpkg_installed"
BREW_BIN="${HOMEBREW_PREFIX:+$HOMEBREW_PREFIX/bin/}brew"
if [[ ! -x "$BREW_BIN" ]]; then
  BREW_BIN="/opt/homebrew/bin/brew"
fi

FFMPEG_BREW_PREFIX=""
if [[ -x "$BREW_BIN" ]] && "$BREW_BIN" list ffmpeg >/dev/null 2>&1; then
  FFMPEG_BREW_PREFIX="$("$BREW_BIN" --prefix ffmpeg)"
fi

LIBMPV_SRC=""
if [[ -f "$MPV_ROOT/lib/libmpv.2.dylib" ]]; then
  LIBMPV_SRC="$MPV_ROOT/lib/libmpv.2.dylib"
elif [[ -f "$MPV_ROOT/lib/libmpv.dylib" ]]; then
  LIBMPV_SRC="$MPV_ROOT/lib/libmpv.dylib"
elif [[ -x "$BREW_BIN" ]] && "$BREW_BIN" list mpv >/dev/null 2>&1; then
  MPV_PREFIX="$("$BREW_BIN" --prefix mpv)"
  LIBMPV_SRC="$MPV_PREFIX/lib/libmpv.2.dylib"
fi

if [[ -z "$LIBMPV_SRC" || ! -f "$LIBMPV_SRC" ]]; then
  echo "missing libmpv source library in MPV_ROOT or Homebrew mpv" >&2
  exit 1
fi

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

bundle_dep_from_candidates() {
  local filename="$1"
  shift
  for src in "$@"; do
    if [[ -n "$src" && -f "$src" ]]; then
      cp -f "$src" "$FRAMEWORKS_DIR/$filename"
      install_name_tool -id "@rpath/$filename" "$FRAMEWORKS_DIR/$filename"
      return 0
    fi
  done
  echo "missing dependency candidates for: $filename" >&2
  printf '  %s\n' "$@" >&2
  exit 1
}

bundle_dep_glob() {
  local formula="$1"
  local pattern="$2"
  local formula_prefix
  formula_prefix="$($BREW_BIN --prefix "$formula")"
  local match
  match="$(find "$formula_prefix" -path "$formula_prefix/$pattern" -type f | head -n 1)"
  if [[ -z "$match" || ! -f "$match" ]]; then
    echo "missing dependency pattern: $formula_prefix/$pattern" >&2
    exit 1
  fi

  local filename
  filename="$(basename "$match")"
  cp -f "$match" "$FRAMEWORKS_DIR/$filename"
  install_name_tool -id "@rpath/$filename" "$FRAMEWORKS_DIR/$filename"
}

detect_vcpkg_triplet() {
  if [[ -d "$VCPKG_INSTALLED_ROOT/arm64-osx" ]]; then
    echo "arm64-osx"
    return
  fi
  if [[ -d "$VCPKG_INSTALLED_ROOT/x64-osx" ]]; then
    echo "x64-osx"
    return
  fi
}

VCPKG_TRIPLET="$(detect_vcpkg_triplet || true)"
VCPKG_LIB_DIR=""
if [[ -n "$VCPKG_TRIPLET" ]]; then
  VCPKG_LIB_DIR="$VCPKG_INSTALLED_ROOT/$VCPKG_TRIPLET/lib"
fi

bundle_dep libass lib/libass.9.dylib
bundle_dep_from_candidates libavcodec.62.dylib "$VCPKG_LIB_DIR/libavcodec.62.dylib" "${FFMPEG_BREW_PREFIX}/lib/libavcodec.62.dylib"
bundle_dep_from_candidates libavdevice.62.dylib "$VCPKG_LIB_DIR/libavdevice.62.dylib" "${FFMPEG_BREW_PREFIX}/lib/libavdevice.62.dylib"
bundle_dep_from_candidates libavfilter.11.dylib "$VCPKG_LIB_DIR/libavfilter.11.dylib" "${FFMPEG_BREW_PREFIX}/lib/libavfilter.11.dylib"
bundle_dep_from_candidates libavformat.62.dylib "$VCPKG_LIB_DIR/libavformat.62.dylib" "${FFMPEG_BREW_PREFIX}/lib/libavformat.62.dylib"
bundle_dep_from_candidates libavutil.60.dylib "$VCPKG_LIB_DIR/libavutil.60.dylib" "${FFMPEG_BREW_PREFIX}/lib/libavutil.60.dylib"
bundle_dep_from_candidates libswresample.6.dylib "$VCPKG_LIB_DIR/libswresample.6.dylib" "${FFMPEG_BREW_PREFIX}/lib/libswresample.6.dylib"
bundle_dep_from_candidates libswscale.9.dylib "$VCPKG_LIB_DIR/libswscale.9.dylib" "${FFMPEG_BREW_PREFIX}/lib/libswscale.9.dylib"
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

framework_bins = {}
for name in os.listdir(frameworks_dir):
    if not name.endswith('.framework'):
        continue
    bin_name = name[:-10]
    candidate = os.path.join(frameworks_dir, name, 'Versions', 'A', bin_name)
    if os.path.isfile(candidate):
        framework_bins[bin_name] = candidate
        subprocess.check_call(['install_name_tool', '-id', f'@rpath/{name}/Versions/A/{bin_name}', candidate])

plain_dylibs = {}
for name in os.listdir(frameworks_dir):
    candidate = os.path.join(frameworks_dir, name)
    if os.path.isfile(candidate) and name.endswith('.dylib'):
        plain_dylibs[name] = candidate
        subprocess.check_call(['install_name_tool', '-id', f'@rpath/{name}', candidate])

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

all_targets = [os.path.join(frameworks_dir, 'Python')]
all_targets.extend(queue)
all_targets.extend(plain_dylibs.values())
all_targets.extend(framework_bins.values())
all_targets.append(os.path.join(os.path.dirname(frameworks_dir), 'MacOS', 'aiplayer'))

for path in all_targets:
    if not os.path.exists(path):
        continue
    output = subprocess.check_output(['otool', '-L', path], text=True)
    for line in output.splitlines()[1:]:
        dep = line.strip().split(' ', 1)[0]
        dep_base = os.path.basename(dep)
        if dep_base in framework_bins and dep.startswith('/opt/homebrew/'):
            subprocess.check_call([
                'install_name_tool', '-change', dep,
                f'@rpath/{dep_base}.framework/Versions/A/{dep_base}', path
            ])
        elif dep_base in plain_dylibs and dep.startswith('/opt/homebrew/'):
            subprocess.check_call([
                'install_name_tool', '-change', dep,
                f'@rpath/{dep_base}', path
            ])

print(f'bundled {len(seen)} transitive Homebrew dependencies')
PY

codesign --force --deep --sign - --timestamp=none "$APP_PATH"

otool -L "$BIN_PATH" > "$DIAG_DIR/otool_aiplayer.txt"
otool -L "$FRAMEWORKS_DIR/libmpv.2.dylib" > "$DIAG_DIR/otool_libmpv.txt"
