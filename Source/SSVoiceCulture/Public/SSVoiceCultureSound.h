/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Sound/SoundBase.h"
#include "Runtime/Launch/Resources/Version.h"
#include "SSVoiceCultureSound.generated.h"

USTRUCT(BlueprintType)
struct FSSCultureAudioEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Culture")
	FString Culture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Culture")
	USoundBase* Sound;
};

/**
 * Voice Culture Sound asset that manages multiple culture versions of a voice line.
 *
 * Inherits from USoundBase to allow seamless integration with all systems expecting generic sound assets,
 * such as Audio Components, Animation Notifies, or Dialogue systems.
 *
 * Stores a collection of culture entries, each associated with a specific culture code (e.g., "fr", "en").
 * At runtime, the appropriate audio is resolved based on the current voice culture.
 */
UCLASS(BlueprintType)
class SSVOICECULTURE_API USSVoiceCultureSound : public USoundBase
{
	GENERATED_BODY()

public:

	USSVoiceCultureSound();

	/** Voice entries per supported culture. Used to select the appropriate voice asset at runtime. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Culture", meta = (
		DisplayName = "Voice Cultures",
		ToolTip = "A list of voice assets, one per culture. The system selects the appropriate one based on the current culture at runtime."
	))
	TArray<FSSCultureAudioEntry> VoiceCultures;

	/**
	 * Retrieves the culture sound for a specific culture code (e.g., "en", "fr").
	 * @param CultureCode The language or culture code to match.
	 * @return A matching sound asset, or nullptr if not found.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voice Culture")
	USoundBase* GetSoundForCulture(const FString& CultureCode) const;

	/**
	 * Retrieves the culture sound based on the current game culture.
	 * Uses internal culture resolution (from subsystem).
	 * @return The appropriate culture voice asset, or nullptr if not found.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voice Culture")
	USoundBase* GetCurrentCultureSound() const;

	// USoundBase overrides
#if ENGINE_MAJOR_VERSION >= 5
	virtual float GetDuration() const override;
#endif
	virtual bool IsPlayable() const override;
	virtual void Parse(class FAudioDevice* AudioDevice, const UPTRINT NodeWaveInstanceHash, FActiveSound& ActiveSound, const FSoundParseParameters& ParseParams, TArray<FWaveInstance*>& WaveInstances) override;
	
#if WITH_EDITOR
	/** Returns the sound that would currently be used in the editor preview (based on editor-selected culture). */
	USoundBase* GetEditorPreviewSound() const;

	/** Returns a comma-separated string of all valid culture codes in lowercase (e.g., "en,fr,ja") */
	FString GetVoiceCultureCSV() const;
	
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3
	/** Adds custom tags to be displayed in the Content Browser (e.g., list of supported cultures). */
	virtual void GetAssetRegistryTags(FAssetRegistryTagsContext Context) const override;
	
#else
	
	/** Adds custom tags to be displayed in the Content Browser (e.g., list of supported cultures). */
	virtual void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
#endif
	
#endif

protected:
	/** Resolves the sound to be used, either for runtime or preview (based on context). */
	USoundBase* ResolveEffectiveSound() const;
	
};
