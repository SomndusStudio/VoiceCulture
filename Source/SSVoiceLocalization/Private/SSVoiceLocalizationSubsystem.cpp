// Fill out your copyright notice in the Description page of Project Settings.


#include "SSVoiceLocalizationSubsystem.h"

#include "SSVoiceLocalizationSettings.h"

void USSVoiceLocalizationSubsystem::SetCurrentVoiceCulture(const FString& Language)
{
	CurrentLanguage = Language;
}

FString USSVoiceLocalizationSubsystem::GetCurrentVoiceCulture() const
{
	return CurrentLanguage;
}

#if WITH_EDITOR
FString USSVoiceLocalizationSubsystem::GetEditorPreviewLanguage() const
{
	auto* VoiceLocalizationSettings = USSVoiceLocalizationSettings::GetSetting();
	return VoiceLocalizationSettings->PreviewLanguage;
}
#endif