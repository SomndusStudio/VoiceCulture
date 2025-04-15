/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "SSVoiceLocalizationSettings.h"

#define LOCTEXT_NAMESPACE "AFT"

USSVoiceLocalizationSettings::USSVoiceLocalizationSettings(const FObjectInitializer& Initializer) {
	SectionName = TEXT("Voice Localization");
}


#if WITH_EDITOR

FText USSVoiceLocalizationSettings::GetSectionText() const
{
	return LOCTEXT("SettingsDisplayName", "Voice Localization");
}

#endif

const USSVoiceLocalizationSettings* USSVoiceLocalizationSettings::GetSetting()
{
	return GetDefault<USSVoiceLocalizationSettings>();
}