// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/SSVoiceLocalizationUtils.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/ScopedSlowTask.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Utils/SSVoiceLocalizationUI.h"

bool FSSVoiceLocalizationUtils::AutoPopulateFromNaming(USSLocalizedVoiceSound* TargetAsset)
{
	if (!TargetAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("AutoPopulateFromNaming: Invalid asset"));
		return false;
	}
	
	const FString AssetName = TargetAsset->GetName(); // e.g. "LVA_NPC01_Hello"

	// Extract suffix after the second underscore
	TArray<FString> Parts;
	AssetName.ParseIntoArray(Parts, TEXT("_"));

	if (Parts.Num() < 3)
	{
		UE_LOG(LogTemp, Warning, TEXT("Asset name '%s' doesn't follow LVA_{lang}_{name} pattern"), *AssetName);
		return false;
	}

	// Extract suffix part to match against
	FString Suffix;
	for (int32 i = 1; i < Parts.Num(); ++i) // ← attention : i = 1 car on ignore le préfixe
	{
		Suffix += (i > 1 ? TEXT("_") : TEXT("")) + Parts[i];
	}

	// ScopedSlowTask -> 0 : Start showing progress task
	FScopedSlowTask SlowTask(1.f, FText::FromString(TEXT("Auto-filling localized voice entries...")));
	SlowTask.MakeDialog(true); // true = affiche la barre dans l’UI
	
	// ScopedSlowTask -> 1 : Scan
	SlowTask.EnterProgressFrame(0.2f, NSLOCTEXT("SSVoice", "AutoFillScan", "Scanning available sound assets..."));
	
	// Start scanning for matching SoundBase assets
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	FARFilter Filter;
	Filter.ClassNames.Add(USoundBase::StaticClass()->GetFName());
	// TODO : Make parameter in editor for scanning
	// Filter.ClassNames.Add(USoundWave::StaticClass()->GetFName());
	// Filter.ClassNames.Add(USoundCue::StaticClass()->GetFName());
	Filter.bRecursiveClasses = true; //Include child classes of Sound base
	
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Add(FName("/Game")); // You could expand this later

	TArray<FAssetData> FoundAssets;
	AssetRegistryModule.Get().GetAssets(Filter, FoundAssets);

	// ScopedSlowTask -> 2 : Match new assets
	SlowTask.EnterProgressFrame(0.8f, NSLOCTEXT("SSVoice", "AutoFillInsert", "Populating matching culture entries..."));
	
	TMap<FString, USoundBase*> LocalizedSounds;

	for (const FAssetData& AssetData : FoundAssets)
	{
		const FString Name = AssetData.AssetName.ToString(); // e.g. LVA_fr_NPC01_Hello

		if (!Name.EndsWith(Suffix))
			continue;

		TArray<FString> NameParts;
		Name.ParseIntoArray(NameParts, TEXT("_"));

		if (NameParts.Num() < 3)
			continue;

		// Get 2nd part = Culture
		const FString CultureCode = NameParts[1].ToLower();

		// Rebuild expected name from parts
		FString ReconstructedSuffix;
		for (int32 i = 2; i < NameParts.Num(); ++i)
		{
			ReconstructedSuffix += (i > 2 ? TEXT("_") : TEXT("")) + NameParts[i];
		}

		// Must match the expected suffix exactly (no partial match)
		if (ReconstructedSuffix != Suffix)
			continue;
		
		USoundBase* Sound = Cast<USoundBase>(AssetData.GetAsset());
		if (Sound)
		{
			LocalizedSounds.Add(CultureCode.ToLower(), Sound);
		}
	}

	// Failure : 0 assets found
	if (LocalizedSounds.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SSVoice] No new matching localized sounds found for '%s'"), *AssetName);

		FSSVoiceLocalizationUI::NotifyFailure(NSLOCTEXT("SSVoice", "AutoFillFailure", "No new localized voices found."));
		return false;
	}

	// Got existing entries
	TSet<FString> ExistingCultures;
	for (const FSSLocalizedAudioEntry& Entry : TargetAsset->LocalizedAudioEntries)
	{
		ExistingCultures.Add(Entry.Culture.ToLower());
	}
	
	// Fill the array
	for (const auto& Pair : LocalizedSounds)
	{
		const FString NormalizedCulture = Pair.Key.ToLower();

		if (ExistingCultures.Contains(NormalizedCulture))
		{
			UE_LOG(LogTemp, Verbose, TEXT("[SSVoice] Culture '%s' already exists, skipping."), *NormalizedCulture);
			continue;
		}

		FSSLocalizedAudioEntry Entry;
		Entry.Culture = NormalizedCulture;
		Entry.Sound = Pair.Value;
		TargetAsset->LocalizedAudioEntries.Add(Entry);
	}

	TargetAsset->MarkPackageDirty();

	UE_LOG(LogTemp, Log, TEXT("[SSVoice] Auto-fill added %d entries into '%s'"), LocalizedSounds.Num(), *AssetName);

	FSSVoiceLocalizationUI::NotifySuccess(NSLOCTEXT("SSVoice", "AutoFillSuccess", "Auto-fill completed."));
	return true;
}
