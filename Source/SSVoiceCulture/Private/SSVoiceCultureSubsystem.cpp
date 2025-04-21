/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "SSVoiceCultureSubsystem.h"

#include "Engine/World.h"
#include "Engine/Engine.h"
#include "SSVoiceCultureLog.h"
#include "SSVoiceCultureSettings.h"

void USSVoiceCultureSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	OnStartGameInstanceHandle = FWorldDelegates::OnStartGameInstance.AddUObject(this, &USSVoiceCultureSubsystem::HandleStartGameInstance);
}

void USSVoiceCultureSubsystem::Deinitialize()
{
	FWorldDelegates::OnStartGameInstance.Remove(OnStartGameInstanceHandle);
	
	Super::Deinitialize();
}

void USSVoiceCultureSubsystem::HandleStartGameInstance(UGameInstance* GameInstance)
{
	auto* VoiceCultureSettings = USSVoiceCultureSettings::GetSetting();
	CurrentLanguage = VoiceCultureSettings->GetCurrentLanguage();
	if (CurrentLanguage.IsEmpty())
	{
		UE_LOG(LogVoiceCulture, Error, TEXT("%s : Language is empty !"), *GetNameSafe(this));
	}
}

void USSVoiceCultureSubsystem::SetCurrentVoiceCulture(const FString& Language)
{
	// Store the language in memory (applied immediately for runtime lookups)
	CurrentLanguage = Language;

	// Persist the new culture setting into the user config file
	auto* VoiceCultureSettings = USSVoiceCultureSettings::GetMutableSetting();
	VoiceCultureSettings->CurrentLanguage = CurrentLanguage;
	VoiceCultureSettings->SaveConfig();

	// Log the culture switch
	UE_LOG(LogVoiceCulture, Log, TEXT("%s : Language switched to [%s]"), *GetNameSafe(this), *CurrentLanguage);
}

FString USSVoiceCultureSubsystem::GetCurrentVoiceCulture() const
{
	return CurrentLanguage;
}

TArray<FString> USSVoiceCultureSubsystem::GetSupportedVoiceCultures() const
{
	const USSVoiceCultureSettings* Settings = USSVoiceCultureSettings::GetSetting();
	return Settings->SupportedVoiceCultures.Array();
}

#if WITH_EDITOR
FString USSVoiceCultureSubsystem::GetEditorPreviewLanguage() const
{
	auto* VoiceLocalizationSettings = USSVoiceCultureSettings::GetSetting();
	return VoiceLocalizationSettings->PreviewLanguage;
}
#endif