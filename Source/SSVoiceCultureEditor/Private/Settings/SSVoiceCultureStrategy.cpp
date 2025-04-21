/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "Settings/SSVoiceCultureStrategy.h"

#include "SSVoiceCultureEditorSubsystem.h"

bool USSVoiceCultureStrategy::ExecuteAutoPopulate_Implementation(const FString& InBaseName,
                                                                 TMap<FString, USoundBase*>& OutCultureToSound) const
{
	return false;
}

bool USSVoiceCultureStrategy::ExecuteOptimizedOneCultureAutoPopulateInAsset(USSVoiceCultureSound* TargetAsset,
	const FString& CultureCode, bool bOverrideExisting, FSSCultureAudioEntry& OutNewEntry,
	const TArray<FAssetData>& AssetCache) const
{
	return false;
}


bool USSVoiceCultureStrategy::ExecuteOneCultureAutoPopulateInAsset_Implementation(USSVoiceCultureSound* TargetAsset,
	const FString& CultureCode, bool bOverrideExisting, FSSCultureAudioEntry& OutNewEntry)
{
	TArray<FAssetData> AllAssets = USSVoiceCultureEditorSubsystem::GetAllSoundBaseAssets();
	return ExecuteOptimizedOneCultureAutoPopulateInAsset(TargetAsset, CultureCode, bOverrideExisting, OutNewEntry, AllAssets);
}
FText USSVoiceCultureStrategy::DisplayMatchLocalizedVoicePattern_Implementation() const
{
	return FText::FromString("Not Specified (You should do it)");
}

FText USSVoiceCultureStrategy::DisplayMatchLocalizedVoicePatternExample_Implementation() const
{
	return FText::FromString("Not Specified (You should do it)");
}

FText USSVoiceCultureStrategy::DisplayMatchCultureRulePattern_Implementation() const
{
	return FText::FromString("Not Specified (You should do it)");
}

FText USSVoiceCultureStrategy::DisplayMatchCultureRulePatternExample_Implementation() const
{
	return FText::FromString("Not Specified (You should do it)");
}

bool USSVoiceCultureStrategy::ExecuteExtractActorNameFromAsset_Implementation(const FAssetData& AssetData,
	FString& OutActorName)
{
	return false;
}

bool USSVoiceCultureStrategy::ExecuteExtractActorNameFromAssetRegistry_Implementation(TSet<FString>& OutUniqueActors)
{
	return false;
}

FString USSVoiceCultureStrategy::BuildExpectedAssetSuffix(const FString& CultureCode, const FString& BaseSuffix) const
{
	return "";
}

bool USSVoiceCultureStrategy::ParseAssetName(const FString& AssetName, FString& OutPrefix, FString& OutCulture,
                                             FString& OutSuffix) const
{
	return false;
}

USoundBase* USSVoiceCultureStrategy::FindMatchingSoundAsset(const FString& CultureCode, const FString& Suffix) const
{
	// Build the expected asset name suffix using the strategy rule (e.g. "fr_MyLine" or "MyLine_fr")
	const FString TargetSuffix = BuildExpectedAssetSuffix(CultureCode, Suffix);

	// Search through all SoundBase assets
	TArray<FAssetData> FoundAssets = USSVoiceCultureEditorSubsystem::GetAllSoundBaseAssets();

	for (const FAssetData& AssetData : FoundAssets)
	{
		const FString CandidateName = AssetData.AssetName.ToString();

		// Skip if it doesn't match expected suffix
		if (!CandidateName.EndsWith(TargetSuffix))
			continue;

		// Try to load it as a USoundBase
		if (USoundBase* Sound = Cast<USoundBase>(AssetData.GetAsset()))
		{
			return Sound;
		}
	}

	// No match found
	return nullptr;
}