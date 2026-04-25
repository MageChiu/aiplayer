#!/usr/bin/env bash
set -euo pipefail

BUILD_TYPE="Release"
PACKAGE_KIND="apk"

usage() {
  cat <<'EOF'
用法: scripts/build_android.sh [--release|--debug] [--apk|--bundle]

说明:
  --release    构建 Release 产物（默认）
  --debug      构建 Debug 产物
  --apk        输出 APK（默认）
  --bundle     输出 AAB
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --release)
      BUILD_TYPE="Release"
      ;;
    --debug)
      BUILD_TYPE="Debug"
      ;;
    --apk)
      PACKAGE_KIND="apk"
      ;;
    --bundle)
      PACKAGE_KIND="bundle"
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "[Android] 未知参数: $1" >&2
      usage >&2
      exit 1
      ;;
  esac
  shift
done

command_exists() {
  command -v "$1" >/dev/null 2>&1
}

SRC_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ANDROID_PROJECT_DIR="${SRC_DIR}/apps/android"
DIST_DIR="${SRC_DIR}/dist/android"
MODULE_NAME="app"
TASK_NAME=""
OUTPUT_GLOB=""

if [[ "${PACKAGE_KIND}" == "bundle" ]]; then
  if [[ "${BUILD_TYPE}" == "Debug" ]]; then
    echo "[Android] AAB 通常只用于 Release，Debug 模式请改用 --apk" >&2
    exit 1
  fi
  TASK_NAME=":${MODULE_NAME}:bundleRelease"
  OUTPUT_GLOB="${ANDROID_PROJECT_DIR}/${MODULE_NAME}/build/outputs/bundle/release/*.aab"
else
  if [[ "${BUILD_TYPE}" == "Debug" ]]; then
    TASK_NAME=":${MODULE_NAME}:assembleDebug"
    OUTPUT_GLOB="${ANDROID_PROJECT_DIR}/${MODULE_NAME}/build/outputs/apk/debug/*.apk"
  else
    TASK_NAME=":${MODULE_NAME}:assembleRelease"
    OUTPUT_GLOB="${ANDROID_PROJECT_DIR}/${MODULE_NAME}/build/outputs/apk/release/*.apk"
  fi
fi

GRADLE_CMD=""
if [[ -x "${ANDROID_PROJECT_DIR}/gradlew" ]]; then
  GRADLE_CMD="${ANDROID_PROJECT_DIR}/gradlew"
elif command_exists gradle; then
  GRADLE_CMD="gradle"
else
  echo "[Android] 未检测到 gradlew 或 gradle，请先安装 Gradle 或补充 Gradle Wrapper。" >&2
  exit 1
fi

MISSING=()
if ! command_exists java; then
  MISSING+=("java")
fi
if [[ -z "${ANDROID_SDK_ROOT:-}" && -z "${ANDROID_HOME:-}" && ! -f "${ANDROID_PROJECT_DIR}/local.properties" && ! -f "${SRC_DIR}/local.properties" ]]; then
  MISSING+=("ANDROID_SDK_ROOT/ANDROID_HOME 或 local.properties")
fi

if ((${#MISSING[@]} > 0)); then
  echo "[Android] 检测到缺少以下依赖: ${MISSING[*]}" >&2
  echo "[Android] 请先准备 JDK 和 Android SDK，再执行本脚本。" >&2
  exit 1
fi

mkdir -p "${DIST_DIR}"

echo "[Android] 工程入口: ${ANDROID_PROJECT_DIR}"
echo "[Android] 使用构建类型: ${BUILD_TYPE}"
echo "[Android] 使用产物类型: ${PACKAGE_KIND}"
echo "[Android] 执行任务: ${TASK_NAME}"

if [[ ! -d "${ANDROID_PROJECT_DIR}" ]]; then
  echo "[Android] 未找到 Android 工程入口目录: ${ANDROID_PROJECT_DIR}" >&2
  exit 1
fi

if [[ "${GRADLE_CMD}" == "${ANDROID_PROJECT_DIR}/gradlew" ]]; then
  (cd "${ANDROID_PROJECT_DIR}" && "${GRADLE_CMD}" "${TASK_NAME}")
else
  (cd "${ANDROID_PROJECT_DIR}" && "${GRADLE_CMD}" "${TASK_NAME}")
fi

shopt -s nullglob
OUTPUTS=( ${OUTPUT_GLOB} )
shopt -u nullglob

if ((${#OUTPUTS[@]} == 0)); then
  echo "[Android] 构建完成，但未找到预期产物，请检查输出目录。" >&2
  exit 1
fi

for file in "${OUTPUTS[@]}"; do
  cp "${file}" "${DIST_DIR}/"
done

echo "[Android] 构建完成，产物已复制到: ${DIST_DIR}"
