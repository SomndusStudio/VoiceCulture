/**
* Copyright (C) 2020-2025 Schartier Isaac
*
* Official Documentation: https://www.somndus-studio.com
*/

#pragma once

#include "CoreMinimal.h"
#include "ContentBrowserModule.h"
#include "SSVoiceCultureEditorTypes.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Settings/SSVoiceCultureStrategy.h"
#include "Subsystems/EngineSubsystem.h"
#include "SSVoiceCultureEditorSubsystem.generated.h"

/**
 * Editor subsystem responsible for managing voice culture editor tools and settings.
 *
 * This includes profile management, asset scanning, strategies, and integration with the asset registry.
 * Only available in the editor (not included in builds).
 */
UCLASS()
class SSVOICECULTUREEDITOR_API USSVoiceCultureEditorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

protected:
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	
	/**
	 * Returns a reference to the Asset Registry module.
	 * Used for querying and scanning assets in the editor.
	 */
	static FAssetRegistryModule& GetAssetRegistryModule();

	/**
	 * Changes the currently active voice culture profile by name.
	 * Will refresh internal strategy and update editor behavior accordingly.
	 *
	 * @param ProfileName The name of the profile to activate.
	 */
	void ChangeActiveProfileFromName(FString ProfileName);

	
	/**
	 * Called when the voice profile name has changed (e.g., from project settings).
	 * Used to refresh the strategy or respond to user interaction.
	 */
	void OnVoiceProfileNameChange();

	/**
	 * Refreshes the currently active strategy based on the selected profile.
	 *
	 * @return A pointer to the updated voice strategy.
	 */
	USSVoiceCultureStrategy* RefreshStrategy();
	
	/**
	 * Returns the currently active strategy used to voice data.
	 * Strategy is based on the selected voice culture profile.
	 */
	UFUNCTION(BlueprintCallable, Category="Voice Culture")
	USSVoiceCultureStrategy* GetActiveStrategy();

	/**
	 * Returns the currently active voice strategy profile from the editor settings.
	 * If no profile matches or settings are invalid, returns the fallback profile.
	 *
	 * @return A pointer to the active FSSVoiceStrategyProfile.
	 */
	const FSSVoiceStrategyProfile* GetActiveProfile() const;

	/**
	 * Returns a voice strategy profile by its name.
	 * If no matching profile is found or settings are invalid, the fallback profile is returned.
	 *
	 * @param ProfileName The name of the profile to retrieve.
	 * @return A pointer to the matching FSSVoiceStrategyProfile or the fallback profile if not found.
	 */
	const FSSVoiceStrategyProfile* GetProfileFromName(FString ProfileName) const;

	/**
	 * Returns whether the subsystem has been properly initialized and is ready for use.
	 */
	bool IsReady() const;

	static TArray<FAssetData> GetAllSoundBaseAssets(bool bRecursivePaths = true);
	static TArray<FAssetData> GetAllLocalizeVoiceSoundAssets();
	
	/**
	 * Filters all localized voice sound assets to only include those that match the specified voice actor name.
	 * This is based on a simple substring match within the asset name (e.g. "NPC01" in "LVA_NPC01_Hello").
	 *
	 * @param Assets [Out] Array that will be filled with matching assets.
	 * @param VoiceActorName The name identifier used to filter voice assets (e.g. "NPC01").
	 */
	UFUNCTION(BlueprintCallable, Category="Voice Culture")
	static void GetAssetsFromVoiceActor(TArray<FAssetData>& Assets, FString VoiceActorName);

	/**
	 * Filters the provided list of assets based on their completeness with regard to supported cultures.
	 *
	 * Each voice asset may support one or more cultures via the metadata tag "VoiceCultures".
	 * This function compares the set of present cultures in each asset against the full set defined in project settings.
	 *
	 * @param Assets            [In/Out] The list of asset data to filter. Modified in-place.
	 * @param bCompleteCulture  If true, only assets that contain all supported cultures will be kept.
	 *                          If false, only assets that are missing at least one culture will be kept.
	 */
	static void GetAssetsWithCulture(TArray<FAssetData>& Assets, const bool bCompleteCulture);

	/**
	 * Retrieves voice assets that are missing one or more required cultures.
	 *
	 * @param Assets Array to populate with incomplete assets.
	 */
	UFUNCTION(BlueprintCallable, Category="Voice Culture")
	static void GetAssetsWithMissingCulture(TArray<FAssetData>& Assets);

	/**
	 * Retrieves voice assets that have all required cultures (complete).
	 *
	 * @param Assets Array to populate with complete assets.
	 */
	UFUNCTION(BlueprintCallable, Category="Voice Culture")
    static void GetAssetsWithCompleteCulture(TArray<FAssetData>& Assets);

	
	/**
	 * Returns a reference to the Content Browser module, scoped for voice culture tools.
	 * Used to extend or filter the content browser based on voice-related workflows.
	 */
	static FContentBrowserModule& GetVoiceContentBrowser();
	
private:
	
	/** Cached pointer to the active voice strategy. */
	UPROPERTY(Transient)
	USSVoiceCultureStrategy* CachedStrategy;
};
