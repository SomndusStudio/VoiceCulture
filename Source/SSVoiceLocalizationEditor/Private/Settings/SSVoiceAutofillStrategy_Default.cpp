// Fill out your copyright notice in the Description page of Project Settings.


#include "Settings/SSVoiceAutofillStrategy_Default.h"

#include "SSLocalizedVoiceSound.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Sound/SoundBase.h"

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
	FAssetRegistryModule& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistry.Get().SearchAllAssets(true);

	FARFilter Filter;
	Filter.ClassNames.Add(USoundBase::StaticClass()->GetFName());
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

	FAssetRegistryModule& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	FARFilter Filter;
	Filter.ClassNames.Add(USoundBase::StaticClass()->GetFName());
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
