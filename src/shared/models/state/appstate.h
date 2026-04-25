#pragma once

#include "statevocabulary.h"

#include <QString>

struct PlaybackState {
    AsyncLoadState loadState = AsyncLoadState::Idle;
    bool paused = true;
    double positionSeconds = 0.0;
    double durationSeconds = 0.0;
    int volume = 100;
    bool muted = false;
    QString loadedSource;
};

struct SubtitleState {
    SubtitleDisplayMode displayMode = SubtitleDisplayMode::Bilingual;
    QString currentText;
    QString currentTranslatedText;
    QString statusMessage;
};

enum class TranslationStatus {
    Disabled,
    Idle,
    Requesting,
    Success,
    Failed,
    Offline,
};

struct TranslationState {
    TranslationStatus status = TranslationStatus::Disabled;
    bool enabled = false;
    QString mode;
    QString provider;
    QString targetLanguage;
    QString lastError;
};
