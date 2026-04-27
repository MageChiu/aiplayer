#pragma once

#include "shared/models/state/statevocabulary.h"

#include <QString>
#include <QStringList>

class IPathProvider;
class ITranslationBackend;

enum class MobileImportResourceKind {
    Video,
    Subtitle,
};

struct MobileImportRequest {
    MobileImportResourceKind resourceKind = MobileImportResourceKind::Video;
    QString title;
    QString successMessage;
    QStringList allowedFileExtensions;
    bool allowsMultiple = false;
};

struct MobileShellState {
    AppLaunchStage launchStage = AppLaunchStage::Idle;
    AppLifecycleState lifecycle = AppLifecycleState::Inactive;
    NavigationStateSnapshot navigation;
    AsyncLoadState importState = AsyncLoadState::Idle;
};

class IMobileNavigator {
public:
    virtual ~IMobileNavigator() = default;

    virtual NavigationStateSnapshot currentState() const = 0;
    virtual void open(AppRouteId route) = 0;
    virtual bool goBack() = 0;
    virtual void resetTo(AppRouteId route) = 0;
};

class IMobileFileImportService {
public:
    virtual ~IMobileFileImportService() = default;

    virtual void beginImport(const MobileImportRequest &request) = 0;
};

struct MobilePlatformServices {
    IMobileNavigator *navigator = nullptr;
    IMobileFileImportService *fileImportService = nullptr;
    IPathProvider *pathProvider = nullptr;
    ITranslationBackend *translationBackend = nullptr;
};
