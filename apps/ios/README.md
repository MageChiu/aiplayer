# iOS Project Entry

`apps/ios/AIPlayerIOS` is the iOS project entry for external validation and XcodeGen generation.

- XcodeGen spec: `apps/ios/AIPlayerIOS/project.yml`
- Checked-in project container: `apps/ios/AIPlayerIOS/AIPlayerIOS.xcodeproj`
- App entry source: `apps/ios/AIPlayerIOS/App`
- Shared SwiftUI source: `src/ui/mobile/ios`
- Platform source: `src/platform/ios`

Build from the repository root with:

```bash
./scripts/build_ios.sh --simulator
```
