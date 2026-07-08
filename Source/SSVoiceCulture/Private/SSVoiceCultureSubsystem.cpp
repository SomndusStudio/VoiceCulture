/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/


#include "SSVoiceCultureSubsystem.h"

#include "SSVoiceCultureCatalog.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "SSVoiceCultureLog.h"
#include "SSVoiceCultureSettings.h"
#include "Engine/AssetManager.h"

void USSVoiceCultureSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	OnStartGameInstanceHandle = FWorldDelegates::OnStartGameInstance.AddUObject(this, &USSVoiceCultureSubsystem::HandleStartGameInstance);
}

void USSVoiceCultureSubsystem::Deinitialize()
{
	FWorldDelegates::OnStartGameInstance.Remove(OnStartGameInstanceHandle);

	Super::Deinitialize();
}

void USSVoiceCultureSubsystem::HandleStartGameInstance(UGameInstance* GameInstance)
{
	auto* VoiceCultureSettings = USSVoiceCultureSettings::GetSetting();
	CurrentLanguage = VoiceCultureSettings->GetCurrentLanguage();
	if (CurrentLanguage.IsEmpty())
	{
		UE_LOG(LogVoiceCulture, Error, TEXT("%s : Language is empty !"), *GetNameSafe(this));
	}
}

void USSVoiceCultureSubsystem::SetCurrentVoiceCulture(const FString& Language, bool bPersist)
{
	// Store the language in memory (applied immediately for runtime lookups)
	CurrentLanguage = Language;

	// Persist the new culture setting into the user config file
	auto* VoiceCultureSettings = USSVoiceCultureSettings::GetMutableSetting();
	VoiceCultureSettings->CurrentLanguage = CurrentLanguage;
	if (bPersist)
	{
		VoiceCultureSettings->SaveConfig();
	}

	// Log the culture switch
	UE_LOG(LogVoiceCulture, Log, TEXT("%s : Language switched to [%s]"), *GetNameSafe(this), *CurrentLanguage);
}

FString USSVoiceCultureSubsystem::GetCurrentVoiceCulture() const
{
	return CurrentLanguage;
}

TArray<FString> USSVoiceCultureSubsystem::GetSupportedVoiceCultures() const
{
	const auto* Settings = USSVoiceCultureSettings::GetSetting();
	return Settings->SupportedVoiceCultures.Array();
}

#if WITH_EDITOR
FString USSVoiceCultureSubsystem::GetEditorPreviewLanguage() const
{
	auto* VoiceLocalizationSettings = USSVoiceCultureSettings::GetSetting();
	return VoiceLocalizationSettings->PreviewLanguage;
}
#endif

////////////////////////////////////////////////////////////////////
// Asset Loading Manager
void USSVoiceCultureSubsystem::PreloadCatalog(
	USSVoiceCultureCatalog* Catalog,
	const FString& CultureCode,
	FStreamableDelegate OnComplete,
	TAsyncLoadPriority Priority)
{
	// A single catalog is just a batch of one - delegate to the unified path.
	PreloadCatalogs({ Catalog }, CultureCode, OnComplete, Priority);
}

