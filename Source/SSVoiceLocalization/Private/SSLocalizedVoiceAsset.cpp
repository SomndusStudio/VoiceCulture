// Fill out your copyright notice in the Description page of Project Settings.


#include "SSLocalizedVoiceAsset.h"

#include "SSVoiceLocalizationSubsystem.h"
#include "Sound/SoundWave.h"

USoundBase* USSLocalizedVoiceAsset::GetLocalizedSound(const FString& Language) const
{
	for (const auto& Entry : LocalizedAudio)
	{
		if (Entry.LanguageCode.Equals(Language, ESearchCase::IgnoreCase))
		{
			return Entry.Sound;
		}
	}
	return nullptr;
}

USoundBase* USSLocalizedVoiceAsset::GetCurrentLocalizedSound() const
{
	if (auto* Subsystem = GEngine->GetEngineSubsystem<USSVoiceLocalizationSubsystem>())
	{
		return GetLocalizedSound(Subsystem->GetCurrentVoiceCulture());
	}
	return nullptr;
}

#if WITH_EDITOR
USoundBase* USSLocalizedVoiceAsset::GetPreviewLocalizedSound() const
{
	if (auto* Subsystem = GEngine->GetEngineSubsystem<USSVoiceLocalizationSubsystem>())
	{
		return GetLocalizedSound(Subsystem->GetEditorPreviewLanguage());
	}
	return nullptr;
}
#endif