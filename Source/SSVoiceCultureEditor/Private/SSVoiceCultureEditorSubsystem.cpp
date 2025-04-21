/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "SSVoiceCultureEditorSubsystem.h"

#include "SSVoiceCultureEditorLog.h"
#include "SSVoiceCultureSettings.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/ScopedSlowTask.h"
#include "Settings/SSVoiceCultureEditorSettings.h"

#define LOCTEXT_NAMESPACE "SSVoiceCultureEditor"

void USSVoiceCultureEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Try to load the active strategy on subsystem startup
	RefreshStrategy();
}

void USSVoiceCultureEditorSubsystem::Deinitialize()
{
	CachedStrategy = nullptr;

	Super::Deinitialize();
}

FAssetRegistryModule& USSVoiceCultureEditorSubsystem::GetAssetRegistryModule()
{
	return FModuleManager::LoadModuleChecked<FAssetRegistryModule>(
		"AssetRegistry");
}

void USSVoiceCultureEditorSubsystem::ChangeActiveProfileFromName(FString ProfileName)
{
	auto* Profile = GetProfileFromName(ProfileName);

	auto* EditorSettings = USSVoiceCultureEditorSettings::GetMutableSetting();
	EditorSettings->ActiveVoiceProfileName = Profile->ProfileName;
	EditorSettings->SaveConfig();
	
	OnVoiceProfileNameChange();
}

void USSVoiceCultureEditorSubsystem::OnVoiceProfileNameChange()
{
	RefreshStrategy();
}

USSVoiceCultureStrategy* USSVoiceCultureEditorSubsystem::GetActiveStrategy()
{
	// Already cached and valid
	if (IsValid(CachedStrategy))
	{
		return CachedStrategy;
	}

	return RefreshStrategy();
}

const FSSVoiceStrategyProfile* USSVoiceCultureEditorSubsystem::GetActiveProfile() const
{
	// Retrieve the editor settings singleton
	const auto* EditorSettings = USSVoiceCultureEditorSettings::GetSetting();

	// If settings are null or no profiles are defined, fallback to the default profile
	if (!EditorSettings || EditorSettings->StrategyProfiles.Num() == 0)
	{
		return &EditorSettings->FallbackProfile;
	}

	// Search for a profile that matches the active name defined in settings
	for (const auto& Profile : EditorSettings->StrategyProfiles)
	{
		if (Profile.ProfileName == EditorSettings->ActiveVoiceProfileName)
		{
			return &Profile;
		}
	}

	// If no matching profile is found, fallback to the default
	return &EditorSettings->FallbackProfile;
}

const FSSVoiceStrategyProfile* USSVoiceCultureEditorSubsystem::GetProfileFromName(FString ProfileName) const
{
	// Get editor settings singleton
	const auto* EditorSettings = USSVoiceCultureEditorSettings::GetSetting();

	// Must be not null
	check(EditorSettings);
	
	// Return fallback if no profiles are defined
	if (EditorSettings->StrategyProfiles.Num() == 0)
	{
		return &EditorSettings->FallbackProfile;
	}

	// Iterate through all defined profiles to find a name match
	for (const auto& Profile : EditorSettings->StrategyProfiles)
	{
		if (Profile.ProfileName == ProfileName)
		{
			return &Profile;
		}
	}

	// Return fallback profile if no matching profile is found
	return &EditorSettings->FallbackProfile;
}

bool USSVoiceCultureEditorSubsystem::IsReady() const
{
	return CachedStrategy != nullptr;
}

