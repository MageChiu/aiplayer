#include "../../shared/models/translation/translationsettings.h"

#include <QSettings>

TranslationSettings TranslationSettings::load(QSettings &settings) {
    TranslationSettings config;
    config.sourceLanguage = settings.value("source_lang", config.sourceLanguage).toString();
    config.targetLanguage = settings.value("target_lang", config.targetLanguage).toString();
    config.translationEnabled = settings.value("translation_enabled", config.translationEnabled).toBool();
    config.translationMode = settings.value("translation_mode", config.translationMode).toString();
    config.provider = settings.value("translation_provider", config.provider).toString();
    config.baseUrl = settings.value("translation_base_url", config.baseUrl).toString();
    config.endpoint = settings.value("translation_endpoint", config.endpoint).toString();
    config.apiKey = settings.value("translation_api_key", config.apiKey).toString();
    config.localModelIndex = settings.value("translation_local_model_index", config.localModelIndex).toInt();
    config.localModelUrl = settings.value("translation_local_model_url", config.localModelUrl).toString();
    config.localModelPath = settings.value("translation_local_model_path", config.localModelPath).toString();
    return config;
}

void TranslationSettings::save(QSettings &settings) const {
    settings.setValue("source_lang", sourceLanguage);
    settings.setValue("target_lang", targetLanguage);
    settings.setValue("translation_enabled", translationEnabled);
    settings.setValue("translation_mode", translationMode);
    settings.setValue("translation_provider", provider);
    settings.setValue("translation_base_url", baseUrl);
    settings.setValue("translation_endpoint", endpoint);
    settings.setValue("translation_api_key", apiKey);
    settings.setValue("translation_local_model_index", localModelIndex);
    settings.setValue("translation_local_model_url", localModelUrl);
    settings.setValue("translation_local_model_path", localModelPath);
}
