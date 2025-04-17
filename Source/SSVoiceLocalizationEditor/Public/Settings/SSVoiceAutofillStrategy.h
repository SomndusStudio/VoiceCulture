// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SSLocalizedVoiceSound.h"
#include "UObject/Object.h"
#include "SSVoiceAutofillStrategy.generated.h"

class USSLocalizedVoiceSound;
/**
 * Abstract base class for voice autofill strategies.
 * Defines how voice assets are located and matched to a given localized voice sound.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class SSVOICELOCALIZATIONEDITOR_API USSVoiceAutofillStrategy : public UObject
{
	GENERATED_BODY()

public:
	/** 
	 * Called to search and match localized assets for a given base name.
	 * @param InBaseName The name of the LocalizedVoiceSound asset.
	 * @param OutCultureToSound Map of matched culture codes and their associated sound.
	 * @return Whether any result was found
	 */
	UFUNCTION(BlueprintNativeEvent)
	bool ExecuteAutofill(const FString& InBaseName, TMap<FString, USoundBase*>& OutCultureToSound) const;

	/**
	 * Try to autofill a specific culture in a specific asset (optimized version).
	 * @param TargetAsset The LocalizedVoiceSound to process
	 * @param CultureCode The culture to match (e.g. "fr")
	 * @param bOverrideExisting Whether to override if the culture is already set
	 * @return true if something was added or updated
	 */
	UFUNCTION(BlueprintNativeEvent)
	bool ExecuteOneCultureAutofillInAsset(
		USSLocalizedVoiceSound* TargetAsset,
		const FString& CultureCode,
		bool bOverrideExisting, FSSLocalizedAudioEntry& OutNewEntry
	);
};
