// Fill out your copyright notice in the Description page of Project Settings.


#include "Settings/SSVoiceLocalizationEditorSettings.h"

#include "Settings/SSVoiceAutofillStrategy_Default.h"

USSVoiceLocalizationEditorSettings::USSVoiceLocalizationEditorSettings()
{
	FallbackProfile = FSSVoiceAutofillProfile();
	FallbackProfile.ProfileName = "Fallback_Default";
	FallbackProfile.StrategyClass = USSVoiceAutofillStrategy_Default::StaticClass();
}

const USSVoiceLocalizationEditorSettings* USSVoiceLocalizationEditorSettings::GetSetting()
{
	return GetDefault<USSVoiceLocalizationEditorSettings>();
}
