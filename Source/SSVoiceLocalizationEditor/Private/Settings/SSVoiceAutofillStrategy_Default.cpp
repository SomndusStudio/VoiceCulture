/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "Settings/SSVoiceAutofillStrategy_Default.h"

#include "SSLocalizedVoiceSound.h"
#include "SSVoiceLocalizationEditorSubsystem.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Sound/SoundBase.h"

FText USSVoiceAutofillStrategy_Default::DisplayMatchLocalizedVoicePattern_Implementation() const
{
	return FText::FromString("LVA_{ActorName}_{Suffix}");
}

FText USSVoiceAutofillStrategy_Default::DisplayMatchLocalizedVoicePatternExample_Implementation() const
{
	return FText::FromString("LVA_NPC001_MarketScene01_L01");
}

FText USSVoiceAutofillStrategy_Default::DisplayMatchCultureRulePattern_Implementation() const
{
	return FText::FromString("{AssetType}_{Culture}_{ActorName}_{Suffix}");
}

FText USSVoiceAutofillStrategy_Default::DisplayMatchCultureRulePatternExample_Implementation() const
{
	return FText::FromString("A_EN_NPC001_MarketScene01_L01");
}

bool USSVoiceAutofillStrategy_Default::ExecuteAutofill_Implementation(const FString& InBaseName,
                                                                      TMap<FString, USoundBase*>& OutCultureToSound) const
{
	// Step 1: Extract suffix from InBaseName (e.g. "LVA_NPC01_Hello" -> "NPC01_Hello")
	TArray<FString> Parts;
	InBaseName.ParseIntoArray(Parts, TEXT("_"));

	if (Parts.Num() < 3)
	{
		UE_LOG(LogTemp, Warning, TEXT("AutofillStrategy: InBaseName '%s' doesn't follow expected pattern."), *InBaseName);
		return false;
	}

	FString ExpectedSuffix;
	for (int32 i = 1; i < Parts.Num(); ++i)
	{
		if (!ExpectedSuffix.IsEmpty())
			ExpectedSuffix += TEXT("_");

		ExpectedSuffix += Parts[i];
	}

	// Step 2: Get all SoundBase assets
	FAssetRegistryModule& AssetRegistry = USSVoiceLocalizationEditorSubsystem::GetAssetRegistryModule();
	AssetRegistry.Get().SearchAllAssets(true);

	FARFilter Filter;
	Filter.ClassPaths.Add(USoundBase::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = bRecursivePaths;
	Filter.bRecursivePaths = bRecursivePaths;
	Filter.PackagePaths.Add(FName("/Game"));

	TArray<FAssetData> FoundAssets;
	AssetRegistry.Get().GetAssets(Filter, FoundAssets);

	// Step 3: Match assets
	for (const FAssetData& Asset : FoundAssets)
	{
		const FString Name = Asset.AssetName.ToString();

		if (!Name.EndsWith(ExpectedSuffix))
			continue;

		TArray<FString> NameParts;
		Name.ParseIntoArray(NameParts, TEXT("_"));

		if (NameParts.Num() < 3)
			continue;

		// Extract culture code at CultureIndex (default 1)
		if (!NameParts.IsValidIndex(CultureIndex))
			continue;

		const FString CultureCode = NameParts[CultureIndex].ToLower();

		// Rebuild suffix from parts after culture
		FString SuffixFromAsset;
		for (int32 i = CultureIndex + 1; i < NameParts.Num(); ++i)
		{
			if (!SuffixFromAsset.IsEmpty())
				SuffixFromAsset += TEXT("_");

			SuffixFromAsset += NameParts[i];
		}

		if (SuffixFromAsset != ExpectedSuffix)
			continue;

		// Optional prefix check
		if (AllowedPrefixes.Num() > 0)
		{
			const FString Prefix = NameParts[0];
			bool bMatch = AllowedPrefixes.ContainsByPredicate([&](const FString& Allowed)
			{
				return bCaseSensitivePrefixes ? (Allowed == Prefix) : (Allowed.Equals(Prefix, ESearchCase::IgnoreCase));
			});

			if (!bMatch)
				continue;
		}

		if (USoundBase* Sound = Cast<USoundBase>(Asset.GetAsset()))
		{
			OutCultureToSound.Add(CultureCode, Sound);
		}
	}

	return OutCultureToSound.Num() > 0;
}

bool USSVoiceAutofillStrategy_Default::ExecuteOneCultureAutofillInAsset_Implementation(
	USSLocalizedVoiceSound* TargetAsset, const FString& CultureCode, bool bOverrideExisting, FSSLocalizedAudioEntry& OutNewEntry)
{
	if (!TargetAsset)
		return false;

	const FString AssetName = TargetAsset->GetName(); // ex: LVA_NPC01_Hello

	// Suffix = ce qu’il y a après le premier "_"
	TArray<FString> Parts;
	AssetName.ParseIntoArray(Parts, TEXT("_"));

	if (Parts.Num() < 2)
		return false;

	FString Suffix;
	for (int32 i = 1; i < Parts.Num(); ++i)
	{
		if (i > 1)
			Suffix += TEXT("_");
		Suffix += Parts[i];
	}

	// Ex: culture = fr → on cherche A_fr_NPC01_Hello, KQ_fr_NPC01_Hello, etc.
	const FString TargetSuffix = CultureCode.ToLower() + TEXT("_") + Suffix;

	FAssetRegistryModule& AssetRegistry = USSVoiceLocalizationEditorSubsystem::GetAssetRegistryModule();

	FARFilter Filter;
	Filter.ClassPaths.Add(USoundBase::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Add("/Game");

	TArray<FAssetData> FoundAssets;
	AssetRegistry.Get().GetAssets(Filter, FoundAssets);

	for (const FAssetData& AssetData : FoundAssets)
	{
		const FString CandidateName = AssetData.AssetName.ToString();

		if (!CandidateName.EndsWith(TargetSuffix))
			continue;

		// Found match
		USoundBase* Matched = Cast<USoundBase>(AssetData.GetAsset());
		if (!Matched)
			continue;

		// Check if already present
		for (FSSLocalizedAudioEntry& Entry : TargetAsset->LocalizedAudioEntries)
		{
			if (Entry.Culture.ToLower() == CultureCode.ToLower())
			{
				if (!bOverrideExisting)
					return false;

				OutNewEntry = Entry;
				OutNewEntry.Sound = Matched;
				return true;
			}
		}

		// Add new entry
		OutNewEntry.Culture = CultureCode.ToLower();
		OutNewEntry.Sound = Matched;
		return true;
	}

	return false;
}
