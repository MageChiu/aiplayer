# Build Matrix

This file provides the minimal external validation matrix for the three platform entry points.

## Desktop

- Entry: `apps/desktop`
- Command: `./scripts/build_macos.sh --release`
- Key requirements: `cmake`, `qt@6`, `pkg-config`, `nasm`, Homebrew, local `.deps`
- Output: `dist/macos/aiplayer.app`

## Android

- Entry: `apps/android`
- Command: `./scripts/build_android.sh --debug`
- Key requirements: `java`, Android SDK, `ANDROID_SDK_ROOT` or `local.properties`, `gradle` or `gradlew`
- Output: `dist/android/*.apk`

## iOS

- Entry: `apps/ios/AIPlayerIOS`
- Command: `./scripts/build_ios.sh --simulator`
- Key requirements: Xcode, Command Line Tools, `xcodebuild`, `xcodegen`, installed iOS Simulator runtime
- Output: `dist/ios/simulator/AIPlayerIOS.app`
