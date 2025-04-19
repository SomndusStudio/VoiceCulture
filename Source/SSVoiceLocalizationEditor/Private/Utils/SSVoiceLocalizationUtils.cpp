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
#include "Utils/SSVoiceLocalizationUI.h"

#define LOCTEXT_NAMESPACE "SSLocalizedVoiceSoundEditor"

bool FSSVoiceLocalizationUtils::AutoPopulateFromNaming(USSLocalizedVoiceSound* TargetAsset, const bool bShowSlowTask, const bool bShowNotify)
{
	// Ensure the target asset is valid
	if (!TargetAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("AutoPopulateFromNaming: Invalid asset"));
		return false;
	}

	const FString AssetName = TargetAsset->GetName(); // e.g. "LVA_NPC01_Hello"

	// Retrieve the editor subsystem and current localization strategy
	auto* VLEditorSubsystem = GEditor->GetEditorSubsystem<USSVoiceLocalizationEditorSubsystem>();
	auto* Strategy = VLEditorSubsystem->GetActiveStrategy();

	if (!IsValid(Strategy))
	{
		UE_LOG(LogTemp, Warning, TEXT("[SSVoice] Profile has no valid strategy instance."));
		return false;
	}

	// Optional: Setup progress dialog
	TUniquePtr<FScopedSlowTask> SlowTask;
	if (bShowSlowTask)
	{
		SlowTask = MakeUnique<FScopedSlowTask>(
			1.f,
			NSLOCTEXT("SSVoice", "AutoFillRunning", "Auto-filling localized voice entries...")
		);
		SlowTask->MakeDialog(true);
		SlowTask->EnterProgressFrame(0.5f, NSLOCTEXT("SSVoice", "AutoFillScanning", "Scanning assets..."));
	}

	// Try to auto-detect localized sound assets using the selected strategy
	TMap<FString, USoundBase*> LocalizedSounds;
	if (!Strategy->ExecuteAutofill(AssetName, LocalizedSounds))
	{
		if (bShowNotify)
		{
			FSSVoiceLocalizationUI::NotifyFailure(NSLOCTEXT("SSVoice", "AutoFillFailure", "No matching assets found."));
		}
		return false;
	}

	if (SlowTask)
	{
		SlowTask->EnterProgressFrame(0.5f, NSLOCTEXT("SSVoice", "AutoFillApplying", "Updating entries..."));
	}

	// Keep track of already existing cultures to avoid duplicates
	TSet<FString> ExistingCultures;
	for (const FSSLocalizedAudioEntry& Entry : TargetAsset->LocalizedAudioEntries)
	{
		ExistingCultures.Add(Entry.Culture.ToLower());
	}

	// Append only new localized entries that do not already exist
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

	// If no new entries were added, notify and return
	if (LocalizedSounds.Num() == 0)
	{
		if (bShowNotify)
		{
			FSSVoiceLocalizationUI::NotifyFailure(NSLOCTEXT("SSVoice", "AutoFillNoNew",
													"No new localized entries were added."));
		}
		return false;
	}

	// Mark the package as dirty to indicate it needs saving
	TargetAsset->MarkPackageDirty();

	// Optionally auto-save the asset if the setting is enabled
	auto* EditorSettings = USSVoiceLocalizationEditorSettings::GetMutableSetting();
	if (EditorSettings->bAutoSaveAfterAutofill)
	{
		FString PackageFilename;
		if (FPackageName::TryConvertLongPackageNameToFilename(
			TargetAsset->GetOutermost()->GetName(), PackageFilename, FPackageName::GetAssetPackageExtension()))
		{
			UPackage::SavePackage(
				TargetAsset->GetOutermost(),
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

	// Final user feedback and log
	if (bShowNotify)
	{
		FSSVoiceLocalizationUI::NotifySuccess(NSLOCTEXT("SSVoice", "AutoFillSuccess", "Auto-fill completed."));
	}
	UE_LOG(LogTemp, Log, TEXT("[SSVoice] Auto-fill added %d entries into '%s'"), LocalizedSounds.Num(), *AssetName);
	return true;
}

bool FSSVoiceLocalizationUtils::AutoPopulateFromVoiceActor(const FString& VoiceActorName, bool bOnlyMissingCulture)
{
	// Declare a progress bar for the full process
	const FText TaskTitle = FText::FromString(TEXT("Populating localized voice assets..."));
	FScopedSlowTask SlowTask(4.f, TaskTitle); // We'll add sub-steps as we go
	SlowTask.MakeDialog(true); // Show UI with cancel button

	// Step 1: Retrieve all localized voice sound assets
	SlowTask.EnterProgressFrame(1.f, FText::FromString(TEXT("Gathering all localized voice assets...")));

	TArray<FAssetData> Assets = USSVoiceLocalizationEditorSubsystem::GetAllLocalizeVoiceSoundAssets();

	// Check for user cancel
	if (SlowTask.ShouldCancel())
	{
		return false;
	}

	// Step 2: Filter by specific voice actor
	SlowTask.EnterProgressFrame(1.f, FText::Format(
		                            LOCTEXT("FilterByVoiceActor", "Filtering assets by voice actor: {0}"),
		                            FText::FromString(VoiceActorName)
	                            ));
	USSVoiceLocalizationEditorSubsystem::GetAssetsFromVoiceActor(Assets, VoiceActorName, false);

	if (SlowTask.ShouldCancel())
	{
		return false;
	}

	// Step 3: Optionally filter only the ones that are missing a culture (e.g., not yet localized)
	if (bOnlyMissingCulture)
	{
		SlowTask.EnterProgressFrame(1.f, FText::FromString(TEXT("Filtering assets with missing culture...")));
		USSVoiceLocalizationEditorSubsystem::GetAssetsWithCulture(Assets, false);

		if (SlowTask.ShouldCancel())
		{
			return false;
		}
	}

	// Still consume the progress frame even if no filtering
	SlowTask.EnterProgressFrame(1.f);

	bool bOneSuccessAtLeast = false;

	// Add sub-steps to track per asset
	SlowTask.TotalAmountOfWork += Assets.Num(); // Add dynamic steps

	for (auto& Asset : Assets)
	{
		// Check for user cancellation
		if (SlowTask.ShouldCancel())
		{
			break;
		}

		// Advance the progress bar
		SlowTask.EnterProgressFrame(1.f, FText::FromName(Asset.AssetName));

		// Load the asset and cast to the expected class
		USSLocalizedVoiceSound* VoiceSound = Cast<USSLocalizedVoiceSound>(Asset.GetAsset());
		if (!VoiceSound)
		{
			continue;
		}
		// Attempt to auto-populate data from the asset's naming convention
		if (AutoPopulateFromNaming(VoiceSound, false, false))
		{
			bOneSuccessAtLeast = true;
		}
	}

	// Notify
	if (bOneSuccessAtLeast)
	{
		FSSVoiceLocalizationUI::NotifySuccess(NSLOCTEXT("SSVoice", "AutoFillSuccess", "Auto-fill completed."));
	}
	else
	{
		FSSVoiceLocalizationUI::NotifyFailure(NSLOCTEXT("SSVoice", "AutoFillNoMatch_Assets", "No asset contains at least one autofill match."));
	}
	// Return whether at least one asset was successfully populated
	return bOneSuccessAtLeast;
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

		// 1. Retrieve assets
		TArray<FAssetData> AssetList = USSVoiceLocalizationEditorSubsystem::GetAllLocalizeVoiceSoundAssets();

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
								          if (EditorSettings->bAutoSaveAfterAutofill)
									          ModifiedPackages.Add(
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
						          if (EditorSettings->bAutoSaveAfterAutofill)
							          ModifiedPackages.Add(
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

#undef LOCTEXT_NAMESPACE