TArray<FAssetData> USSVoiceCultureEditorSubsystem::GetAllSoundBaseAssets(bool bRecursivePaths)
{
	FAssetRegistryModule& AssetRegistry = GetAssetRegistryModule();
	AssetRegistry.Get().SearchAllAssets(true);

	FARFilter Filter;
	Filter.ClassPaths.Add(USoundBase::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true;
	Filter.bRecursivePaths = bRecursivePaths;
	Filter.PackagePaths.Add(FName("/Game"));

	TArray<FAssetData> FoundAssets;
	AssetRegistry.Get().GetAssets(Filter, FoundAssets);

	return FoundAssets;
}

TArray<FAssetData> USSVoiceCultureEditorSubsystem::GetAllLocalizeVoiceSoundAssets()
{
	// 1. Prepare registry
	FAssetRegistryModule& AssetRegistry = GetAssetRegistryModule();

	FARFilter Filter;
	Filter.ClassPaths.Add(USSVoiceCultureSound::StaticClass()->GetClassPathName());
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Add(FName("/Game"));

	TArray<FAssetData> FoundAssets;
	AssetRegistry.Get().GetAssets(Filter, FoundAssets);

	return FoundAssets;
}

void USSVoiceCultureEditorSubsystem::GetAssetsFromVoiceActor(TArray<FAssetData>& Assets, FString VoiceActorName)
{
	// Access the asset registry module
	FAssetRegistryModule& AssetRegistry = GetAssetRegistryModule();

	// Early exit if assets are still being loaded (unsafe to query)
	if (AssetRegistry.Get().IsLoadingAssets()) return;

	TArray<FAssetData> AssetsAll;
	TSet<FAssetData> VoiceActorAssets;

	// If no actor name provided, exit
	if (VoiceActorName.IsEmpty()) return;
	
	// 1. Retrieve all assets of type USSVoiceCultureSound
	Assets = GetAllLocalizeVoiceSoundAssets();

	// 2. Filter those that contain the actor name in their asset name
	for (const auto& AssetData : Assets)
	{
		const FString Name = AssetData.AssetName.ToString();

		if (!Name.Contains(VoiceActorName))
			continue;
		
		VoiceActorAssets.Add(AssetData);
	}

	// 3. Return filtered list
	Assets = VoiceActorAssets.Array();
}

void USSVoiceCultureEditorSubsystem::GetAssetsWithCulture(TArray<FAssetData>& Assets, const bool bCompleteCulture)
{
	FAssetRegistryModule& AssetRegistry = GetAssetRegistryModule();

	// Skip if asset registry is still scanning
	if (AssetRegistry.Get().IsLoadingAssets()) return;

	// Retrieve supported cultures from settings
	const auto* Settings = USSVoiceCultureSettings::GetSetting();
	const TSet<FString> AllCultures = Settings ? Settings->SupportedVoiceCultures : TSet<FString>();
	int32 AllCultureCount = AllCultures.Num();

	// Work on a copy to avoid modifying the array during iteration
	TArray<FAssetData> AllAssets = Assets;

	for (const auto& AssetData : AllAssets)
	{
		// Try reading the "VoiceCultures" tag (comma-separated string like "en,fr,jp")
		const FAssetTagValueRef CultureTag = AssetData.TagsAndValues.FindTag("VoiceCultures");
		TSet<FString> PresentCultures;

		if (CultureTag.IsSet())
		{
			const FString CultureString = CultureTag.GetValue();

			TArray<FString> Cultures;
			CultureString.ParseIntoArray(Cultures, TEXT(","));
			
			// Normalize cultures (lowercase)
			for (const FString& Culture : Cultures)
			{
				const FString Normalized = Culture.ToLower();
				PresentCultures.Add(Normalized);
			}
		}

		// Filter logic based on completeness
		if (bCompleteCulture)
		{
			// If not complete, remove the asset from the list
			if (PresentCultures.Num() != AllCultureCount)
			{
				Assets.Remove(AssetData);
			}
		}
		else
		{
			// If already complete, remove it (we want incomplete only)
			if (PresentCultures.Num() == AllCultureCount)
			{
				Assets.Remove(AssetData);
			}
		}
	}
}

void USSVoiceCultureEditorSubsystem::GetAssetsWithMissingCulture(TArray<FAssetData>& Assets)
{
	GetAssetsWithCulture(Assets, false);
}

void USSVoiceCultureEditorSubsystem::GetAssetsWithCompleteCulture(TArray<FAssetData>& Assets)
{
	GetAssetsWithCulture(Assets, true);
}

FContentBrowserModule& USSVoiceCultureEditorSubsystem::GetVoiceContentBrowser()
{
	return FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
}

USSVoiceCultureStrategy* USSVoiceCultureEditorSubsystem::RefreshStrategy()
{
	const FSSVoiceStrategyProfile* ActiveProfile = GetActiveProfile();
	if (!ActiveProfile || !ActiveProfile->StrategyClass.IsValid())
	{
		return nullptr;
	}

	// Instantiate the strategy
	UClass* StrategyClass = ActiveProfile->StrategyClass.LoadSynchronous();
	if (!StrategyClass)
	{
		UE_LOG(LogVoiceCultureEditor, Warning, TEXT("[SSVoiceCulture] Failed to load AutoPopulate strategy class"));
		return nullptr;
	}

	CachedStrategy = NewObject<USSVoiceCultureStrategy>(GetTransientPackage(), StrategyClass);
	return CachedStrategy;
}

#undef LOCTEXT_NAMESPACE
