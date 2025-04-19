/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


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
FText USSVoiceAutofillStrategy::DisplayMatchLocalizedVoicePattern_Implementation() const
{
	return FText::FromString("Not Specified (You should do it)");
}

FText USSVoiceAutofillStrategy::DisplayMatchLocalizedVoicePatternExample_Implementation() const
{
	return FText::FromString("Not Specified (You should do it)");
}

FText USSVoiceAutofillStrategy::DisplayMatchCultureRulePattern_Implementation() const
{
	return FText::FromString("Not Specified (You should do it)");
}

FText USSVoiceAutofillStrategy::DisplayMatchCultureRulePatternExample_Implementation() const
{
	return FText::FromString("Not Specified (You should do it)");
}

