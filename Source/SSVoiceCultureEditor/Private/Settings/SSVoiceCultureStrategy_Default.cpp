/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "Settings/SSVoiceCultureStrategy_Default.h"

#include "SSVoiceCultureEditorLog.h"
#include "SSVoiceCultureSound.h"
#include "SSVoiceCultureEditorSubsystem.h"
#include "Sound/SoundBase.h"

FString USSVoiceCultureStrategy_Default::BuildExpectedAssetSuffix(const FString& CultureCode,
	const FString& BaseSuffix) const
{
	return CultureCode.ToLower() + TEXT("_") + BaseSuffix;
}

FString USSVoiceCultureStrategy_Default::ExtractSuffixFromBaseName(const FString& BaseName) const
{
	// Split the name by underscores
	TArray<FString> Parts;
	BaseName.ParseIntoArray(Parts, TEXT("_"));

	if (Parts.Num() < 2)
	{
		// Not enough parts to extract a suffix
		return FString();
	}

	// Reconstruct the suffix by skipping the first part (prefix)
	FString Suffix;
	for (int32 i = 1; i < Parts.Num(); ++i)
	{
		if (i > 1)
			Suffix += TEXT("_");

		Suffix += Parts[i];
	}

	return Suffix;
}

bool USSVoiceCultureStrategy_Default::ParseAssetName(const FString& AssetName, FString& OutPrefix, FString& OutCulture,
                                                     FString& OutSuffix) const
{
	// Split the asset name into parts
	TArray<FString> Parts;
	AssetName.ParseIntoArray(Parts, TEXT("_"));

	// Minimum expected format: Prefix_Culture_Suffix1[_Suffix2...]
	if (Parts.Num() < 3)
		return false;

	OutPrefix = Parts[0];
	OutCulture = Parts[1];

	// Combine all parts after the culture into the suffix
	for (int32 i = 2; i < Parts.Num(); ++i)
	{
		if (!OutSuffix.IsEmpty())
			OutSuffix += TEXT("_");
		OutSuffix += Parts[i];
	}

	return true;
}

FText USSVoiceCultureStrategy_Default::DisplayMatchVoiceCulturePattern_Implementation() const
{
	return FText::FromString("LVA_{ActorName}_{Suffix}");
}

FText USSVoiceCultureStrategy_Default::DisplayMatcVoiceCulturePatternExample_Implementation() const
{
	return FText::FromString("LVA_NPC001_MarketScene01_L01");
}

FText USSVoiceCultureStrategy_Default::DisplayMatchCultureRulePattern_Implementation() const
{
	return FText::FromString("{AssetType}_{Culture}_{ActorName}_{Suffix}");
}

FText USSVoiceCultureStrategy_Default::DisplayMatchCultureRulePatternExample_Implementation() const
{
	return FText::FromString("A_EN_NPC001_MarketScene01_L01");
}

bool USSVoiceCultureStrategy_Default::ExecuteAutoPopulate_Implementation(const FString& InBaseName,
                                                                     TMap<FString, USoundBase*>& OutCultureToSound)
const
{
	// Step 1: Extract the expected suffix from the base asset name.
	// Example: "LVA_NPC01_Hello" → expected suffix = "NPC01_Hello"
	TArray<FString> Parts;
	InBaseName.ParseIntoArray(Parts, TEXT("_"));

	if (Parts.Num() < 3)
	{
		// The base name does not follow the expected pattern (e.g., missing parts).
		UE_LOG(LogVoiceCultureEditor, Warning,
		       TEXT("ProfileStrategy: InBaseName '%s' doesn't follow expected pattern."), *InBaseName);
		return false;
	}
	
	// Use helper to extract the suffix from the base name
	FString ExpectedSuffix = ExtractSuffixFromBaseName(InBaseName);

	// Step 2: Retrieve all available USoundBase assets in the project
	TArray<FAssetData> FoundAssets = USSVoiceCultureEditorSubsystem::GetAllSoundBaseAssets(bRecursivePaths);

	// Step 3: Iterate over each asset and match against the expected pattern
	for (const FAssetData& Asset : FoundAssets)
	{
		const FString Name = Asset.AssetName.ToString();
		
		// Try to parse the asset name into parts: prefix, culture code, and suffix
		FString Prefix, CultureCode, AssetSuffix;
		if (!ParseAssetName(Name, Prefix, CultureCode, AssetSuffix))
			continue;

		// Compare the suffix part of the asset name with the expected suffix
		if (AssetSuffix != ExpectedSuffix)
			continue;
		
		// Optional: If specified, check that the asset name ends with the expected suffix
		if (!Name.EndsWith(ExpectedSuffix))
			continue;

		// Optional: Split again if needed (legacy or extended logic)
		TArray<FString> NameParts;
		Name.ParseIntoArray(NameParts, TEXT("_"));

		if (NameParts.Num() < 3)
			continue;

		// Check if the culture code is at the expected index (typically index 1)
		if (!NameParts.IsValidIndex(CultureIndex))
			continue;

		// Optional prefix check: ensure the prefix matches allowed prefixes (if any)
		if (AllowedPrefixes.Num() > 0)
		{
			const bool bMatch = AllowedPrefixes.ContainsByPredicate([&](const FString& Allowed)
			{
				return bCaseSensitivePrefixes ? (Allowed == Prefix) : Allowed.Equals(Prefix, ESearchCase::IgnoreCase);
			});

			if (!bMatch)
				continue;
		}

		// Asset is considered valid — load the sound and map it to the culture code
		if (USoundBase* Sound = Cast<USoundBase>(Asset.GetAsset()))
		{
			OutCultureToSound.Add(CultureCode.ToLower(), Sound);
		}
	}

	// Return true if at least one culture was successfully matched
	return OutCultureToSound.Num() > 0;
}

