/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "SSVoiceLocalizationSubsystem.h"

#include "SSVoiceLocalizationLog.h"
#include "SSVoiceLocalizationSettings.h"

void USSVoiceLocalizationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	OnStartGameInstanceHandle = FWorldDelegates::OnStartGameInstance.AddUObject(this, &USSVoiceLocalizationSubsystem::HandleStartGameInstance);
}

void USSVoiceLocalizationSubsystem::Deinitialize()
{
	FWorldDelegates::OnStartGameInstance.Remove(OnStartGameInstanceHandle);
	
	Super::Deinitialize();
}

void USSVoiceLocalizationSubsystem::HandleStartGameInstance(UGameInstance* GameInstance)
{
	auto* VoiceLocalizationSettings = USSVoiceLocalizationSettings::GetSetting();
	CurrentLanguage = VoiceLocalizationSettings->DefaultLanguage;
	if (CurrentLanguage.IsEmpty())
	{
		UE_LOG(LogVoiceLocalization, Error, TEXT("%s : Language is empty !"), *GetNameSafe(this));
	}
}

void USSVoiceLocalizationSubsystem::SetCurrentVoiceCulture(const FString& Language)
{
	CurrentLanguage = Language;

	UE_LOG(LogVoiceLocalization, Log, TEXT("%s : Language switched to [%s]"), *GetNameSafe(this), *CurrentLanguage);
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