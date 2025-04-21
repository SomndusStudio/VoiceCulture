/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "Utils/SSVoiceCultureUtils.h"

#include "JsonObjectConverter.h"
#include "SSVoiceCultureEditorLog.h"
#include "SSVoiceCultureEditorSubsystem.h"
#include "SSVoiceCultureSettings.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Async/Async.h"
#include "Misc/FileHelper.h"
#include "Misc/ScopedSlowTask.h"
#include "Settings/SSVoiceCultureStrategy.h"
#include "Settings/SSVoiceCultureEditorSettings.h"
#include "UObject/SavePackage.h"
#include "Utils/SSVoiceCultureUI.h"

#define LOCTEXT_NAMESPACE "SSVoiceCultureEditor"

bool FSSVoiceCultureUtils::SaveAsset(UPackage* Package, const FString& PackageFilename)
{
	if (!Package)
	{
		UE_LOG(LogVoiceCultureEditor, Warning, TEXT("[SSVoiceCulture] SaveAsset: Invalid package"));
		return false;
	}

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3
	// UE 5.3+ version
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = EObjectFlags::RF_Standalone;
	SaveArgs.Error = GError;
	SaveArgs.SaveFlags = SAVE_NoError;

	return UPackage::SavePackage(Package, nullptr, *PackageFilename, SaveArgs);

#else
	// UE4-compatible version
	return UPackage::SavePackage(
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
#endif
}

bool FSSVoiceCultureUtils::AutoPopulateFromNaming(USSVoiceCultureSound* TargetAsset, const bool bShowSlowTask,
                                                  const bool bShowNotify)
{
	// Ensure the target asset is valid
	if (!TargetAsset)
	{
		UE_LOG(LogVoiceCultureEditor, Warning, TEXT("AutoPopulateFromNaming: Invalid asset"));
		return false;
	}

	const FString AssetName = TargetAsset->GetName(); // e.g. "LVA_NPC01_Hello"

	// Retrieve the editor subsystem and current localization strategy
	auto* VLEditorSubsystem = GEditor->GetEditorSubsystem<USSVoiceCultureEditorSubsystem>();
	auto* Strategy = VLEditorSubsystem->GetActiveStrategy();

	if (!IsValid(Strategy))
	{
		UE_LOG(LogVoiceCultureEditor, Warning, TEXT("[SSVoiceCulture] Profile has no valid strategy instance."));
		return false;
	}

	// Optional: Setup progress dialog
	TUniquePtr<FScopedSlowTask> SlowTask;
	if (bShowSlowTask)
	{
		SlowTask = MakeUnique<FScopedSlowTask>(
			1.f,
			NSLOCTEXT("SSVoiceCultureEditor", "AutoPopulateRunning", "Auto-populate voice culture entries...")
		);
		SlowTask->MakeDialog(true);
		SlowTask->EnterProgressFrame(0.5f, NSLOCTEXT("SSVoiceCultureEditor", "AutoPopulateScanning", "Scanning assets..."));
	}

	// Try to auto-detect localized sound assets using the selected strategy
	TMap<FString, USoundBase*> LocalizedSounds;
	if (!Strategy->ExecuteAutoPopulate(AssetName, LocalizedSounds))
	{
		if (bShowNotify)
		{
			FSSVoiceCultureUI::NotifyFailure(NSLOCTEXT("SSVoiceCultureEditor", "AutoPopulateFailure", "No matching assets found."));
		}
		return false;
	}

	if (SlowTask)
	{
		SlowTask->EnterProgressFrame(0.5f, NSLOCTEXT("SSVoiceCultureEditor", "AutoPopulateApplying", "Updating entries..."));
	}

	// Keep track of already existing cultures to avoid duplicates
	TSet<FString> ExistingCultures;
	for (const FSSCultureAudioEntry& Entry : TargetAsset->VoiceCultures)
	{
		ExistingCultures.Add(Entry.Culture.ToLower());
	}
	
	const auto* EditorSettings = USSVoiceCultureEditorSettings::GetSetting();
	
	// Append only new localized entries that do not already exist
	for (const auto& Pair : LocalizedSounds)
	{
		const FString NormalizedCulture = Pair.Key.ToLower();
		
		if (ExistingCultures.Contains(NormalizedCulture))
		{
			// Overwrite
			if (EditorSettings->bAutoPopulateOverwriteExisting)
			{
				for (FSSCultureAudioEntry& Entry : TargetAsset->VoiceCultures)
				{
					if (Entry.Culture.ToLower() == NormalizedCulture)
					{
						Entry.Sound = Pair.Value;

						UE_LOG(LogVoiceCultureEditor, Log, TEXT("[SSVoiceCulture] Overwrote culture '%s' with new sound: %s"),
							   *NormalizedCulture, *Pair.Value->GetName());
						break;
					}
				}
			}
			
			UE_LOG(LogVoiceCultureEditor, Verbose, TEXT("[SSVoiceCulture] Culture '%s' already exists, skipping."),
			       *NormalizedCulture);
			continue;
		}

		FSSCultureAudioEntry Entry;
		Entry.Culture = NormalizedCulture;
		Entry.Sound = Pair.Value;
		TargetAsset->VoiceCultures.Add(Entry);
	}

	// If no new entries were added, notify and return
	if (LocalizedSounds.Num() == 0)
	{
		if (bShowNotify)
		{
			FSSVoiceCultureUI::NotifyFailure(NSLOCTEXT("SSVoiceCultureEditor", "AutoPopulateNoNew",
			                                           "No new culture entries were added."));
		}
		return false;
	}

	// Mark the package as dirty to indicate it needs saving
	TargetAsset->MarkPackageDirty();

	// Optionally auto-save the asset if the setting is enabled
	auto* EditorMutableSettings = USSVoiceCultureEditorSettings::GetMutableSetting();
	if (EditorMutableSettings->bAutoSaveAfterAutoPopulate)
	{
		FString PackageFilename;
		if (FPackageName::TryConvertLongPackageNameToFilename(
			TargetAsset->GetOutermost()->GetName(), PackageFilename, FPackageName::GetAssetPackageExtension()))
		{
			SaveAsset(TargetAsset->GetOutermost(), PackageFilename);
		}
	}

	// Final user feedback and log
	if (bShowNotify)
	{
		FSSVoiceCultureUI::NotifySuccess(NSLOCTEXT("SSVoiceCultureEditor", "AutoPopulateSuccess", "Auto-populate completed."));
	}
	UE_LOG(LogVoiceCultureEditor, Log, TEXT("[SSVoiceCulture] Auto-populate added %d entries into '%s'"), LocalizedSounds.Num(),
	       *AssetName);
	return true;
}

bool FSSVoiceCultureUtils::AutoPopulateFromVoiceActor(const FString& VoiceActorName, bool bOnlyMissingCulture)
{
	// Declare a progress bar for the full process
	const FText TaskTitle = FText::FromString(TEXT("Populating voice culture assets..."));
	FScopedSlowTask SlowTask(4.f, TaskTitle); // We'll add sub-steps as we go
	SlowTask.MakeDialog(true); // Show UI with cancel button

	// Step 1: Retrieve all voice culture sound assets
	SlowTask.EnterProgressFrame(1.f, FText::FromString(TEXT("Gathering all voice culture assets...")));

	TArray<FAssetData> Assets;

	// Check for user cancel
	if (SlowTask.ShouldCancel())
	{
		return false;
	}

	// Step 2: Filter by specific voice actor
	SlowTask.EnterProgressFrame(1.f, FText::Format(
		                            NSLOCTEXT("SSVoiceCultureEditor", "FilterByVoiceActor", "Filtering assets by voice actor: {0}"),
		                            FText::FromString(VoiceActorName)
	                            ));
	USSVoiceCultureEditorSubsystem::GetAssetsFromVoiceActor(Assets, VoiceActorName);

	if (SlowTask.ShouldCancel())
	{
		return false;
	}

	// Step 3: Optionally filter only the ones that are missing a culture (e.g., not yet localized)
	if (bOnlyMissingCulture)
	{
		SlowTask.EnterProgressFrame(1.f, FText::FromString(TEXT("Filtering assets with missing culture...")));
		USSVoiceCultureEditorSubsystem::GetAssetsWithCulture(Assets, false);

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
		USSVoiceCultureSound* VoiceSound = Cast<USSVoiceCultureSound>(Asset.GetAsset());
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
		FSSVoiceCultureUI::NotifySuccess(NSLOCTEXT("SSVoiceCultureEditor", "AutoPopulateSuccess", "Auto-populate completed."));
	}
	else
	{
		FSSVoiceCultureUI::NotifyFailure(NSLOCTEXT("SSVoiceCultureEditor", "AutoPopulateNoMatch_Assets",
		                                           "No asset contains at least one auto populate match."));
	}
	// Return whether at least one asset was successfully populated
	return bOneSuccessAtLeast;
}

void FSSVoiceCultureUtils::GenerateCultureCoverageReport(FSSVoiceCultureReport& OutReport)
{
	// Mapping of total asset count per culture (expected)
	// Used to track how many voice assets should exist for each culture
	TMap<FString, int32> TotalAssetsPerCulture;
	
	// Mapping of actual assets found that contain each culture (observed)
	TMap<FString, int32> CultureHitCount;

	// Load the Asset Registry module to query assets
	FAssetRegistryModule& AssetRegistryModule = USSVoiceCultureEditorSubsystem::GetAssetRegistryModule();

	// Build filter to query all USSVoiceCultureSound assets under /Game/
	FARFilter Filter;
	Filter.ClassPaths.Add(USSVoiceCultureSound::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Add(FName("/Game"));

	TArray<FAssetData> FoundAssets;
	AssetRegistryModule.Get().GetAssets(Filter, FoundAssets);

	// Get the list of all supported voice cultures (configured in settings)
	const USSVoiceCultureSettings* VoiceCultureSettings = USSVoiceCultureSettings::GetSetting();
	TSet<FString> AllCultures = VoiceCultureSettings->SupportedVoiceCultures;

	// Iterate over all found assets and extract culture usage
	for (const FAssetData& AssetData : FoundAssets)
	{
		// Attempt to read the "VoiceCultures" metadata tag (e.g., "en,fr,jp")
		const FAssetTagValueRef Result = AssetData.TagsAndValues.FindTag("VoiceCultures");

		TSet<FString> PresentCultures;

		if (Result.IsSet())
		{
			const FString CultureString = Result.GetValue();

			TArray<FString> Cultures;
			CultureString.ParseIntoArray(Cultures, TEXT(","));
			
			// Register each present culture
			for (const FString& Culture : Cultures)
			{
				const FString Normalized = Culture.ToLower();
				CultureHitCount.FindOrAdd(Normalized)++;
				PresentCultures.Add(Normalized);
			}
		}
		
		// For each supported culture, consider this asset as expected to have a version
		for (const FString& Culture : AllCultures)
		{
			TotalAssetsPerCulture.FindOrAdd(Culture)++;

			// Optional logging: notify if this asset is missing this culture
			if (!PresentCultures.Contains(Culture))
			{
				// Missing culture in this asset
				UE_LOG(LogVoiceCultureEditor, Warning, TEXT("Asset '%s' missing culture: %s"), *AssetData.AssetName.ToString(), *Culture);
			}
		}
	}

	// Create the final report structure
	OutReport.GeneratedAt = FDateTime::UtcNow();

	for (const FString& Culture : AllCultures)
	{
		FSSVoiceCultureReportEntry Entry;
		Entry.Culture = Culture;
		Entry.TotalAssets = TotalAssetsPerCulture.FindRef(Culture);
		Entry.AssetsWithCulture = CultureHitCount.FindRef(Culture);
		OutReport.Entries.Add(Entry);
	}

	// Serialize the report as JSON
	FString Json;
	FJsonObjectConverter::UStructToJsonObjectString(OutReport, Json);

	// Save the report to a file
	FString Path = FPaths::ProjectSavedDir() / TEXT("SSVoiceCulture/VoiceCultureReport.json");
	FFileHelper::SaveStringToFile(Json, *Path);
}

bool FSSVoiceCultureUtils::LoadSavedCultureReport(FSSVoiceCultureReport& OutReport)
{
	const FString Path = FPaths::ProjectSavedDir() / TEXT("SSVoiceCulture/VoiceCultureReport.json");

	FString Json;
	if (!FFileHelper::LoadFileToString(Json, *Path))
		return false;

	return FJsonObjectConverter::JsonObjectStringToUStruct(Json, &OutReport);
}

int32 FSSVoiceCultureUtils::AutoPopulateCulture(
	const FString& TargetCulture,
	bool bOverrideExisting)
{
	// Display a progress dialog
	TSharedPtr<FScopedSlowTask> SlowTask = MakeShared<FScopedSlowTask>(
		1.f,
		FText::Format(
			NSLOCTEXT("SSVoiceCultureEditor", "AutoPopulateCultureTitle", "Auto-populate voice culture '{0}'..."),
			FText::FromString(TargetCulture.ToUpper()))
	);
	SlowTask->MakeDialog(true);

	// Step 0: Preload all SoundBase assets once to avoid multiple scans
	const TArray<FAssetData> AllSoundAssets = USSVoiceCultureEditorSubsystem::GetAllSoundBaseAssets();
	
	// Phase 1 - Asset scan and filtering using metadata only
	const FString NormalizedCulture = TargetCulture.ToLower();
	
	// 1. Retrieve assets
	TArray<FAssetData> AssetList = USSVoiceCultureEditorSubsystem::GetAllLocalizeVoiceSoundAssets();

	// Filter down to only the assets that need AutoPopulate
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

	// Phase 2 - load and modify eligible assets
	const auto* EditorSettings = USSVoiceCultureEditorSettings::GetSetting();

	TSet<UPackage*> ModifiedPackages;
	int32 ModifiedAssets = 0;

	// Retrieve the active strategy from project settings
	auto* VLEditorSubsystem = GEditor->GetEditorSubsystem<USSVoiceCultureEditorSubsystem>();
	USSVoiceCultureStrategy* Strategy = VLEditorSubsystem->GetActiveStrategy();

	if (!Strategy)
	{
		FSSVoiceCultureUI::NotifyFailure(
			NSLOCTEXT("SSVoiceCultureEditor", "AutoPopulate_NoProfile", "No active auto-populate strategy found."));
		return 0;
	}

	// Iterate through each asset and apply AutoPopulate
	for (const FAssetData& AssetData : AssetsToProcess)
	{
		USSVoiceCultureSound* Asset = Cast<USSVoiceCultureSound>(AssetData.GetAsset());
		if (!IsValid(Asset))
			continue;

		// Check if culture is already set
		const bool bAlreadyHasCulture = Asset->VoiceCultures.ContainsByPredicate(
			[&](const FSSCultureAudioEntry& Entry)
			{
				return Entry.Culture.ToLower() == NormalizedCulture;
			});

		if (bAlreadyHasCulture && !bOverrideExisting)
			continue;

		// Execute AutoPopulate strategy
		FSSCultureAudioEntry NewEntry;

		// NOTE: Each call to FindMatchingSoundAsset performs a full asset search.
		// Consider optimizing if matching for many cultures in sequence.
		if (Strategy->ExecuteOptimizedOneCultureAutoPopulateInAsset(
			Asset, NormalizedCulture, bOverrideExisting, NewEntry, AllSoundAssets))
		{
			bool bCultureAlreadyExists = false;

			// Update existing entry if needed
			for (FSSCultureAudioEntry& Entry : Asset->VoiceCultures)
			{
				if (Entry.Culture.ToLower() == NewEntry.Culture.ToLower())
				{
					bCultureAlreadyExists = true;

					if (bOverrideExisting)
					{
						Entry.Sound = NewEntry.Sound;
						ModifiedAssets++;
						Asset->MarkPackageDirty();
						if (EditorSettings->bAutoSaveAfterAutoPopulate)
							ModifiedPackages.Add(
								Asset->GetOutermost());
					}

					break;
				}
			}

			// Add new entry if it didn't exist
			if (!bCultureAlreadyExists)
			{
				Asset->VoiceCultures.Add(NewEntry);
				ModifiedAssets++;
				Asset->MarkPackageDirty();
				if (EditorSettings->bAutoSaveAfterAutoPopulate)
					ModifiedPackages.Add(
						Asset->GetOutermost());
			}
		}
	}

	// if should auto save
	if (EditorSettings->bAutoSaveAfterAutoPopulate)
	{
		// Save modified packages
		for (UPackage* Package : ModifiedPackages)
		{
			FString PackageFilename;
			if (FPackageName::TryConvertLongPackageNameToFilename(
				Package->GetName(), PackageFilename, FPackageName::GetAssetPackageExtension()))
			{
				SaveAsset(Package->GetOutermost(), PackageFilename);
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
		FSSVoiceCultureUI::NotifySuccess(FText::Format(
			NSLOCTEXT("SSVoiceCultureEditor", "AutoPopulateDone", "Auto-populate completed: {0} assets updated."),
			FText::AsNumber(ModifiedAssets)));
	}
	else
	{
		FSSVoiceCultureUI::NotifyFailure(
			NSLOCTEXT("SSVoiceCultureEditor", "AutoPopulateNoChange", "No assets required auto-populate."));
	}

	return ModifiedAssets;
}

#undef LOCTEXT_NAMESPACE
