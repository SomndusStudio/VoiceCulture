/**
* Copyright (C) Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "SSPreloadVoiceCatalogAction.generated.h"

class USSVoiceCultureCatalog;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSSVoiceCatalogPreloadPin);

////////////////////////////////////////////////////////////////////
// USSPreloadVoiceCatalogAction

/**
 * Blueprint async action node that preloads every voice line of a USSVoiceCultureCatalog
 * for a given voice culture, as a single streamable batch.
 *
 * Exposes two exec pins to Blueprint:
 *   - OnLoaded : fired on the game thread once every soft-referenced audio asset in the
 *                catalog for the requested culture has finished loading.
 *   - OnFailed : fired synchronously from Activate() when the request cannot even start
 *                (null catalog, missing subsystem, unresolvable culture, ...).
 *
 * The actual load work is delegated to USSVoiceCultureSubsystem::PreloadCatalog, which
 * routes the request through FStreamableManager and retains an FStreamableHandle for the
 * lifetime of the batch. This means:
 *   - Assets stay resident as long as the subsystem holds the handle (ref-counted with
 *     any other catalog that shares the same sounds).
 *   - Cancelling / releasing is done at the subsystem level, not on this action.
 *   - Re-requesting the same catalog+culture while a load is in flight reuses the
 *     existing handle instead of duplicating the work.
 *
 * Two static factory functions are exposed:
 *   - PreloadVoiceCultureCatalog                  : explicit culture code, for cases where
 *                                                    a specific language must be preloaded
 *                                                    (e.g. preloading upcoming culture
 *                                                    before applying a language switch).
 *   - PreloadVoiceCultureCatalogForCurrentCulture : convenience wrapper that resolves the
 *                                                    active culture from the subsystem at
 *                                                    Activate() time. Preferred entry point
 *                                                    for gameplay code that just wants
 *                                                    "load the VO for what the player is
 *                                                    currently hearing".
 *
 * Lifetime notes:
 *   - The action is a UObject managed by the Blueprint async machinery; SetReadyToDestroy()
 *     is called after either delegate broadcast, allowing it to be garbage collected.
 *   - The completion delegate is bound via CreateUObject, so if the action is destroyed
 *     before the load completes (rare - the subsystem holds the handle), the callback is
 *     safely skipped.
 *   - Timing between Activate() and completion is logged in milliseconds to help streaming
 *     teams triage hitches without needing a full Insights capture.
 */
UCLASS()
class SSVOICECULTURE_API USSPreloadVoiceCatalogAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FSSVoiceCatalogPreloadPin OnLoaded;

	UPROPERTY(BlueprintAssignable)
	FSSVoiceCatalogPreloadPin OnFailed;

	/**
	 * Async-preload every voice line in the catalog for the given culture.
	 * Leave CultureCode empty to use the currently active voice culture.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voice Culture",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Preload Voice Culture Catalog"))
	static USSPreloadVoiceCatalogAction* PreloadVoiceCultureCatalog(UObject* WorldContextObject, USSVoiceCultureCatalog* Catalog,const FString& CultureCode);

	/**
	* Async-preload every voice line in the catalog for the currently active voice culture.
	* Convenience wrapper around PreloadVoiceCultureCatalog that resolves the culture from the subsystem.
	*/
	UFUNCTION(BlueprintCallable, Category = "Voice Culture",
		meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "Preload Voice Culture Catalog (Current Culture)"))
	static USSPreloadVoiceCatalogAction* PreloadVoiceCultureCatalogForCurrentCulture(UObject* WorldContextObject, USSVoiceCultureCatalog* Catalog);

	virtual void Activate() override;

private:
	/** Called by the streamable manager when the preload batch is done. */
	UFUNCTION()
	void HandlePreloadCompleted();

	UPROPERTY()
	TObjectPtr<USSVoiceCultureCatalog> CatalogRef;

	FString RequestedCulture;

	TWeakObjectPtr<UObject> WorldContext;

	// For logging
	double StartTimeSeconds = 0.0;
};

////////////////////////////////////////////////////////////////////
// USSPreloadVoiceCatalogsAction

/**
 * Blueprint async action node that preloads every voice line across a list of
 * USSVoiceCultureCatalog assets in a single streamable batch.
 *
 * This is the preferred entry point for level-scoped preloads where the VO surface area
 * spans multiple catalogs (e.g. main dialogue + each companion + ambient barks). Compared
 * to firing N instances of PreloadVoiceCultureCatalog in parallel, this action:
 *   - Issues a single FStreamableManager request with the deduplicated union of every
 *     catalog's soft references - assets shared across catalogs are loaded once.
 *   - Fires OnLoaded exactly once, when the entire batch is resident, so gameplay code
 *     does not need to count completions manually.
 *   - Reuses an existing in-flight batch if the same catalog set + culture is requested
 *     again, avoiding redundant work when a streaming volume re-triggers.
 *
 * Two static factory functions are exposed:
 *   - PreloadVoiceCultureCatalogs                  : explicit culture code.
 *   - PreloadVoiceCultureCatalogsForCurrentCulture : convenience wrapper using the active
 *                                                     culture from the subsystem.
 *
 * Lifetime and callback safety follow the same rules as USSPreloadVoiceCatalogAction.
 */
UCLASS()
class SSVOICECULTURE_API USSPreloadVoiceCatalogsAction : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintAssignable)
    FSSVoiceCatalogPreloadPin OnLoaded;

    UPROPERTY(BlueprintAssignable)
    FSSVoiceCatalogPreloadPin OnFailed;

    /**
     * Async-preload every voice line across the given catalogs for the specified culture,
     * as a single deduplicated batch.
     */
    UFUNCTION(BlueprintCallable, Category = "Voice Culture",
        meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject",
                DisplayName = "Preload Voice Culture Catalogs"))
    static USSPreloadVoiceCatalogsAction* PreloadVoiceCultureCatalogs(
        UObject* WorldContextObject,
        const TArray<USSVoiceCultureCatalog*>& Catalogs,
        const FString& CultureCode);

    /**
     * Async-preload every voice line across the given catalogs for the currently active
     * voice culture. Convenience wrapper around PreloadVoiceCultureCatalogs.
     */
    UFUNCTION(BlueprintCallable, Category = "Voice Culture",
        meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject",
                DisplayName = "Preload Voice Culture Catalogs (Current Culture)"))
    static USSPreloadVoiceCatalogsAction* PreloadVoiceCultureCatalogsForCurrentCulture(
        UObject* WorldContextObject,
        const TArray<USSVoiceCultureCatalog*>& Catalogs);

    virtual void Activate() override;

private:

    /** Called by the streamable manager when the batch is done. */
    UFUNCTION()
    void HandlePreloadCompleted();

    UPROPERTY()
    TArray<TObjectPtr<USSVoiceCultureCatalog>> CatalogRefs;

    FString RequestedCulture;
    TWeakObjectPtr<UObject> WorldContext;
    double StartTimeSeconds = 0.0;
};
