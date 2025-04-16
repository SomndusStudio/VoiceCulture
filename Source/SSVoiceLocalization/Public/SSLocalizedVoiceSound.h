/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Sound/SoundBase.h"
#include "SSLocalizedVoiceSound.generated.h"

USTRUCT(BlueprintType)
struct FSSLocalizedAudioEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Culture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* Sound;
};

/**
 * Localized voice audio containing multiple audio versions per language.
 *
 * Inherits from USoundBase to be compatible with all systems expecting generic sound assets,
 * such as Audio Components, Anim Notifies, or Dialogue systems.
 *
 * This asset allows you to define a set of localized voice entries, each tied to a specific culture,
 * and resolves the correct one at runtime depending on the current voice language.
 *
 * The actual Sound asset played is chosen via internal logic or via editor preview.
 */
UCLASS(BlueprintType)
class SSVOICELOCALIZATION_API USSLocalizedVoiceSound : public USoundBase
{
	GENERATED_BODY()

public:

	USSLocalizedVoiceSound();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Localization")
	TArray<FSSLocalizedAudioEntry> LocalizedAudioEntries;

	/**
	 * Select current sound from langage
	 * @param Language 
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, Category = "Voice Localization")
	USoundBase* GetLocalizedSound(const FString& Language) const;

	/**
	 * Shortcut : Select current local voice sound
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, Category = "Voice Localization")
	USoundBase* GetCurrentLocalizedSound() const;

	// USoundBase overrides
#if ENGINE_MAJOR_VERSION >= 5
	virtual float GetDuration() const override;
#endif
	virtual bool IsPlayable() const override;
	virtual void Parse(class FAudioDevice* AudioDevice, const UPTRINT NodeWaveInstanceHash, FActiveSound& ActiveSound, const FSoundParseParameters& ParseParams, TArray<FWaveInstance*>& WaveInstances) override;
	
#if WITH_EDITOR
	USoundBase* GetPreviewLocalizedSound() const;
	virtual void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
#endif
	
protected:
	USoundBase* GetEffectiveSound() const;
	
};
