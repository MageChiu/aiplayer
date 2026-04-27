# AIPlayer iOS UI Shell

This directory now contains a near-runnable SwiftUI iOS app skeleton.

Included:
- root app shell and coordinator
- playback, library, settings tabs
- AVPlayer-based player surface
- file importer flow entry
- persisted imported resource repository
- progress tracking and local sandbox copy flow
- xcodegen project spec support from `apps/ios/AIPlayerIOS/project.yml`

Current limitations:
- no verified local Xcode build in this environment
- subtitle parsing is still placeholder driven
- online translation remains mocked in the player UI
- no bridge to the C++ core yet
