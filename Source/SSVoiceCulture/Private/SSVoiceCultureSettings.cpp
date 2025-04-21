/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "SSVoiceCultureSettings.h"

#define LOCTEXT_NAMESPACE "SSVoiceCultureEditor"

FOnPreviewLanguageChanged USSVoiceCultureSettings::OnPreviewLanguageChanged;

USSVoiceCultureSettings::USSVoiceCultureSettings(const FObjectInitializer& Initializer) {
	SectionName = TEXT("Voice Culture");

	// Default supported voice culture
	if (SupportedVoiceCultures.Num() == 0)
	{
		SupportedVoiceCultures.Add("en");
	}
}


#if WITH_EDITOR

FText USSVoiceCultureSettings::GetSectionText() const
{
	return LOCTEXT("SettingsDisplayName", "Voice Culture");
}


#endif

USSVoiceCultureSettings* USSVoiceCultureSettings::GetMutableSetting()
{
	return GetMutableDefault<USSVoiceCultureSettings>();
}

const USSVoiceCultureSettings* USSVoiceCultureSettings::GetSetting()
{
	return GetDefault<USSVoiceCultureSettings>();
}

FString USSVoiceCultureSettings::GetCurrentLanguage() const
{
	// if current language is empty -> default fallback
	if (CurrentLanguage.IsEmpty())
	{
		return DefaultLanguageFallback;
	}

	// if current language supported
	if (SupportedVoiceCultures.Contains(CurrentLanguage))
	{
		return CurrentLanguage;
	}

	return DefaultLanguageFallback;
}

void USSVoiceCultureSettings::SetPreviewLanguage(const FString& NewLanguage)
{
	auto* Settings = GetMutableSetting();
	Settings->PreviewLanguage = NewLanguage;
	Settings->SaveConfig();

	OnPreviewLanguageChanged.Broadcast(NewLanguage);
}

#if WITH_EDITOR
void USSVoiceCultureSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.GetPropertyName();

	if (PropertyName == GET_MEMBER_NAME_CHECKED(USSVoiceCultureSettings, PreviewLanguage))
	{
		OnPreviewLanguageChanged.Broadcast(PreviewLanguage);
	}
}
#endif