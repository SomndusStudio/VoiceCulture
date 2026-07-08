/**
* Copyright (C) Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#include "SSPreloadVoiceCatalogAction.h"

#include "SSVoiceCultureCatalog.h"
#include "SSVoiceCultureLog.h"
#include "SSVoiceCultureSubsystem.h"
#include "Engine/Engine.h"

////////////////////////////////////////////////////////////////////
// USSPreloadVoiceCatalogAction

USSPreloadVoiceCatalogAction* USSPreloadVoiceCatalogAction::PreloadVoiceCultureCatalog(
    UObject* WorldContextObject,
    USSVoiceCultureCatalog* Catalog,
    const FString& CultureCode)
{
    USSPreloadVoiceCatalogAction* Action = NewObject<USSPreloadVoiceCatalogAction>();
    Action->CatalogRef = Catalog;
    Action->RequestedCulture = CultureCode;
    Action->WorldContext = WorldContextObject;
    return Action;
}

USSPreloadVoiceCatalogAction* USSPreloadVoiceCatalogAction::PreloadVoiceCultureCatalogForCurrentCulture(
    UObject* WorldContextObject, USSVoiceCultureCatalog* Catalog)
{
    // Empty culture string tells Activate() to use the current culture from the subsystem.
    return PreloadVoiceCultureCatalog(WorldContextObject, Catalog, FString());
}

void USSPreloadVoiceCatalogAction::Activate()
{
    // Track wall-clock time from Activate() to completion - useful when profiling streaming stalls.
    StartTimeSeconds = FPlatformTime::Seconds();

    if (!CatalogRef)
    {
        UE_LOG(LogVoiceCulture, Warning,
            TEXT("PreloadVoiceCultureCatalog: null catalog provided (WorldContext=[%s]). Broadcasting OnFailed."),
            *GetNameSafe(WorldContext.Get()));
        OnFailed.Broadcast();
        SetReadyToDestroy();
        return;
    }

    if (!GEngine)
    {
        UE_LOG(LogVoiceCulture, Error,
            TEXT("PreloadVoiceCultureCatalog [%s]: GEngine unavailable at Activate time. Broadcasting OnFailed."),
            *GetNameSafe(CatalogRef));
        OnFailed.Broadcast();
        SetReadyToDestroy();
        return;
    }

    auto* Subsystem = GEngine->GetEngineSubsystem<USSVoiceCultureSubsystem>();
    if (!Subsystem)
    {
        UE_LOG(LogVoiceCulture, Error,
            TEXT("PreloadVoiceCultureCatalog [%s]: VoiceCultureSubsystem not found. Broadcasting OnFailed."),
            *GetNameSafe(CatalogRef));
        OnFailed.Broadcast();
        SetReadyToDestroy();
        return;
    }

    // Resolve current culture here if none was requested, so the subsystem receives an explicit value
    const bool bUsedCurrentCulture = RequestedCulture.IsEmpty();
    if (bUsedCurrentCulture)
    {
        RequestedCulture = Subsystem->GetCurrentVoiceCulture();
        UE_LOG(LogVoiceCulture, Verbose,
            TEXT("PreloadVoiceCultureCatalog [%s]: no culture requested, resolved current culture to [%s]."),
            *GetNameSafe(CatalogRef), *RequestedCulture);
    }

    if (RequestedCulture.IsEmpty())
    {
        UE_LOG(LogVoiceCulture, Warning,
            TEXT("PreloadVoiceCultureCatalog [%s]: could not resolve any culture code. Broadcasting OnFailed."),
            *GetNameSafe(CatalogRef));
        OnFailed.Broadcast();
        SetReadyToDestroy();
        return;
    }

    UE_LOG(LogVoiceCulture, Log,
        TEXT("PreloadVoiceCultureCatalog [%s]: preload requested for culture [%s]%s."),
        *GetNameSafe(CatalogRef),
        *RequestedCulture,
        bUsedCurrentCulture ? TEXT(" (current)") : TEXT(""));

    Subsystem->PreloadCatalog(CatalogRef, RequestedCulture,
        FStreamableDelegate::CreateUObject(this, &USSPreloadVoiceCatalogAction::HandlePreloadCompleted));
}

void USSPreloadVoiceCatalogAction::HandlePreloadCompleted()
{
    const double ElapsedMs = (FPlatformTime::Seconds() - StartTimeSeconds) * 1000.0;

    UE_LOG(LogVoiceCulture, Log,
        TEXT("PreloadVoiceCultureCatalog [%s]: completed for culture [%s] in %.2f ms. Broadcasting OnLoaded."),
        *GetNameSafe(CatalogRef),
        *RequestedCulture,
        ElapsedMs);

    OnLoaded.Broadcast();
    SetReadyToDestroy();
}

////////////////////////////////////////////////////////////////////
// USSPreloadVoiceCatalogsAction
USSPreloadVoiceCatalogsAction* USSPreloadVoiceCatalogsAction::PreloadVoiceCultureCatalogs(
    UObject* WorldContextObject,
    const TArray<USSVoiceCultureCatalog*>& Catalogs,
    const FString& CultureCode)
{
    USSPreloadVoiceCatalogsAction* Action = NewObject<USSPreloadVoiceCatalogsAction>();
    Action->CatalogRefs.Reserve(Catalogs.Num());
    for (USSVoiceCultureCatalog* Catalog : Catalogs)
    {
        Action->CatalogRefs.Add(Catalog);
    }
    Action->RequestedCulture = CultureCode;
    Action->WorldContext = WorldContextObject;
    return Action;
}

USSPreloadVoiceCatalogsAction* USSPreloadVoiceCatalogsAction::PreloadVoiceCultureCatalogsForCurrentCulture(
    UObject* WorldContextObject,
    const TArray<USSVoiceCultureCatalog*>& Catalogs)
{
    // Empty culture string tells Activate() to use the current culture from the subsystem.
    return PreloadVoiceCultureCatalogs(WorldContextObject, Catalogs, FString());
}

void USSPreloadVoiceCatalogsAction::Activate()
{
    StartTimeSeconds = FPlatformTime::Seconds();

    // Filter out nulls up front so counts in logs are meaningful
    CatalogRefs.RemoveAll([](const TObjectPtr<USSVoiceCultureCatalog>& C){ return C == nullptr; });

    if (CatalogRefs.Num() == 0)
    {
        UE_LOG(LogVoiceCulture, Warning,
            TEXT("PreloadVoiceCultureCatalogs: empty or all-null catalog list (WorldContext=[%s]). Broadcasting OnFailed."),
            *GetNameSafe(WorldContext.Get()));
        OnFailed.Broadcast();
        SetReadyToDestroy();
        return;
    }

    if (!GEngine)
    {
        UE_LOG(LogVoiceCulture, Error,
            TEXT("PreloadVoiceCultureCatalogs: GEngine unavailable at Activate time. Broadcasting OnFailed."));
        OnFailed.Broadcast();
        SetReadyToDestroy();
        return;
    }

    auto* Subsystem = GEngine->GetEngineSubsystem<USSVoiceCultureSubsystem>();
    if (!Subsystem)
    {
        UE_LOG(LogVoiceCulture, Error,
            TEXT("PreloadVoiceCultureCatalogs: VoiceCultureSubsystem not found. Broadcasting OnFailed."));
        OnFailed.Broadcast();
        SetReadyToDestroy();
        return;
    }

    const bool bUsedCurrentCulture = RequestedCulture.IsEmpty();
    if (bUsedCurrentCulture)
    {
        RequestedCulture = Subsystem->GetCurrentVoiceCulture();
        UE_LOG(LogVoiceCulture, Verbose,
            TEXT("PreloadVoiceCultureCatalogs: no culture requested, resolved current culture to [%s]."),
            *RequestedCulture);
    }

    if (RequestedCulture.IsEmpty())
    {
        UE_LOG(LogVoiceCulture, Warning,
            TEXT("PreloadVoiceCultureCatalogs: could not resolve any culture code. Broadcasting OnFailed."));
        OnFailed.Broadcast();
        SetReadyToDestroy();
        return;
    }

    UE_LOG(LogVoiceCulture, Log,
        TEXT("PreloadVoiceCultureCatalogs: batch preload requested for culture [%s]%s across %d catalog(s)."),
        *RequestedCulture,
        bUsedCurrentCulture ? TEXT(" (current)") : TEXT(""),
        CatalogRefs.Num());

    // Materialize the TObjectPtr array into a raw pointer array for the subsystem API
    TArray<USSVoiceCultureCatalog*> RawCatalogs;
    RawCatalogs.Reserve(CatalogRefs.Num());
    for (const TObjectPtr<USSVoiceCultureCatalog>& C : CatalogRefs)
    {
        RawCatalogs.Add(C.Get());
    }

    Subsystem->PreloadCatalogs(RawCatalogs, RequestedCulture,
        FStreamableDelegate::CreateUObject(this, &USSPreloadVoiceCatalogsAction::HandlePreloadCompleted));
}

void USSPreloadVoiceCatalogsAction::HandlePreloadCompleted()
{
    const double ElapsedMs = (FPlatformTime::Seconds() - StartTimeSeconds) * 1000.0;

    UE_LOG(LogVoiceCulture, Log,
        TEXT("PreloadVoiceCultureCatalogs: batch completed for culture [%s] across %d catalog(s) in %.2f ms. Broadcasting OnLoaded."),
        *RequestedCulture,
        CatalogRefs.Num(),
        ElapsedMs);

    OnLoaded.Broadcast();
    SetReadyToDestroy();
}