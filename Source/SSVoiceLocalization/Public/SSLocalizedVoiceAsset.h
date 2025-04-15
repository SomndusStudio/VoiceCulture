// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SSLocalizedVoiceAsset.generated.h"

USTRUCT(BlueprintType)
struct FSSLocalizedAudioEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString LanguageCode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* Sound;
};

/**
 * 
 */
UCLASS(BlueprintType)
class SSVOICELOCALIZATION_API USSLocalizedVoiceAsset : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Localization")
	TArray<FSSLocalizedAudioEntry> LocalizedAudio;

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

#if WITH_EDITOR
	USoundBase* GetPreviewLocalizedSound() const;
#endif
	
};
