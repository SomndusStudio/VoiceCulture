/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/StreamableManager.h"
#include "Subsystems/EngineSubsystem.h"
#include "SSVoiceCultureSubsystem.generated.h"

class USSVoiceCultureCatalog;
/**
 * Global engine subsystem that manages voice culture settings.
 *
 * Responsible for storing and resolving the current voice language used to pick culture voice audios.
 * 
 * Automatically resets to the default language (from developer settings) at the start of each game session,
 * without requiring a custom GameInstance.
 *
 * Can be queried or modified at runtime to switch voice language independently from UI/text localization.
 */
UCLASS()
class SSVOICECULTURE_API USSVoiceCultureSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Sets the active voice culture (language) used by the runtime system to resolve localized voice audio.
	 *
	 * This will override the current voice language both in memory and in the user config file (Saved/Config/).
	 * The setting is persistent between editor/game sessions.
	 *
	 * @param Language The culture code to apply (e.g. "en", "fr", "jp").
	 * @param bPersist Save ini file ?
	 */
	UFUNCTION(BlueprintCallable, Category = "Voice Culture")
	void SetCurrentVoiceCulture(const FString& Language, bool bPersist = true);

	/**
	 * Returns the currently active voice culture.
	 *
	 * @return The language code currently used for selecting culture voice assets.
	 */
	UFUNCTION(BlueprintCallable, Category = "Voice Culture")
	FString GetCurrentVoiceCulture() const;

	/**
	 * 
	 * @return The array of all supported cultures from .ini
	 */
	UFUNCTION(BlueprintCallable, Category = "Voice Culture")
	TArray<FString> GetSupportedVoiceCultures() const;

#if WITH_EDITOR
	/**
	 * In editor only: returns the preview voice culture defined in the developer settings.
	 * This is used for voice testing in PIE or the editor environment.
	 */
	FString GetEditorPreviewLanguage() const;
#endif

private:
	/** Holds the currently active voice language code. */
	FString CurrentLanguage = TEXT("en");

	/**
	 * Internal handler that is called at the start of each GameInstance.
	 * Resets the voice culture to the default (from developer settings) or preview language if in PIE.
	 */
	void HandleStartGameInstance(UGameInstance* GameInstance);

	/** Handle for the delegate binding to GameInstance start events. */
	FDelegateHandle OnStartGameInstanceHandle;

	////////////////////////////////////////////////////////////////////
	// Asset Loading Manager

public:
	/**
	 * Async-loads every audio asset referenced by the given catalog for the specified culture.
	 * The catalog is retained internally as long as the handle is alive.
	 * If a preload for the same catalog+culture is already active, the existing handle is reused.
	 *
	 * @param Catalog       The catalog to preload. Must not be null.
	 * @param CultureCode   Culture to load (e.g. "en", "fr"). Empty = use current voice culture.
	 * @param OnComplete    Optional callback fired on the game thread when loading finishes.
	 * @param Priority      Streamable priority (default: async loading priority).
	 */
	void PreloadCatalog(USSVoiceCultureCatalog* Catalog, const FString& CultureCode,
		FStreamableDelegate OnComplete = {}, TAsyncLoadPriority Priority = FStreamableManager::DefaultAsyncLoadPriority);

	/**
	  * Async-loads every audio asset referenced by the given catalogs for the specified culture,
	  * as a single streamable batch. This is the preferred entry point for level-scoped preloads
	  * where several catalogs (main dialogue, companions, ambient barks, ...) must be resident
	  * together.
	  *
	  * The batch is retained internally under a synthetic key derived from the catalog set,
	  * so releasing individual catalogs is not supported via this call - use ReleaseAllCatalogs
	  * or manage lifetime with a scope object on the caller side.
	  */
	void PreloadCatalogs(const TArray<USSVoiceCultureCatalog*>& Catalogs, const FString& CultureCode,
		FStreamableDelegate OnComplete = {},
		TAsyncLoadPriority Priority = FStreamableManager::DefaultAsyncLoadPriority);

	
	/** Releases the retained handle for a catalog, allowing its assets to be unloaded if no one else holds them. */
	void ReleaseCatalog(USSVoiceCultureCatalog* Catalog);

	/** Releases every retained catalog. */
	void ReleaseAllCatalogs();

	/** True while a preload for the given catalog is still in flight. */
	bool IsCatalogLoading(const USSVoiceCultureCatalog* Catalog) const;

private:
	
	struct FCatalogLoadState
	{
		TSharedPtr<FStreamableHandle> Handle;
		FString CultureCode;
	};
	
	/**
	   * Unified retention entry for any streamable batch - 1 catalog or N.
	   * The Catalogs array lets ReleaseCatalog find and release batches that reference
	   * a given catalog, without exposing the batch/single distinction to callers.
	   */
	struct FBatchLoadState
	{
		TSharedPtr<FStreamableHandle> Handle;
		FString CultureCode;
		TArray<TWeakObjectPtr<USSVoiceCultureCatalog>> Catalogs;
	};

	/** All active preload batches keyed by a stable hash of (culture + sorted catalog set). */
	TMap<uint32, FBatchLoadState> ActiveHandles;
};
