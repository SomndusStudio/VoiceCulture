/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "SSLocalizedVoiceSound.h"

#include "SSVoiceLocalizationLog.h"
#include "SSVoiceLocalizationSettings.h"
#include "SSVoiceLocalizationSubsystem.h"
#include "Sound/SoundWave.h"

USSLocalizedVoiceSound::USSLocalizedVoiceSound()
{
}

USoundBase* USSLocalizedVoiceSound::GetLocalizedSound(const FString& Language) const
{
	for (const auto& Entry : LocalizedAudioEntries)
	{
		if (Entry.Culture.Equals(Language, ESearchCase::IgnoreCase))
		{
			return Entry.Sound;
		}
	}
	UE_LOG(LogVoiceLocalization, Error, TEXT("%s : Can't found valid LocalizedSound from given language [%s]"), *GetNameSafe(this), *Language);

	return nullptr;
}

USoundBase* USSLocalizedVoiceSound::GetCurrentLocalizedSound() const
{
	if (auto* Subsystem = GEngine->GetEngineSubsystem<USSVoiceLocalizationSubsystem>())
	{
		return GetLocalizedSound(Subsystem->GetCurrentVoiceCulture());
	}

	UE_LOG(LogVoiceLocalization, Error, TEXT("%s : Can't found valid CurrentLocalizedSound"), *GetNameSafe(this));
	
	return nullptr;
}

#if ENGINE_MAJOR_VERSION >= 5
float USSLocalizedVoiceSound::GetDuration() const
{
	if (USoundBase* Inner = GetEffectiveSound())
	{
		return Inner->GetDuration();
	}
	return 0.0f;
}
#endif

bool USSLocalizedVoiceSound::IsPlayable() const
{
	return GetEffectiveSound() != nullptr;
}

void USSLocalizedVoiceSound::Parse(class FAudioDevice* AudioDevice, const UPTRINT NodeWaveInstanceHash,
	FActiveSound& ActiveSound, const FSoundParseParameters& ParseParams, TArray<FWaveInstance*>& WaveInstances)
{
	if (USoundBase* Inner = GetEffectiveSound())
	{
		Inner->Parse(AudioDevice, NodeWaveInstanceHash, ActiveSound, ParseParams, WaveInstances);
	}
}

#if WITH_EDITOR
USoundBase* USSLocalizedVoiceSound::GetPreviewLocalizedSound() const
{
	if (auto* Subsystem = GEngine->GetEngineSubsystem<USSVoiceLocalizationSubsystem>())
	{
		return GetLocalizedSound(Subsystem->GetEditorPreviewLanguage());
	}
	
	UE_LOG(LogVoiceLocalization, Error, TEXT("%s : Can't found valid PreviewLocalizedSound"), *GetNameSafe(this));

	return nullptr;
}

void USSLocalizedVoiceSound::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	Super::GetAssetRegistryTags(OutTags);
	
	// On génère une string CSV comme "fr,en,ja"
	FString Cultures;
	for (const FSSLocalizedAudioEntry& Entry : LocalizedAudioEntries)
	{
		// ignore if empty sound
		if (!Entry.Sound) continue;
		
		if (!Cultures.IsEmpty())
		{
			Cultures += TEXT(",");
		}
		Cultures += Entry.Culture.ToLower();
	}

	OutTags.Add(FAssetRegistryTag("VoiceCultures", Cultures, FAssetRegistryTag::TT_Hidden));
}
#endif

USoundBase* USSLocalizedVoiceSound::GetEffectiveSound() const
{
#if WITH_EDITOR
	if (GIsEditor)
	{
		auto* VoiceLocalizationSettings = USSVoiceLocalizationSettings::GetSetting();
		
		// If in game (PIE/Simulator/etc) but want to use preview language for testing
		if (GWorld->IsGameWorld() && VoiceLocalizationSettings->bUsePreviewLanguageInGame)
		{
			return GetPreviewLocalizedSound();
		}

		// If not in game
		if (!GWorld->IsGameWorld())
		{
			return GetPreviewLocalizedSound();
		}
	}
#endif
	return GetCurrentLocalizedSound();
}