void USSVoiceCultureSubsystem::PreloadCatalogs(
    const TArray<USSVoiceCultureCatalog*>& Catalogs,
    const FString& CultureCode,
    FStreamableDelegate OnComplete,
    TAsyncLoadPriority Priority)
{
    // Filter nulls once, up front - every downstream counter/log is then meaningful.
    TArray<USSVoiceCultureCatalog*> ValidCatalogs;
    ValidCatalogs.Reserve(Catalogs.Num());
    for (USSVoiceCultureCatalog* Catalog : Catalogs)
    {
        if (Catalog) ValidCatalogs.Add(Catalog);
    }

    if (ValidCatalogs.Num() == 0)
    {
        UE_LOG(LogVoiceCulture, Warning, TEXT("PreloadCatalogs: empty or all-null catalog list."));
        if (OnComplete.IsBound()) OnComplete.Execute();
        return;
    }

    const FString EffectiveCulture = CultureCode.IsEmpty() ? GetCurrentVoiceCulture() : CultureCode;

    // Stable batch hash: sort catalog paths so {A,B} and {B,A} produce the same key.
    TArray<FString> CatalogKeys;
    CatalogKeys.Reserve(ValidCatalogs.Num());
    for (USSVoiceCultureCatalog* Catalog : ValidCatalogs)
    {
        CatalogKeys.Add(FSoftObjectPath(Catalog).ToString());
    }
    CatalogKeys.Sort();

    uint32 BatchHash = GetTypeHash(EffectiveCulture.ToLower());
    for (const FString& Key : CatalogKeys)
    {
        BatchHash = HashCombine(BatchHash, GetTypeHash(Key));
    }

    // Reuse an existing identical batch (same culture + same catalog set), completed or in flight.
    if (FBatchLoadState* Existing = ActiveHandles.Find(BatchHash))
    {
        if (Existing->Handle.IsValid())
        {
            if (Existing->Handle->HasLoadCompleted())
            {
                if (OnComplete.IsBound()) OnComplete.Execute();
            }
            else if (OnComplete.IsBound())
            {
                Existing->Handle->BindCompleteDelegate(OnComplete);
            }
            UE_LOG(LogVoiceCulture, Log,
                TEXT("Preload: reusing existing batch (hash=0x%x, culture=[%s], %d catalog(s))."),
                BatchHash, *EffectiveCulture, ValidCatalogs.Num());
            return;
        }
        // Stale entry - drop it and reissue.
        ActiveHandles.Remove(BatchHash);
    }

    // Gather every soft path across every catalog, deduplicated.
    TSet<FSoftObjectPath> UniquePaths;
    UniquePaths.Reserve(ValidCatalogs.Num() * 16);
    for (USSVoiceCultureCatalog* Catalog : ValidCatalogs)
    {
        TArray<FSoftObjectPath> PerCatalogPaths;
        Catalog->GatherSoftPathsForCulture(EffectiveCulture, PerCatalogPaths);
        UniquePaths.Append(MoveTemp(PerCatalogPaths));
    }

    if (UniquePaths.Num() == 0)
    {
        UE_LOG(LogVoiceCulture, Verbose,
            TEXT("Preload: nothing to load for culture [%s] across %d catalog(s)."),
            *EffectiveCulture, ValidCatalogs.Num());
        if (OnComplete.IsBound()) OnComplete.Execute();
        return;
    }

    FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
    const FString DebugName = (ValidCatalogs.Num() == 1)
        ? FString::Printf(TEXT("VoiceCultureCatalog:%s[%s]"), *ValidCatalogs[0]->GetName(), *EffectiveCulture)
        : FString::Printf(TEXT("VoiceCultureBatch:%d catalogs [%s]"), ValidCatalogs.Num(), *EffectiveCulture);

    TSharedPtr<FStreamableHandle> Handle = Streamable.RequestAsyncLoad(
        UniquePaths.Array(),
        OnComplete,
        Priority,
        /*bManageActiveHandle*/ false,
        /*bStartStalled*/ false,
        DebugName);

    // Retain the handle + weak refs to catalogs so ReleaseCatalog can find & drop this batch.
    FBatchLoadState State;
    State.Handle = Handle;
    State.CultureCode = EffectiveCulture;
    State.Catalogs.Reserve(ValidCatalogs.Num());
    for (USSVoiceCultureCatalog* Catalog : ValidCatalogs)
    {
        State.Catalogs.Add(Catalog);
    }
    ActiveHandles.Add(BatchHash, MoveTemp(State));

    UE_LOG(LogVoiceCulture, Log,
        TEXT("Preload: batch for culture [%s] : %d catalog(s), %d unique asset(s) (hash=0x%x)."),
        *EffectiveCulture, ValidCatalogs.Num(), UniquePaths.Num(), BatchHash);
}

void USSVoiceCultureSubsystem::ReleaseCatalog(USSVoiceCultureCatalog* Catalog)
{
	if (!Catalog) return;

	// Drop any batch that references this catalog.
	for (auto It = ActiveHandles.CreateIterator(); It; ++It)
	{
		if (It.Value().Catalogs.ContainsByPredicate(
				[Catalog](const TWeakObjectPtr<USSVoiceCultureCatalog>& Weak){ return Weak.Get() == Catalog; }))
		{
			It.RemoveCurrent();
		}
	}
}

void USSVoiceCultureSubsystem::ReleaseAllCatalogs()
{
	ActiveHandles.Empty();
}

bool USSVoiceCultureSubsystem::IsCatalogLoading(const USSVoiceCultureCatalog* Catalog) const
{
	if (!Catalog) return false;
	for (const auto& Pair : ActiveHandles)
	{
		const FBatchLoadState& State = Pair.Value;
		if (!State.Handle.IsValid() || State.Handle->HasLoadCompleted()) continue;

		const bool bReferences = State.Catalogs.ContainsByPredicate(
			[Catalog](const TWeakObjectPtr<USSVoiceCultureCatalog>& Weak){ return Weak.Get() == Catalog; });

		if (bReferences) return true;
	}
	return false;
}
