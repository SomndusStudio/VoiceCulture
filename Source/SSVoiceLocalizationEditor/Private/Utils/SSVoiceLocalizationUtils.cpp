// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/SSVoiceLocalizationUtils.h"

#include "JsonObjectConverter.h"
#include "SSVoiceLocalizationSettings.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Async/Async.h"
#include "Misc/FileHelper.h"
#include "Misc/ScopedSlowTask.h"
#include "Settings/SSVoiceAutofillStrategy.h"
#include "Settings/SSVoiceLocalizationEditorSettings.h"
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

	// Retrieve autofill settings
	const USSVoiceLocalizationEditorSettings* Settings = GetDefault<USSVoiceLocalizationEditorSettings>();
	if (!Settings || Settings->AutofillProfiles.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SSVoice] No autofill profile available."));
		return false;
	}

	const FSSVoiceAutofillProfile& Profile = Settings->AutofillProfiles[0];
	USSVoiceAutofillStrategy* Strategy = NewObject<USSVoiceAutofillStrategy>(
		GetTransientPackage(), Profile.StrategyClass.LoadSynchronous());

	if (!IsValid(Strategy))
	{
		UE_LOG(LogTemp, Warning, TEXT("[SSVoice] Profile '%s' has no valid strategy instance."), *Profile.ProfileName);
		return false;
	}

	// Show UI progress
	FScopedSlowTask SlowTask(1.f, NSLOCTEXT("SSVoice", "AutoFillRunning", "Auto-filling localized voice entries..."));
	SlowTask.MakeDialog(true);
	SlowTask.EnterProgressFrame(0.5f, NSLOCTEXT("SSVoice", "AutoFillScanning", "Scanning assets..."));

	TMap<FString, USoundBase*> LocalizedSounds;
	if (!Strategy->ExecuteAutofill(AssetName, LocalizedSounds))
	{
		FSSVoiceLocalizationUI::NotifyFailure(NSLOCTEXT("SSVoice", "AutoFillFailure", "No matching assets found."));
		return false;
	}

	SlowTask.EnterProgressFrame(0.5f, NSLOCTEXT("SSVoice", "AutoFillApplying", "Updating entries..."));

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

	if (LocalizedSounds.Num() == 0)
	{
		FSSVoiceLocalizationUI::NotifyFailure(NSLOCTEXT("SSVoice", "AutoFillNoNew",
		                                                "No new localized entries were added."));
		return false;
	}

	TargetAsset->MarkPackageDirty();

	FSSVoiceLocalizationUI::NotifySuccess(NSLOCTEXT("SSVoice", "AutoFillSuccess", "Auto-fill completed."));
	UE_LOG(LogTemp, Log, TEXT("[SSVoice] Auto-fill added %d entries into '%s'"), LocalizedSounds.Num(), *AssetName);
	return true;
}

void FSSVoiceLocalizationUtils::GenerateCultureCoverageReportAsync(
	TFunction<void(const FSSVoiceCultureReport&)> OnComplete)
{
	Async(EAsyncExecution::ThreadPool, [OnComplete]()
	{
		TMap<FString, int32> TotalAssetsPerCulture;
		TMap<FString, int32> CultureHitCount;

		// Load asset registry
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(
			"AssetRegistry");

		FARFilter Filter;
		Filter.ClassNames.Add(USSLocalizedVoiceSound::StaticClass()->GetFName());
		Filter.bRecursiveClasses = true;
		Filter.bRecursivePaths = true;
		Filter.PackagePaths.Add(FName("/Game"));

		TArray<FAssetData> FoundAssets;
		AssetRegistryModule.Get().GetAssets(Filter, FoundAssets);

		// Get supported voice cultures
		const USSVoiceLocalizationSettings* VoiceLocalizationSettings = USSVoiceLocalizationSettings::GetSetting();
		TSet<FString> AllCultures = VoiceLocalizationSettings->SupportedVoiceCultures;

		for (const FAssetData& AssetData : FoundAssets)
		{
			// Lire le tag "VoiceCultures" directement
			const FAssetTagValueRef Result = AssetData.TagsAndValues.FindTag("VoiceCultures");
			if (!Result.IsSet())
			{
				continue;
			}

			const FString CultureString = Result.GetValue();

			TArray<FString> Cultures;
			CultureString.ParseIntoArray(Cultures, TEXT(","));

			for (const FString& Culture : Cultures)
			{
				FString Normalized = Culture.ToLower();
				CultureHitCount.FindOrAdd(Normalized)++;
			}

			// On considère que tous les assets sont comptés comme Total
			for (const FString& Culture : AllCultures)
			{
				TotalAssetsPerCulture.FindOrAdd(Culture)++;
			}
		}

		FSSVoiceCultureReport Report;
		Report.GeneratedAt = FDateTime::UtcNow();

		for (const FString& Culture : AllCultures)
		{
			FSSVoiceCultureReportEntry Entry;
			Entry.Culture = Culture;
			Entry.TotalAssets = TotalAssetsPerCulture.FindRef(Culture);
			Entry.AssetsWithCulture = CultureHitCount.FindRef(Culture);
			Report.Entries.Add(Entry);
		}

		// Save report
		FString Json;
		FJsonObjectConverter::UStructToJsonObjectString(Report, Json);

		FString Path = FPaths::ProjectSavedDir() / TEXT("SSVoiceLocalization/VoiceCultureReport.json");
		FFileHelper::SaveStringToFile(Json, *Path);

		// Return to game thread
		AsyncTask(ENamedThreads::GameThread, [Report, OnComplete]()
		{
			OnComplete(Report);
		});
	});
}

bool FSSVoiceLocalizationUtils::LoadSavedCultureReport(FSSVoiceCultureReport& OutReport)
{
	const FString Path = FPaths::ProjectSavedDir() / TEXT("SSVoiceLocalization/VoiceCultureReport.json");

	FString Json;
	if (!FFileHelper::LoadFileToString(Json, *Path))
		return false;

	return FJsonObjectConverter::JsonObjectStringToUStruct(Json, &OutReport, 0, 0);
}
