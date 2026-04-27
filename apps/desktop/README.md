# Desktop App Entry

`apps/desktop` now hosts the desktop product-facing files that define the minimal app entry boundary:

- `src/dialogs/helpdialog.*`
- `src/widgets/logwindow.*`
- `resources/resources.qrc`
- `packaging/fix_mpv_bundle.sh`
- `CMakeLists.txt`

The root `CMakeLists.txt` includes `apps/desktop/CMakeLists.txt` to pull these files into the desktop target.

Files that still remain under `src/` and their current blockers:

- `src/mainwindow.*`: still wires together desktop player, translation, and model flows in one product shell
- `src/settingsdialog.*`: still directly depends on desktop model coordination and current settings persistence
- `src/mpvwidget.*`: still couples Qt widget lifecycle to the desktop player surface and packaging assumptions
