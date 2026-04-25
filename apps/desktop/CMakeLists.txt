set(AIPLAYER_DESKTOP_APP_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/dialogs/helpdialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/widgets/logwindow.cpp
)

set(AIPLAYER_DESKTOP_APP_RESOURCES
    ${CMAKE_CURRENT_LIST_DIR}/resources/resources.qrc
)

set(AIPLAYER_DESKTOP_APP_INCLUDE_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/src/dialogs
    ${CMAKE_CURRENT_LIST_DIR}/src/widgets
)

set(AIPLAYER_DESKTOP_BUNDLE_SCRIPT
    ${CMAKE_CURRENT_LIST_DIR}/packaging/fix_mpv_bundle.sh
)
