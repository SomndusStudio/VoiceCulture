/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "Utils/SSVoiceLocalizationUtils.h"

#include "JsonObjectConverter.h"
#include "SSVoiceLocalizationEditorSubsystem.h"
#include "SSVoiceLocalizationSettings.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Async/Async.h"
#include "Misc/FileHelper.h"
#include "Misc/ScopedSlowTask.h"
#include "Settings/SSVoiceAutofillStrategy.h"
#include "Settings/SSVoiceLocalizationEditorSettings.h"
#include "Sound/SoundCue.h"
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
	auto* VLEditorSubsystem = GEditor->GetEditorSubsystem<USSVoiceLocalizationEditorSubsystem>();
	USSVoiceAutofillStrategy* Strategy = VLEditorSubsystem->GetActiveStrategy();

	if (!IsValid(Strategy))
	{
		UE_LOG(LogTemp, Warning, TEXT("[SSVoice] Profile has no valid strategy instance."));
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
	AsyncTask(ENamedThreads::GameThread, [OnComplete]()
	{
		TMap<FString, int32> TotalAssetsPerCulture;
		TMap<FString, int32> CultureHitCount;

		// Load asset registry
		FAssetRegistryModule& AssetRegistryModule = USSVoiceLocalizationEditorSubsystem::GetAssetRegistryModule();

		FARFilter Filter;
		Filter.ClassPaths.Add(USSLocalizedVoiceSound::StaticClass()->GetClassPathName());
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

			TSet<FString> PresentCultures;

			if (Result.IsSet())
			{
				const FString CultureString = Result.GetValue();

				TArray<FString> Cultures;
				CultureString.ParseIntoArray(Cultures, TEXT(","));

				for (const FString& Culture : Cultures)
				{
					const FString Normalized = Culture.ToLower();
					CultureHitCount.FindOrAdd(Normalized)++;
					PresentCultures.Add(Normalized);
				}
			}
			// 2. Compter comme 1 asset total pour *chaque culture supportée*
			for (const FString& Culture : AllCultures)
			{
				TotalAssetsPerCulture.FindOrAdd(Culture)++;

				// (Optionnel) Log if missing culture
				if (!PresentCultures.Contains(Culture))
				{
					// Missing culture in this asset
					// UE_LOG(LogTemp, Verbose, TEXT("Asset '%s' missing culture: %s"), *AssetData.AssetName.ToString(), *Culture);
				}
			}
		}

		// 3. Generate json report
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

void FSSVoiceLocalizationUtils::AutoFillCultureAsync(
	const FString& TargetCulture,
	bool bOverrideExisting,
	TFunction<void(int32 ModifiedAssetCount)> OnCompleted)
{
	// Display a progress dialog
	TSharedPtr<FScopedSlowTask> SlowTask = MakeShared<FScopedSlowTask>(
		1.f,
		FText::Format(
			NSLOCTEXT("SSVoice", "AutoFillCultureTitle", "Auto-filling voice culture '{0}'..."),
			FText::FromString(TargetCulture.ToUpper()))
	);
	SlowTask->MakeDialog(true);

	// Phase 1 - Background thread: asset scan and filtering using metadata only
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [TargetCulture, bOverrideExisting, SlowTask, OnCompleted]()
	{
		const FString NormalizedCulture = TargetCulture.ToLower();

		// Query the asset registry for all localized voice assets
		FAssetRegistryModule& AssetRegistry = USSVoiceLocalizationEditorSubsystem::GetAssetRegistryModule();

		FARFilter Filter;
		Filter.ClassPaths.Add(USSLocalizedVoiceSound::StaticClass()->GetClassPathName());
		Filter.bRecursivePaths = true;
		Filter.PackagePaths.Add(FName("/Game"));

		TArray<FAssetData> AssetList;
		AssetRegistry.Get().GetAssets(Filter, AssetList);

		// Filter down to only the assets that need autofill
		TArray<FAssetData> AssetsToProcess;
		for (const FAssetData& AssetData : AssetList)
		{
			bool bSkip = false;

			if (!bOverrideExisting)
			{
				const FAssetTagValueRef Tag = AssetData.TagsAndValues.FindTag("VoiceCultures");

				if (Tag.IsSet())
				{
					const FString TagValue = Tag.GetValue(); // e.g. "fr,en"
					TArray<FString> TagCultures;
					TagValue.ParseIntoArray(TagCultures, TEXT(","));

					// Skip if culture already listed and we do not want to override
					if (TagCultures.ContainsByPredicate(
						[&](const FString& Val) { return Val.ToLower() == NormalizedCulture; }))
					{
						bSkip = true;
					}
				}
			}

			if (!bSkip)
			{
				AssetsToProcess.Add(AssetData);
			}
		}

		// Phase 2 - Game thread: load and modify eligible assets
		AsyncTask(ENamedThreads::GameThread,
		          [AssetsToProcess, NormalizedCulture, bOverrideExisting, SlowTask, OnCompleted]()
		          {
			          const auto* EditorSettings = USSVoiceLocalizationEditorSettings::GetSetting();

			          TSet<UPackage*> ModifiedPackages;
			          int32 ModifiedAssets = 0;

			          // Retrieve the active autofill strategy from project settings
			          auto* VLEditorSubsystem = GEditor->GetEditorSubsystem<USSVoiceLocalizationEditorSubsystem>();
			          USSVoiceAutofillStrategy* Strategy = VLEditorSubsystem->GetActiveStrategy();

			          if (!Strategy)
			          {
				          FSSVoiceLocalizationUI::NotifyFailure(
					          NSLOCTEXT("SSVoice", "AutoFill_NoProfile", "No active auto-fill strategy found."));
				          return;
			          }

			          // Iterate through each asset and apply autofill
			          for (const FAssetData& AssetData : AssetsToProcess)
			          {
				          USSLocalizedVoiceSound* Asset = Cast<USSLocalizedVoiceSound>(AssetData.GetAsset());
				          if (!IsValid(Asset))
					          continue;

				          // Check if culture is already set
				          const bool bAlreadyHasCulture = Asset->LocalizedAudioEntries.ContainsByPredicate(
					          [&](const FSSLocalizedAudioEntry& Entry)
					          {
						          return Entry.Culture.ToLower() == NormalizedCulture;
					          });

				          if (bAlreadyHasCulture && !bOverrideExisting)
					          continue;

				          // Execute autofill strategy
				          FSSLocalizedAudioEntry NewEntry;
				          if (Strategy->ExecuteOneCultureAutofillInAsset(
					          Asset, NormalizedCulture, bOverrideExisting, NewEntry))
				          {
					          bool bCultureAlreadyExists = false;

					          // Update existing entry if needed
					          for (FSSLocalizedAudioEntry& Entry : Asset->LocalizedAudioEntries)
					          {
						          if (Entry.Culture.ToLower() == NewEntry.Culture.ToLower())
						          {
							          bCultureAlreadyExists = true;

							          if (bOverrideExisting)
							          {
								          Entry.Sound = NewEntry.Sound;
								          ModifiedAssets++;
								          Asset->MarkPackageDirty();
								          if (EditorSettings->bAutoSaveAfterAutofill) ModifiedPackages.Add(
									          Asset->GetOutermost());
							          }

							          break;
						          }
					          }

					          // Add new entry if it didn't exist
					          if (!bCultureAlreadyExists)
					          {
						          Asset->LocalizedAudioEntries.Add(NewEntry);
						          ModifiedAssets++;
						          Asset->MarkPackageDirty();
						          if (EditorSettings->bAutoSaveAfterAutofill) ModifiedPackages.Add(
							          Asset->GetOutermost());
					          }
				          }
			          }

			          // if should auto save
			          if (EditorSettings->bAutoSaveAfterAutofill)
			          {
				          // Save modified packages
				          for (UPackage* Package : ModifiedPackages)
				          {
					          FString PackageFilename;
					          if (FPackageName::TryConvertLongPackageNameToFilename(
						          Package->GetName(), PackageFilename, FPackageName::GetAssetPackageExtension()))
					          {
						          UPackage::SavePackage(
							          Package,
							          nullptr,
							          EObjectFlags::RF_Standalone,
							          *PackageFilename,
							          GError,
							          nullptr,
							          true, // bSaveToDisk
							          true, // bForceByteSwapping
							          SAVE_NoError
						          );
					          }
				          }
			          }

			          // Finalize UI
			          if (SlowTask.IsValid())
			          {
				          SlowTask->EnterProgressFrame(1.f);
			          }

			          // Notify result
			          if (ModifiedAssets > 0)
			          {
				          FSSVoiceLocalizationUI::NotifySuccess(FText::Format(
					          NSLOCTEXT("SSVoice", "AutoFillDone", "Auto-fill completed: {0} assets updated."),
					          FText::AsNumber(ModifiedAssets)));
			          }
			          else
			          {
				          FSSVoiceLocalizationUI::NotifyFailure(
					          NSLOCTEXT("SSVoice", "AutoFillNoChange", "No assets required auto-filling."));
			          }

			          if (OnCompleted)
			          {
				          OnCompleted(ModifiedAssets);
			          }
		          });
	});
}
