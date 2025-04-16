// Fill out your copyright notice in the Description page of Project Settings.


#include "Settings/SSVoiceAutofillStrategy_Default.h"

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
