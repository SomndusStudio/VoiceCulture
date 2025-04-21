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
	const auto* EditorSettings = USSVoiceCultureEditorSettings::GetSetting();
	if (!EditorSettings || EditorSettings->StrategyProfiles.Num() == 0)
	{
		return &EditorSettings->FallbackProfile;
	}

	for (const auto& Profile : EditorSettings->StrategyProfiles)
	{
		if (Profile.ProfileName == EditorSettings->ActiveVoiceProfileName)
		{
			return &Profile;
		}
	}

	// If not found (Default internal profile)
	return &EditorSettings->FallbackProfile;
}

const FSSVoiceStrategyProfile* USSVoiceCultureEditorSubsystem::GetProfileFromName(FString ProfileName) const
{
	const auto* EditorSettings = USSVoiceCultureEditorSettings::GetSetting();
	if (!EditorSettings || EditorSettings->StrategyProfiles.Num() == 0)
	{
		return &EditorSettings->FallbackProfile;
	}

	for (const auto& Profile : EditorSettings->StrategyProfiles)
	{
		if (Profile.ProfileName == ProfileName)
		{
			return &Profile;
		}
	}

	// If not found (Default internal profile)
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
	FAssetRegistryModule& AssetRegistry = GetAssetRegistryModule();

	if (AssetRegistry.Get().IsLoadingAssets()) return;

	TArray<FAssetData> AssetsAll;
	TSet<FAssetData> VoiceActorAssets;

	// If empty do nothing
	if (VoiceActorName.IsEmpty()) return;
	
	// 1. Retrieve assets
	Assets = GetAllLocalizeVoiceSoundAssets();

	for (const auto& AssetData : Assets)
	{
		const FString Name = AssetData.AssetName.ToString();

		if (!Name.Contains(VoiceActorName))
			continue;
		
		VoiceActorAssets.Add(AssetData);
	}
	
	Assets = VoiceActorAssets.Array();
}

void USSVoiceCultureEditorSubsystem::GetAssetsWithCulture(TArray<FAssetData>& Assets, const bool bCompleteCulture)
{
	FAssetRegistryModule& AssetRegistry = GetAssetRegistryModule();

	if (AssetRegistry.Get().IsLoadingAssets()) return;

	const USSVoiceCultureSettings* Settings = USSVoiceCultureSettings::GetSetting();
	const TSet<FString> AllCultures = Settings ? Settings->SupportedVoiceCultures : TSet<FString>();
	int32 AllCultureCount = AllCultures.Num();

	// Avoid array edition inf iteration
	TArray<FAssetData> AllAssets = Assets;

	for (const auto& AssetData : AllAssets)
	{
		const FAssetTagValueRef CultureTag = AssetData.TagsAndValues.FindTag("VoiceCultures");
		TSet<FString> PresentCultures;

		if (CultureTag.IsSet())
		{
			const FString CultureString = CultureTag.GetValue();

			TArray<FString> Cultures;
			CultureString.ParseIntoArray(Cultures, TEXT(","));

			for (const FString& Culture : Cultures)
			{
				const FString Normalized = Culture.ToLower();
				PresentCultures.Add(Normalized);
			}
		}

		// bCompleteCulture => Include else Exclude
		if (bCompleteCulture)
		{
			if (PresentCultures.Num() != AllCultureCount)
			{
				Assets.Remove(AssetData);
			}
		}
		else
		{
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
		UE_LOG(LogVoiceCultureEditor, Warning, TEXT("[SSVoice] Failed to load AutoPopulate strategy class"));
		return nullptr;
	}

	CachedStrategy = NewObject<USSVoiceCultureStrategy>(GetTransientPackage(), StrategyClass);
	return CachedStrategy;
}

#undef LOCTEXT_NAMESPACE