bool USSVoiceCultureStrategy_Default::
ExecuteOptimizedOneCultureAutoPopulateInAsset(USSVoiceCultureSound* TargetAsset,
	const FString& CultureCode, bool bOverrideExisting, FSSCultureAudioEntry& OutNewEntry,
	const TArray<FAssetData>& AssetCache) const
{
	// Early exit if the target asset is invalid
	if (!TargetAsset)
		return false;

	// Get the asset name to use as a base for matching (e.g. "LVA_NPC01_Hello")
	const FString AssetName = TargetAsset->GetName();

	// Extract the suffix portion of the asset name (e.g. "NPC01_Hello")
	const FString Suffix = ExtractSuffixFromBaseName(AssetName);

	// If the suffix could not be extracted, the name is likely not valid for this strategy
	if (Suffix.IsEmpty())
		return false;

	// Build the expected suffix to search for, depending on the strategy implementation
	// For example: "FR_NPC01_Hello" or "NPC01_Hello_FR"
	const FString TargetSuffix = BuildExpectedAssetSuffix(CultureCode, Suffix);

	// Try to find a matching USoundBase in the cache
	USoundBase* Matched = nullptr;
	
	for (const FAssetData& AssetData : AssetCache)
	{
		const FString Name = AssetData.AssetName.ToString();

		if (!Name.EndsWith(TargetSuffix))
			continue;

		Matched = Cast<USoundBase>(AssetData.GetAsset());
		if (Matched)
			break;
	}

	if (!Matched)
		return false;
	
	// Check if this culture already exists in the target asset
	for (FSSCultureAudioEntry& Entry : TargetAsset->VoiceCultures)
	{
		if (Entry.Culture.ToLower() == CultureCode.ToLower())
		{
			// Entry already exists for this culture
			if (!bOverrideExisting)
			{
				// Do not override — abort operation
				return false;
			}

			// Override the existing entry
			OutNewEntry = Entry;
			OutNewEntry.Sound = Matched;
			return true;
		}
	}

	// No existing entry — create new one
	OutNewEntry.Culture = CultureCode.ToLower();
	OutNewEntry.Sound = Matched;
	return true;
}

bool USSVoiceCultureStrategy_Default::ExecuteExtractActorNameFromAsset_Implementation(const FAssetData& AssetData,
	FString& OutActorName)
{
	const FString AssetName = AssetData.AssetName.ToString(); // e.g. LVA_NPC01_Hello
	TArray<FString> Parts;
	AssetName.ParseIntoArray(Parts, TEXT("_"));

	if (Parts.Num() >= 3)
	{
		OutActorName = Parts[1];
		return true;
	}
	return false;
}

bool USSVoiceCultureStrategy_Default::ExecuteExtractActorNameFromAssetRegistry_Implementation(
	TSet<FString>& OutUniqueActors)
{
	// 1. Retrieve assets
	TArray<FAssetData> FoundAssets = USSVoiceCultureEditorSubsystem::GetAllLocalizeVoiceSoundAssets();

	// 2. Parse actor names
	for (const FAssetData& AssetData : FoundAssets)
	{
		FString ActorName;
		if (ExecuteExtractActorNameFromAsset(AssetData, ActorName))
		{
			OutUniqueActors.Add(ActorName); // Actor = 2nd part
		}
	}

	return true;
}
