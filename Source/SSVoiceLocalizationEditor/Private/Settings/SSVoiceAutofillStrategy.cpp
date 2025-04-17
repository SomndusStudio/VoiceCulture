// Fill out your copyright notice in the Description page of Project Settings.


#include "Settings/SSVoiceAutofillStrategy.h"

bool USSVoiceAutofillStrategy::ExecuteAutofill_Implementation(const FString& InBaseName,
	TMap<FString, USoundBase*>& OutCultureToSound) const
{
	return false;
}

bool USSVoiceAutofillStrategy::ExecuteOneCultureAutofillInAsset_Implementation(USSLocalizedVoiceSound* TargetAsset,
	const FString& CultureCode, bool bOverrideExisting, FSSLocalizedAudioEntry& OutNewEntry)
{
	return false;
}
