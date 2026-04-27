#!/usr/bin/env bash
set -euo pipefail

BUILD_TYPE="Release"
DESTINATION="simulator"

usage() {
  cat <<'EOF'
用法: scripts/build_ios.sh [--release|--debug] [--simulator|--device]

说明:
  --release      构建 Release 产物（默认）
  --debug        构建 Debug 产物
  --simulator    构建 iOS Simulator 产物（默认，适合外部校验）
  --device       构建设备产物，需要本机签名环境
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
    --simulator)
      DESTINATION="simulator"
      ;;
    --device)
      DESTINATION="device"
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "[iOS] 未知参数: $1" >&2
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
IOS_PROJECT_DIR="${SRC_DIR}/apps/ios/AIPlayerIOS"
SPEC_FILE="${IOS_PROJECT_DIR}/project.yml"
PROJECT_NAME="AIPlayerIOS"
SCHEME_NAME="AIPlayerIOS"
DERIVED_DATA_DIR="${SRC_DIR}/build/ios"
DIST_DIR="${SRC_DIR}/dist/ios"
XCODEPROJ_PATH="${IOS_PROJECT_DIR}/${PROJECT_NAME}.xcodeproj"

MISSING=()
if ! command_exists xcodebuild; then
  MISSING+=("xcodebuild")
fi
if ! command_exists xcodegen; then
  MISSING+=("xcodegen")
fi

if ((${#MISSING[@]} > 0)); then
  echo "[iOS] 检测到缺少以下依赖: ${MISSING[*]}" >&2
  echo "[iOS] 可参考安装: brew install xcodegen" >&2
  echo "[iOS] 同时需要安装 Xcode 和对应 Command Line Tools。" >&2
  exit 1
fi

ACTIVE_DEVELOPER_DIR="$(xcode-select -p 2>/dev/null || true)"
if [[ -z "${ACTIVE_DEVELOPER_DIR}" || "${ACTIVE_DEVELOPER_DIR}" == "/Library/Developer/CommandLineTools" ]]; then
  echo "[iOS] 当前 xcode-select 指向的不是完整 Xcode: ${ACTIVE_DEVELOPER_DIR:-<empty>}" >&2
  echo "[iOS] 请先执行以下命令后再重试:" >&2
  echo "  sudo xcode-select -s /Applications/Xcode.app/Contents/Developer" >&2
  exit 1
fi

if ! xcodebuild -checkFirstLaunchStatus >/dev/null 2>&1; then
  echo "[iOS] 当前 Xcode 处于未完成初始化或系统框架不一致状态。" >&2
  echo "[iOS] 请按顺序尝试以下命令后再重试:" >&2
  echo "  sudo xcodebuild -runFirstLaunch" >&2
  echo "  sudo xcodebuild -license accept" >&2
  echo "[iOS] 如果仍失败，说明 Xcode 或 Command Line Tools 安装不完整，建议重新安装或更新到匹配版本。" >&2
  exit 1
fi

if [[ ! -f "${SPEC_FILE}" ]]; then
  echo "[iOS] 未找到工程描述文件: ${SPEC_FILE}" >&2
  exit 1
fi

if [[ ! -d "${IOS_PROJECT_DIR}" ]]; then
  echo "[iOS] 未找到 iOS 工程入口目录: ${IOS_PROJECT_DIR}" >&2
  exit 1
fi

mkdir -p "${DERIVED_DATA_DIR}" "${DIST_DIR}"

echo "[iOS] 工程入口: ${IOS_PROJECT_DIR}"
echo "[iOS] 生成 Xcode 工程..."
(cd "${IOS_PROJECT_DIR}" && xcodegen generate --spec "${SPEC_FILE}")

SDK_NAME="iphonesimulator"
DESTINATION_ARG="generic/platform=iOS Simulator"
PRODUCT_DIR_SUFFIX="${BUILD_TYPE}-iphonesimulator"
EXTRA_ARGS=(CODE_SIGNING_ALLOWED=NO)

if [[ "${DESTINATION}" == "device" ]]; then
  SDK_NAME="iphoneos"
  DESTINATION_ARG="generic/platform=iOS"
  PRODUCT_DIR_SUFFIX="${BUILD_TYPE}-iphoneos"
  EXTRA_ARGS=()
fi

if [[ "${DESTINATION}" == "simulator" ]]; then
  RUNTIMES_OUTPUT="$(xcrun simctl list runtimes 2>/dev/null || true)"
  DEVICES_OUTPUT="$(xcrun simctl list devices available 2>/dev/null || true)"

  if ! grep -q "iOS" <<<"${RUNTIMES_OUTPUT}"; then
    echo "[iOS] 当前未检测到任何 iOS Simulator runtime。" >&2
    echo "[iOS] 请先在 Xcode > Settings > Components 中安装 iOS Simulator 平台。" >&2
    echo "[iOS] 安装后可再次执行: ./scripts/build_ios.sh" >&2
    exit 1
  fi

  if ! grep -q "(" <<<"${DEVICES_OUTPUT}"; then
    echo "[iOS] 当前未检测到任何可用 iOS Simulator 设备。" >&2
    echo "[iOS] 请先打开 Xcode 的 Devices and Simulators 创建一个模拟器，或安装对应 runtime 后重试。" >&2
    exit 1
  fi
fi

echo "[iOS] 使用构建类型: ${BUILD_TYPE}"
echo "[iOS] 使用目标平台: ${DESTINATION}"

xcodebuild \
  -project "${XCODEPROJ_PATH}" \
  -scheme "${SCHEME_NAME}" \
  -configuration "${BUILD_TYPE}" \
  -sdk "${SDK_NAME}" \
  -destination "${DESTINATION_ARG}" \
  -derivedDataPath "${DERIVED_DATA_DIR}" \
  "${EXTRA_ARGS[@]}" \
  build

APP_PATH="${DERIVED_DATA_DIR}/Build/Products/${PRODUCT_DIR_SUFFIX}/${PROJECT_NAME}.app"
if [[ ! -d "${APP_PATH}" ]]; then
  echo "[iOS] 构建完成，但未找到预期 .app 产物: ${APP_PATH}" >&2
  exit 1
fi

TARGET_DIST_DIR="${DIST_DIR}/${DESTINATION}"
mkdir -p "${TARGET_DIST_DIR}"
rm -rf "${TARGET_DIST_DIR}/${PROJECT_NAME}.app"
cp -R "${APP_PATH}" "${TARGET_DIST_DIR}/${PROJECT_NAME}.app"

echo "[iOS] 构建完成，产物已复制到: ${TARGET_DIST_DIR}/${PROJECT_NAME}.app"
