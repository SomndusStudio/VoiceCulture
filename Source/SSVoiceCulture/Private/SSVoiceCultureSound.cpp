/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "SSVoiceCultureSound.h"

#include "SSVoiceCultureLog.h"
#include "SSVoiceCultureSettings.h"
#include "SSVoiceCultureSubsystem.h"
#include "Sound/SoundWave.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "UObject/AssetRegistryTagsContext.h"

USSVoiceCultureSound::USSVoiceCultureSound()
{
}

USoundBase* USSVoiceCultureSound::GetSoundForCulture(const FString& Language) const
{
	for (const auto& Entry : VoiceCultures)
	{
		if (Entry.Culture.Equals(Language, ESearchCase::IgnoreCase))
		{
			return Entry.Sound;
		}
	}
	
	UE_LOG(LogVoiceCulture, Error, TEXT("%s : Can't found valid CultureSound from given language [%s]"), *GetNameSafe(this), *Language);

	return nullptr;
}

USoundBase* USSVoiceCultureSound::GetCurrentCultureSound() const
{
	if (auto* Subsystem = GEngine->GetEngineSubsystem<USSVoiceCultureSubsystem>())
	{
		return GetSoundForCulture(Subsystem->GetCurrentVoiceCulture());
	}

	UE_LOG(LogVoiceCulture, Error, TEXT("%s : Can't found valid CurrentCultureSound"), *GetNameSafe(this));
	
	return nullptr;
}

#if ENGINE_MAJOR_VERSION >= 5
float USSVoiceCultureSound::GetDuration() const
{
	if (USoundBase* Inner = ResolveEffectiveSound())
	{
		return Inner->GetDuration();
	}
	return 0.0f;
}
#endif

bool USSVoiceCultureSound::IsPlayable() const
{
	return ResolveEffectiveSound() != nullptr;
}

void USSVoiceCultureSound::Parse(class FAudioDevice* AudioDevice, const UPTRINT NodeWaveInstanceHash,
	FActiveSound& ActiveSound, const FSoundParseParameters& ParseParams, TArray<FWaveInstance*>& WaveInstances)
{
	if (USoundBase* Inner = ResolveEffectiveSound())
	{
		Inner->Parse(AudioDevice, NodeWaveInstanceHash, ActiveSound, ParseParams, WaveInstances);
	}
}

#if WITH_EDITOR
USoundBase* USSVoiceCultureSound::GetEditorPreviewSound() const
{
	if (auto* Subsystem = GEngine->GetEngineSubsystem<USSVoiceCultureSubsystem>())
	{
		return GetSoundForCulture(Subsystem->GetEditorPreviewLanguage());
	}
	
	UE_LOG(LogVoiceCulture, Error, TEXT("%s : Can't found valid PreviewCultureSound"), *GetNameSafe(this));

	return nullptr;
}

FString USSVoiceCultureSound::GetVoiceCultureCSV() const
{
	// Build a comma-separated list of all available cultures from the culture voices
	FString Cultures;
	for (const FSSCultureAudioEntry& Entry : VoiceCultures)
	{
		// Skip entries that do not have a valid sound asset
		if (!Entry.Sound) continue;

		// Add a comma separator if needed
		if (!Cultures.IsEmpty())
		{
			Cultures += TEXT(",");
		}
		// Append the culture code in lowercase (e.g., "en", "fr")
		Cultures += Entry.Culture.ToLower();
	}
	return Cultures;
}

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3
void USSVoiceCultureSound::GetAssetRegistryTags(FAssetRegistryTagsContext Context) const
{
	Super::GetAssetRegistryTags(Context);
	
	Context.AddTag(FAssetRegistryTag("VoiceCultures", GetVoiceCultureCSV(), FAssetRegistryTag::TT_Hidden));
}

#else

void USSVoiceCultureSound::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	Super::GetAssetRegistryTags(OutTags);
	
	OutTags.Add(FAssetRegistryTag("VoiceCultures", GetVoiceCultureCSV(), FAssetRegistryTag::TT_Hidden));
}
#endif

#endif

USoundBase* USSVoiceCultureSound::ResolveEffectiveSound() const
{
#if WITH_EDITOR
	if (GIsEditor)
	{
		auto* VoiceLocalizationSettings = USSVoiceCultureSettings::GetSetting();
		
		// If in game (PIE/Simulator/etc) but want to use preview language for testing
		if (GWorld->IsGameWorld() && VoiceLocalizationSettings->bUsePreviewLanguageInGame)
		{
			return GetEditorPreviewSound();
		}

		// If not in game
		if (!GWorld->IsGameWorld())
		{
			return GetEditorPreviewSound();
		}
	}
#endif
	return GetCurrentCultureSound();
}