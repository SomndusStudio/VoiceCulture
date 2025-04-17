// Fill out your copyright notice in the Description page of Project Settings.


#include "SSVoiceLocalizationEditorSubsystem.h"

#include "Settings/SSVoiceLocalizationEditorSettings.h"


void USSVoiceLocalizationEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Try to load the active strategy on subsystem startup
	RefreshStrategy();
}

void USSVoiceLocalizationEditorSubsystem::Deinitialize()
{
	CachedStrategy = nullptr;
	
	Super::Deinitialize();
}

void USSVoiceLocalizationEditorSubsystem::OnVoiceProfileNameChange()
{
	RefreshStrategy();
}

USSVoiceAutofillStrategy* USSVoiceLocalizationEditorSubsystem::GetActiveStrategy()
{
	// Already cached and valid
	if (IsValid(CachedStrategy))
	{
		return CachedStrategy;
	}

	return RefreshStrategy();
}

const FSSVoiceAutofillProfile* USSVoiceLocalizationEditorSubsystem::GetActiveProfile() const
{
	const auto* EditorSettings = USSVoiceLocalizationEditorSettings::GetSetting();
	if (!EditorSettings || EditorSettings->AutofillProfiles.Num() == 0)
	{
		return nullptr;
	}
	
	for (const auto& Profile : EditorSettings->AutofillProfiles)
	{
		if (Profile.ProfileName == EditorSettings->ActiveVoiceProfileName)
		{
			return &Profile;
		}
	}
	
	// If not found (Default internal profile)
	return &EditorSettings->FallbackProfile;
}

bool USSVoiceLocalizationEditorSubsystem::IsReady() const
{
	return CachedStrategy != nullptr;
}

USSVoiceAutofillStrategy* USSVoiceLocalizationEditorSubsystem::RefreshStrategy()
{
	const FSSVoiceAutofillProfile* ActiveProfile = GetActiveProfile();
	if (!ActiveProfile || !ActiveProfile->StrategyClass.IsValid())
	{
		return nullptr;
	}

	// Instantiate the strategy
	UClass* StrategyClass = ActiveProfile->StrategyClass.LoadSynchronous();
	if (!StrategyClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SSVoice] Failed to load autofill strategy class"));
		return nullptr;
	}

	CachedStrategy = NewObject<USSVoiceAutofillStrategy>(GetTransientPackage(), StrategyClass);
	return CachedStrategy;
}
