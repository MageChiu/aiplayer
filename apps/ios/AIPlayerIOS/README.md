# AIPlayerIOS Project Entry

This directory is the real iOS project entry under `apps/ios`.

Usage in a full Xcode environment:

```bash
brew install xcodegen
cd apps/ios/AIPlayerIOS
xcodegen generate --spec project.yml
open AIPlayerIOS.xcodeproj
```

Current environment note:
- this repository currently does not have full Xcode configured
- the project entry keeps `App/` inside `apps/ios/AIPlayerIOS`
- shared SwiftUI screens stay in `src/ui/mobile/ios`
- platform services stay in `src/platform/ios`
