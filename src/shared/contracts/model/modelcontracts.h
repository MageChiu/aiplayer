#pragma once

#include "../../models/state/statevocabulary.h"

#include <QVector>
#include <QtCore/QString>

struct SharedModelRecord {
    QString identifier;
    QString displayName;
    QString fileName;
    QString sourceUrl;
    bool installed = false;
    bool downloadable = false;
};

struct SharedModelInventorySnapshot {
    AsyncLoadState loadState = AsyncLoadState::Idle;
    QVector<SharedModelRecord> asrModels;
    QVector<SharedModelRecord> translationModels;
    QString lastError;
};

class ISharedModelCatalog {
public:
    virtual ~ISharedModelCatalog() = default;

    virtual SharedModelInventorySnapshot inventory() const = 0;
};
