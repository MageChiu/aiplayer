# Mobile Transition Assets

`src/platform/mobile` no longer hosts Android or iOS runtime implementations.

The remaining headers are the only stage-two transition assets still kept in this directory:

- `mobilecontracts.h`: still acts as a shared mobile-only contract container and is retained until its stable subsets are either promoted into `src/shared` or explicitly retired
- `mobilepathprovider.h`: remains as a lightweight fallback adapter used only for syntax and boundary verification, not as the long-term Android/iOS path strategy
- `mobilelocaltranslationbackend.h`: remains as an explicit stub backend so mobile translation wiring can fail with a clear message before a native implementation exists

Exit criteria for this directory:

- move stable cross-platform contracts out of `mobilecontracts.h`
- replace `mobilepathprovider.h` with platform-specific path services or remove the fallback entirely
- implement a real mobile local translation backend or delete the stub path from the translation wiring

New Android and iOS platform code must live in `src/platform/android` and `src/platform/ios`.
